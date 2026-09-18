#pragma once

#include "f1sim/SimulationResult.hpp"
#include <string>
#include <vector>

namespace f1sim {

/**
 * @class CSVExporter
 * @brief Handles file I/O operations to bridge C++ computation results with Python visualization.
 */
class CSVExporter final {
public:
    CSVExporter() = delete;

    /**
     * @brief Exports granular, lap-by-lap telemetry for a single simulation run.
     * @param result The computed simulation result.
     * @param filename Target export filepath.
     * @param startingLap The lap from which data recording begins.
     */
    static void exportLapTimes(const SimulationResult& result, const std::string& filename,
                               int startingLap = 1);

    /**
     * @brief Exports aggregate race times for probability density plotting (Monte Carlo distributions).
     * @param results The dataset containing all simulation iterations.
     * @param filename Target export filepath.
     */
    static void exportMonteCarloResults(const std::vector<SimulationResult>& results,
                                        const std::string& filename);
};

} // namespace f1sim