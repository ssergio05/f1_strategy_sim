@echo off
echo --- F1 AWS LIVE STRATEGY DASHBOARD (DEMO MODE) ---
echo 1. Initializing F1 Live Telemetry Tracker (Local Demo)...
start cmd /k "python scripts/live_tracker.py --demo"

echo 2. Initializing Strategy Monitor and Monte Carlo Engine...
python scripts/mega_dashboard.py