#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
live_tracker.py — Real-Time Telemetry Collector for the Live Predictor
=================================================================================

Connects to the public **F1 Live Timing** API (SignalR Core protocol, the same
underlying stream used by FastF1) and dynamically overwrites ``live_state.json``
in the project root with the live race state, strictly matching the JSON schema
expected by the C++ backend (``src/main.cpp``).

Why a DIRECT connection instead of fastf1?
------------------------------------------
``fastf1.livetiming`` (``SignalRClient``) is designed to *record* the stream to
a file and parse it post-session via ``Session.load(livedata=...)``. The official
documentation explicitly states it cannot be used for mid-session real-time analysis.
To stream live data every ~10s, we replicate the underlying SignalR Core flow 
against ``livetiming.formula1.com`` using ``requests`` (negotiate) and ``websockets``.
No F1TV credentials are required: timing, track status, laps, and tyre streams 
are broadcast unauthenticated.

Dependencies:
    pip install requests websockets

Usage:
    python scripts/live_tracker.py                   # Live mode (during an active GP)
    python scripts/live_tracker.py --demo            # Local simulation mode (offline)
"""

from __future__ import annotations

import argparse
import asyncio
import datetime as _dt
import json
import logging
import os
import re
import random
import sys
import zlib
from pathlib import Path
from typing import Any, Dict, List, Optional
from urllib.parse import quote

import requests

try:
    import websockets
except ImportError: 
    sys.exit(
        "[FATAL] Missing 'websockets' dependency.\n"
        "  pip install requests websockets\n"
        "Check the header of scripts/live_tracker.py."
    )

LOG = logging.getLogger("live_tracker")

# --------------------------------------------------------------------------- #
#  Public F1 Live Timing API Constants (SignalR Core)                         #
# --------------------------------------------------------------------------- #
SIGNALR_NEGOTIATE_URL = "https://livetiming.formula1.com/signalrcore/negotiate?negotiateVersion=1"
SIGNALR_WS_URL = "wss://livetiming.formula1.com/signalrcore"
RECORD_SEPARATOR = "\x1e"          
HTTP_USER_AGENT = "BestHTTP"
HTTP_ORIGIN = "https://www.formula1.com"

# Unauthenticated public topics. Omitting .z (compressed) where unnecessary.
TOPICS = [
    "SessionInfo", "SessionData", "SessionStatus", "DriverList", "LapCount",
    "TrackStatus", "TimingData", "TimingAppData", "TimingStats", "TopThree",
    "ExtrapolatedClock", "Heartbeat", "WeatherData", "RaceControlMessages",
    "TeamRadio",
]

# F1 TrackStatus codes: 4 = Safety Car, 6 = VSC deployed, 7 = VSC ending. (2 is Yellow).
SAFETY_CAR_TRACK_STATUS = {"4", "6", "7"}

# Rough estimation to convert a "N LAP" gap into seconds.
SECONDS_PER_LAP_ESTIMATE = 90.0

COMPOUND_MAP = {"SOFT": "Soft", "MEDIUM": "Medium", "HARD": "Hard"}
_LAP_GAP_RE = re.compile(r"(\d+(?:\.\d+)?)\s*LAP", re.IGNORECASE)

_WS_VERSION = int(getattr(websockets, "__version__", "0").split(".")[0])
WS_HEADERS_ARG = "additional_headers" if _WS_VERSION >= 12 else "extra_headers"

PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = str(PROJECT_ROOT / "live_state.json")

# --------------------------------------------------------------------------- #
#  Utilities                                                                  #
# --------------------------------------------------------------------------- #
def _frame(payload: Dict[str, Any]) -> str:
    return json.dumps(payload, separators=(",", ":")) + RECORD_SEPARATOR

def _parse_json(text: str) -> Optional[Any]:
    try:
        return json.loads(text)
    except (TypeError, ValueError, json.JSONDecodeError):
        return None

def _zlib_decompress(data: Any) -> Any:
    if not isinstance(data, str):
        return data
    try:
        import base64
        raw = zlib.decompress(base64.b64decode(data), -zlib.MAX_WBITS)
        return _parse_json(raw.decode("utf-8"))
    except Exception:
        return _parse_json(data) or data

def _deep_merge(target: Any, patch: Any) -> Any:
    """
    Deep merge for F1 differential patches. The feed only broadcasts delta changes,
    so we must retain previous state and combine them.
    """
    if patch is None:
        return target
    if not isinstance(patch, dict):
        return patch
    if not isinstance(target, dict):
        target = {}
    result = dict(target)
    for key, patch_val in patch.items():
        target_val = result.get(key)
        if (isinstance(patch_val, dict) and isinstance(target_val, dict)):
            result[key] = _deep_merge(target_val, patch_val)
        else:
            result[key] = patch_val
    return result

def _to_int(value: Any) -> Optional[int]:
    if value is None or value == "": return None
    try:
        return int(float(str(value)))
    except (TypeError, ValueError):
        return None

def _normalize(name: Any) -> str:
    return " ".join((name or "").strip().lower().split())

def parse_position(value: Any) -> Optional[int]:
    if value is None: return None
    match = re.search(r"(\d+)", str(value))
    return int(match.group(1)) if match else None

def parse_gap(value: Any) -> float:
    if value is None: return 0.0
    text = str(value).strip().lstrip("+")
    if not text: return 0.0
    laps = _LAP_GAP_RE.search(text)
    if laps:
        return round(float(laps.group(1)) * SECONDS_PER_LAP_ESTIMATE, 3)
    try:
        return round(float(text), 3)
    except ValueError:
        return 0.0

def map_compound(compound: Any) -> Optional[str]:
    return COMPOUND_MAP.get(str(compound or "").upper())

def _write_json(obj: Dict[str, Any], path: str) -> None:
    """Atomic write to prevent the C++ engine from reading a partial JSON."""
    dst = Path(path)
    dst.parent.mkdir(parents=True, exist_ok=True)
    tmp = dst.with_suffix(dst.suffix + ".tmp")
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump(obj, fh, indent=2, ensure_ascii=False)
        fh.write("\n")
    os.replace(tmp, dst)

# --------------------------------------------------------------------------- #
#  In-Memory Store                                                            #
# --------------------------------------------------------------------------- #
class LiveStateStore:
    SESSION_SCOPED_TOPICS = {
        "TimingData", "TimingAppData", "TimingStats", "LapCount", "TopThree",
        "SessionData", "RaceControlMessages", "SessionStatus",
    }

    def __init__(self) -> None:
        self.topics: Dict[str, Any] = {}
        self.connected = False
        self.last_update: Optional[str] = None

    def apply_update(self, topic: str, data: Any) -> None:
        self.last_update = _dt.datetime.now(_dt.timezone.utc).isoformat()
        self.topics.setdefault(topic, {})

        if topic == "SessionInfo":
            new_key = data.get("Key") or data.get("key")
            old_key = self.topics[topic].get("Key") or self.topics[topic].get("key")
            if new_key is not None and old_key is not None and new_key != old_key:
                for scoped in self.SESSION_SCOPED_TOPICS:
                    self.topics[scoped] = {}

        self.topics[topic] = _deep_merge(self.topics[topic], data)

    def lap_count(self) -> Dict[str, Any]:
        return self.topics.get("LapCount") or {}

    def current_lap(self) -> Optional[int]:
        return _to_int(self.lap_count().get("CurrentLap"))

    def total_laps(self) -> Optional[int]:
        return _to_int(self.lap_count().get("TotalLaps"))

    def track_status_code(self) -> Optional[str]:
        ts = self.topics.get("TrackStatus")
        if isinstance(ts, list):
            ts = ts[-1] if ts else {}
        if not isinstance(ts, dict): return None
        code = ts.get("Status")
        if isinstance(code, dict): code = code.get("Value") or code.get("value")
        return str(code).strip() if code is not None else None

    def safety_car_active(self) -> bool:
        return self.track_status_code() in SAFETY_CAR_TRACK_STATUS

    def drivers(self) -> Dict[str, Dict[str, Any]]:
        dl = self.topics.get("DriverList")
        return dl if isinstance(dl, dict) else {}

    def timing_line(self, num: str) -> Dict[str, Any]:
        td = self.topics.get("TimingData")
        if not isinstance(td, dict): return {}
        lines = td.get("Lines") if isinstance(td.get("Lines"), dict) else {}
        line = lines.get(str(num))
        return line if isinstance(line, dict) else {}

    def tyre_stint(self, num: str) -> Dict[str, Any]:
        app = self.topics.get("TimingAppData")
        if not isinstance(app, dict): return {}
        lines = app.get("Lines") if isinstance(app.get("Lines"), dict) else {}
        line = lines.get(str(num))
        if not isinstance(line, dict): return {}
        stints = line.get("Stints")
        if isinstance(stints, dict): stints = list(stints.values())
        if not isinstance(stints, list) or not stints: return {}
        stint = stints[-1]
        return stint if isinstance(stint, dict) else {}

    def session_name(self) -> str:
        si = self.topics.get("SessionInfo")
        if not isinstance(si, dict): return ""
        meeting = si.get("Meeting") if isinstance(si.get("Meeting"), dict) else {}
        meeting_name = (meeting.get("Name") or "").strip()
        session_name = (si.get("Name") or "").strip()
        if session_name:
            return f"{session_name} ({meeting_name})" if meeting_name else session_name
        return meeting_name

# --------------------------------------------------------------------------- #
#  SignalR Core Client                                                        #
# --------------------------------------------------------------------------- #
class SignalRClient:
    def __init__(self, store: LiveStateStore, debug: bool = False) -> None:
        self.store = store
        self.debug = debug

    async def run_forever(self) -> None:
        delay = 2.0
        while True:
            try:
                await self._connect_once()
            except asyncio.CancelledError:
                raise
            except Exception as exc: 
                LOG.error("Connection Error: %s", exc)
            self.store.connected = False
            LOG.info("Retrying in %.0f s...", delay)
            try:
                await asyncio.sleep(delay)
            except asyncio.CancelledError:
                raise
            delay = min(delay * 2, 30.0)

    async def _negotiate(self):
        try:
            resp = requests.post(
                SIGNALR_NEGOTIATE_URL, json={}, timeout=15,
                headers={"User-Agent": HTTP_USER_AGENT, "Origin": HTTP_ORIGIN, "Accept": "application/json, text/plain, */*"},
            )
        except requests.RequestException as exc:
            raise ConnectionError(f"HTTP Negotiate failed: {exc}") from exc
        if resp.status_code != 200:
            raise ConnectionError(f"Negotiate returned HTTP {resp.status_code}: {resp.text[:200]}")
        payload = resp.json()
        token = payload.get("connectionToken") or payload.get("connectionId")
        if not token:
            raise ConnectionError("Negotiate did not return connectionId/connectionToken.")
        cookie = "; ".join(f"{k}={v}" for k, v in resp.cookies.items())
        LOG.debug("Negotiate OK (token=%s..., cookies=%s)", token[:8], bool(cookie))
        return token, cookie

    async def _ping_loop(self, ws) -> None:
        try:
            while True:
                await asyncio.sleep(15)
                await ws.send(_frame({"type": 6}))
        except (asyncio.CancelledError, Exception): 
            return

    async def _connect_once(self) -> None:
        token, cookie = await self._negotiate()
        ws_url = f"{SIGNALR_WS_URL}?id={quote(token, safe='')}"
        headers = {"User-Agent": HTTP_USER_AGENT, "Origin": HTTP_ORIGIN}
        if cookie: headers["Cookie"] = cookie

        LOG.info("Opening WebSocket and negotiating handshake...")
        connect_kwargs = {WS_HEADERS_ARG: headers}
        async with websockets.connect(ws_url, **connect_kwargs) as ws:
            await ws.send(_frame({"protocol": "json", "version": 1}))
            self.store.connected = True
            LOG.info("WebSocket connected. Awaiting F1 Live Timing data...")

            ping_task = asyncio.create_task(self._ping_loop(ws))
            buffer = ""
            handshake_done = False
            try:
                async for message in ws:
                    buffer += message
                    frames = buffer.split(RECORD_SEPARATOR)
                    buffer = frames.pop() 

                    if not handshake_done:
                        handshake_done = True
                        handshake = _parse_json(frames[0]) if frames else {}
                        if isinstance(handshake, dict) and handshake.get("error"):
                            raise ConnectionError(f"SignalR Handshake failed: {handshake['error']}")
                        await ws.send(_frame({
                            "type": 1, "invocationId": "0",
                            "target": "Subscribe", "arguments": [TOPICS],
                        }))
                        LOG.info("Subscribed to %d topics.", len(TOPICS))
                        if len(frames) > 1:
                            await self._handle_frames(ws, RECORD_SEPARATOR.join(frames[1:]) + RECORD_SEPARATOR)
                        continue

                    if frames:
                        await self._handle_frames(ws, RECORD_SEPARATOR.join(frames))
            finally:
                ping_task.cancel()
            self.store.connected = False
            LOG.warning("WebSocket closed by the server.")

    async def _handle_frames(self, ws, raw: str) -> None:
        for frame in raw.split(RECORD_SEPARATOR):
            frame = frame.strip()
            if not frame: continue
            msg = _parse_json(frame)
            if not isinstance(msg, dict): continue
            msg_type = msg.get("type")
            if msg_type == 1 and msg.get("target") == "feed":
                args = msg.get("arguments") or []
                if len(args) >= 2:
                    self._process_topic(str(args[0]), args[1])
            elif msg_type == 3: 
                if msg.get("invocationId") == "0":
                    result = msg.get("result")
                    if isinstance(result, dict):
                        for topic, data in result.items():
                            if data not in (None, "", {}):
                                self._process_topic(str(topic), data)
            elif msg_type == 6: 
                await ws.send(_frame({"type": 6}))
            elif msg_type == 7:
                LOG.warning("SignalR server requested connection closure.")

    def _process_topic(self, topic: str, data: Any) -> None:
        if topic.endswith(".z"): data = _zlib_decompress(data)
        elif isinstance(data, str): data = _parse_json(data)
        if data is None: return
        self.store.apply_update(topic, data)

# --------------------------------------------------------------------------- #
#  JSON State Builder                                                         #
# --------------------------------------------------------------------------- #
def build_live_state(store: LiveStateStore, target_names: List[str], memory: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    drivers = store.drivers()
    if not drivers: return None

    driver_index: Dict[str, tuple] = {}
    for num, drv in drivers.items():
        if not isinstance(drv, dict): continue
        driver_index[str(num)] = (_normalize(drv.get("FullName")), str(drv.get("Tla") or "").upper())

    competitors: List[Dict[str, Any]] = []
    seen_numbers = set()
    for target in target_names:
        norm = _normalize(target)
        num = None
        for n, (full, tla) in driver_index.items():
            if n in seen_numbers: continue
            if target.upper() == tla or (norm and norm in full):
                num = n
                break
        if num is None: continue
        seen_numbers.add(num)

        td = store.timing_line(num)
        position = parse_position(td.get("Position"))
        
        # Memory patch: Avoid missing drivers due to temporary AWS API dropouts
        if position is None:
            position = memory.get((target, "pos"))
        else:
            memory[(target, "pos")] = position
            
        if position is None: continue

        st = store.tyre_stint(num)
        compound = map_compound(st.get("Compound"))
        if compound is None:
            compound = memory.get(target)
        else:
            memory[target] = compound
        if compound is None:
            LOG.debug("No known compound for %s; skipping.", target)
            continue

        laps_on_tyre = _to_int(st.get("TotalLaps"))
        if laps_on_tyre is None:
            laps_on_tyre = memory.get((target, "laps"), 0)
        else:
            memory[(target, "laps")] = laps_on_tyre

        competitors.append({
            "name": target,
            "position": position,
            "gap_to_leader": parse_gap(td.get("GapToLeader")),
            "current_tyre": compound,
            "laps_on_tyre": laps_on_tyre,
        })

    if not competitors: return None
    competitors.sort(key=lambda c: c["position"])

    lap = store.current_lap()
    total = store.total_laps()
    if lap is None or total is None:
        LOG.debug("Awaiting valid laps from feed (lap=%s, total=%s).", lap, total)
        return None

    return {
        "session": {
            "current_lap": lap,
            "total_laps": total,
            "safety_car": bool(store.safety_car_active()),
        },
        "competitors": competitors,
    }

# --------------------------------------------------------------------------- #
#  Demo Mode (Local testing without F1 connection)                            #
# --------------------------------------------------------------------------- #
class DemoSession:
    DEMO_DRIVERS = [
        ("Hamilton",   "Medium", 30, "Hard", 0.0),
        ("Leclerc",    "Hard",   31, "Medium", 1.2),
        ("Verstappen", "Medium", 35, "Soft",   2.8),
    ]

    def __init__(self, total_laps: int = 70, drivers: Optional[list] = None) -> None:
        self.total_laps = total_laps
        self.current_lap = 0
        self.sc_until_lap = 0
        self.drivers = [
            {
                "name": name, "compound": first, "pit_lap": pit,
                "next_compound": second, "gap": gap0, "laps_on_tyre": 15 - i * 3,
            }
            for i, (name, first, pit, second, gap0) in enumerate(drivers or self.DEMO_DRIVERS)
        ]

    def snapshot(self) -> Dict[str, Any]:
        self.current_lap = min(self.current_lap + 1, self.total_laps)
        if self.sc_until_lap > 0:
            self.sc_until_lap -= 1
        elif random.random() < 0.03: 
            self.sc_until_lap = 4

        competitors = []
        for drv in self.drivers:
            drv["laps_on_tyre"] += 1
            if drv["pit_lap"] == self.current_lap:
                drv["compound"] = drv["next_compound"]
                drv["laps_on_tyre"] = 0
            drv["laps_on_tyre"] = min(drv["laps_on_tyre"], self.current_lap)
            drv["gap"] = max(0.0, round(drv["gap"] + random.uniform(-0.4, 0.6), 3))
            competitors.append({
                "name": drv["name"], "position": 0, "gap_to_leader": drv["gap"],
                "current_tyre": drv["compound"], "laps_on_tyre": drv["laps_on_tyre"],
            })

        competitors.sort(key=lambda c: c["gap_to_leader"])
        for pos, comp in enumerate(competitors, start=1):
            comp["position"] = pos

        return {
            "session": {"current_lap": self.current_lap, "total_laps": self.total_laps, "safety_car": bool(self.sc_until_lap > 0)},
            "competitors": competitors,
        }

# --------------------------------------------------------------------------- #
#  Main Execution Loop                                                        #
# --------------------------------------------------------------------------- #
def _parse_args(argv: Optional[List[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="live_tracker.py",
        description="Real-Time F1 Telemetry Collector -> live_state.json",
    )
    parser.add_argument("--output", default=DEFAULT_OUTPUT, help=f"JSON output path (default: {DEFAULT_OUTPUT})")
    parser.add_argument("--interval", type=float, default=10.0, help="Seconds between updates (default: 10.0)")
    parser.add_argument("--drivers", default="Verstappen,Leclerc,Norris", help="Target drivers separated by commas")
    parser.add_argument("--demo", action="store_true", help="Local simulation mode (no F1 connection required)")
    parser.add_argument("--total-laps", type=int, default=70, help="Total race laps in demo mode (default: 70)")
    parser.add_argument("--debug", action="store_true", help="Detailed logging")
    return parser.parse_args(argv)

async def _live_loop(args: argparse.Namespace, driver_names: List[str]) -> None:
    store = LiveStateStore()
    client = SignalRClient(store, debug=args.debug)
    conn_task = asyncio.create_task(client.run_forever())

    memory: Dict[Any, Any] = {}
    last_session = ""
    try:
        while True:
            session = store.session_name()
            if session and session != last_session:
                last_session = session
                LOG.info("Session detected: %s", session)

            snapshot = build_live_state(store, driver_names, memory)
            if snapshot is None:
                LOG.warning("Awaiting sufficient live feed data. JSON not overwritten.")
            else:
                _write_json(snapshot, args.output)
                LOG.info(
                    "live_state.json updated -> Lap %s/%s, SC=%s, %d drivers",
                    snapshot["session"]["current_lap"], snapshot["session"]["total_laps"],
                    snapshot["session"]["safety_car"], len(snapshot["competitors"]),
                )
            await asyncio.sleep(args.interval)
    finally:
        conn_task.cancel()
        try:
            await conn_task
        except asyncio.CancelledError:
            pass

async def _demo_loop(args: argparse.Namespace) -> None:
    demo = DemoSession(total_laps=args.total_laps)
    try:
        while True:
            snapshot = demo.snapshot()
            _write_json(snapshot, args.output)
            LOG.info(
                "[demo] Lap %s/%s, SC=%s, %d drivers -> %s",
                snapshot["session"]["current_lap"], snapshot["session"]["total_laps"],
                snapshot["session"]["safety_car"], len(snapshot["competitors"]), args.output,
            )
            await asyncio.sleep(args.interval)
    except asyncio.CancelledError:
        raise
    except KeyboardInterrupt:
        pass

async def _amain(args: argparse.Namespace) -> None:
    if args.demo: await _demo_loop(args)
    else: await _live_loop(args, driver_names=args.driver_names)

def main(argv: Optional[List[str]] = None) -> int:
    args = _parse_args(argv)
    logging.basicConfig(
        level=logging.DEBUG if args.debug else logging.INFO,
        format="%(asctime)s [%(levelname)s] %(message)s",
        datefmt="%H:%M:%S",
    )
    args.driver_names = [d.strip() for d in args.drivers.split(",") if d.strip()]
    if not args.driver_names:
        LOG.error("The --drivers list is empty.")
        return 2

    try:
        asyncio.run(_amain(args))
    except KeyboardInterrupt:
        LOG.info("Stopped by user. Final state retained at %s.", args.output)
    return 0

if __name__ == "__main__":
    sys.exit(main())