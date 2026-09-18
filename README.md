# 🏎️ F1 Live Strategy Simulator & Monte Carlo Engine

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9-green.svg)
![CMake](https://img.shields.io/badge/CMake-Build-orange.svg)
![FastF1](https://img.shields.io/badge/Data-FastF1%20%7C%20AWS-yellow.svg)

## 📌 Overview
An advanced, real-time Formula 1 race strategy simulator that combines a **high-performance C++ Monte Carlo Engine** with a **Python-based live telemetry tracker**. This project is designed to predict race outcomes, evaluate tyre degradation, and optimize pit-stop strategies dynamically using live AWS telemetry data.

## ⚙️ Core Architecture
The system follows a strict decoupling between heavy mathematical computation and live data processing:

1. **The C++ Core Engine (`/src`, `/include/f1sim`):** Handles the mathematical modeling of the race. It features a custom tyre degradation model and a Monte Carlo simulation engine capable of running thousands of race permutations in milliseconds to calculate expected total race times and probabilistic finishing positions.
2. **The Python Telemetry Layer (`/scripts`):** Acts as the live data ingestion and visualization layer. It hooks into live telemetry (via AWS/FastF1), handles missing data points (e.g., VSC latency dropouts), and outputs the current race state to `live_state.json`.
3. **Inter-Process Communication:** The C++ engine parses the live JSON state (via `nlohmann/json`), computes the Monte Carlo distributions, and exports the results to CSV files (`monte_carlo_results.csv`), which are then dynamically plotted by Python using Matplotlib.

## 🛠️ Key Features
- **Live AWS Telemetry Integration:** Real-time tracking of lap times, tyre compounds, and driver positions.
- **Dynamic Tyre Degradation Model:** Calculates performance drop-off based on tyre age and compound (Soft, Medium, Hard).
- **Monte Carlo Strategy Optimization:** Runs probabilistic distributions to determine the optimal pit lap and predict final race gaps.
- **Robust Anomaly Handling:** Features a "memory" state in the Python tracker to handle temporary F1 API telemetry dropouts (e.g., during pit entry or Virtual Safety Car periods) without crashing the C++ engine.

## 📂 Project Structure
```text
f1_strategy_sim/
├── include/f1sim/       # C++ Headers (Car, RaceSession, MonteCarloEngine, TyreModel, etc.)
├── src/                 # C++ Source Files 
├── scripts/             # Python Modules (live_tracker.py, mega_dashboard.py)
├── tests/               # Unit tests for the C++ mathematical models
├── CMakeLists.txt       # Build configuration
└── live_dashboard.bat   # Windows execution script to run the full pipeline
```

## 🚀 Build and Run

### Prerequisites
- C++17 Compiler (GCC/MSVC) and CMake.
- Python 3.9+ with `requirements-live.txt` installed.

### Build the C++ Engine
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Run the Live Dashboard
Execute the batch script to launch both the live tracker and the Monte Carlo visualization dashboard simultaneously:
```bash
./live_dashboard.bat
```

## 👨‍💻 Author
**Sergio**
*Software Engineering & Race Strategy Enthusiast*