@echo off
echo --- F1 AWS LIVE STRATEGY DASHBOARD ---
echo 1. Initializing F1 Live Telemetry Tracker...
start cmd /k "python scripts/live_tracker.py --drivers Norris,Antonelli,Verstappen"

echo 2. Initializing Strategy Monitor and Monte Carlo Engine...
python scripts/mega_dashboard.py