# F1 Monte Carlo Strategy Simulator 🏎️⏱️

An ultra-fast, multithreaded Formula 1 race strategy simulator built in modern **C++20**. It uses Monte Carlo methods to evaluate race strategies by calculating tyre degradation profiles, fuel mass depletion, and stochastic race events (traffic, driver variance) across tens of thousands of parallel race simulations.

Coupled with a **Python/Pandas** visualization bridge, it generates professional telemetry dashboards to assist the pitwall in decision-making.

## 🚀 Core Engine Features

* **Lock-Free Concurrency**: Leverages `std::async` and hardware concurrency to run 10,000+ full race simulations in ~11 milliseconds by isolating memory states per thread.
* **Zero-Overhead Math**: Aggressive compiler optimization (`-O3 -march=native`), `constexpr`/`noexcept` math functions, and contiguous memory structures to maximize L1/L2 cache locality.
* **Stochastic Physics Engine**: Simulates fuel weight penalty and a dual-phase tyre degradation model (linear wear + exponential cliff).
* **100% Test Coverage**: Fully unit-tested mathematical core using **Google Test**.
* **Telemetry Pipeline**: C++ backend exports raw data to CSV, consumed by a Python telemetry script for dark-mode data visualization.

## 🧮 Mathematical Model

The physics engine calculates the lap time at any given lap $L$ using the following model:

`T_lap = T_base - (Delta_fuel * M_fuel) + (Alpha * L) + Beta * e^(Lambda * (L - L_cliff))`

* **Fuel Penalty (`Delta_fuel * M_fuel`)**: The car gets faster as fuel mass decreases.
* **Linear Degradation (`Alpha * L`)**: Standard tyre wear over time.
* **Exponential Cliff (`Beta * e^...`)**: Simulates the sudden loss of grip when a tyre compound drops out of its working temperature/rubber window after `L_cliff`.

## 🛠️ Prerequisites

* **C++ Compiler**: MSVC (Windows), GCC, or Clang with C++20 support.
* **Build System**: CMake (v3.14+).
* **Python 3.x**: With `pandas` and `matplotlib` for the visualization dashboard.

## ⚙️ Build Instructions (Out-of-source Build)

The project handles dependencies (like Google Test) automatically via CMake `FetchContent`.

```bash
# 1. Clone the repository
git clone https://github.com/ssergio05/f1_strategy_sim.git
cd f1_strategy_sim

# 2. Create build directory and configure
mkdir build
cd build
cmake ..

# 3. Build the project (Release mode for maximum CPU optimization)
cmake --build . --config Release
```

## 🏁 Running the Simulation & Tests

Execute the test suite to validate the physics engine:
```bash
ctest -C Release --output-on-failure
```

Run the Monte Carlo simulator:
```bash
# Windows
Release\f1_simulator.exe

# Linux/macOS
./f1_simulator
```

## 📊 Telemetry Visualization

After running the C++ simulator, the engine will generate `lap_times.csv` and `monte_carlo_results.csv` in your build folder. To render the pitwall dashboard:

```bash
# Run from the project root
python scripts/plot_results.py
```
*(This will generate a high-resolution `f1_strategy_dashboard.png` with the degradation profile and the Monte Carlo Gaussian distribution).*

## 📂 Architecture Overview

* `include/` & `src/`: Core C++20 engine (`TyreModel`, `Car`, `RaceSession`, `MonteCarloEngine`).
* `tests/`: Google Test suite ensuring the mathematical integrity of the physical models.
* `scripts/`: Python analytics bridge.