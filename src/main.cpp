#include <iostream>
#include <memory>
#include <chrono>
#include "f1sim/TyreModel.hpp"
#include "f1sim/Car.hpp"
#include "f1sim/Strategy.hpp"
#include "f1sim/RaceSession.hpp"
#include "f1sim/MonteCarloEngine.hpp"
#include "f1sim/CSVExporter.hpp"

using namespace f1sim;

int main() {
    std::cout << "--- F1 Monte Carlo Strategy Simulator ---\n";

    // 1. Setup Initial Parameters
    const double baseLapTime = 90.0; // 1:30.000
    const double initialFuelMass = 110.0; // kg
    const double fuelDelta = 0.03; // time lost per kg of fuel
    const int totalLaps = 70;

    // 2. Create Tyre Compounds
    // Soft tyre: fast degradation, steep cliff after lap 15
    auto startingTyre = std::make_shared<TyreModel>(0.1, 0.05, 0.2, 15.0);
    // Hard tyre: low degradation, cliff after lap 40
    auto secondTyre = std::make_shared<TyreModel>(0.04, 0.02, 0.1, 40.0);

    // 3. Create Car and Strategy
    Car car(baseLapTime, initialFuelMass, fuelDelta, startingTyre);
    Strategy strategy{20, startingTyre, secondTyre}; // Pit stop on lap 20

    // 4. Create Race Session Template
    RaceSession baseSession(car, strategy, totalLaps);

    // 5. Initialize Monte Carlo Engine
    MonteCarloEngine engine;
    const uint32_t numSimulations = 10000;

    std::cout << "Starting " << numSimulations << " parallel simulations...\n";

    // Measure execution time
    auto start_time = std::chrono::high_resolution_clock::now();

    // 6. Run Simulations
    std::vector<SimulationResult> results = engine.runSimulations(numSimulations, baseSession);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    // 7. Output Results
    std::cout << "Simulations completed successfully!\n";
    std::cout << "Total Results gathered: " << results.size() << "\n";
    std::cout << "Execution time: " << elapsed.count() << " ms\n";

    // 8. Export to CSV
    CSVExporter::exportLapTimes(results[0], "lap_times.csv");
    CSVExporter::exportMonteCarloResults(results, "monte_carlo_results.csv");
    std::cout << "Data exported to CSV files.\n";

    return 0;
}