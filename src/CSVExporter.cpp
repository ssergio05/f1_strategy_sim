#include "f1sim/CSVExporter.hpp"

#include <fstream>
#include <iostream>

namespace f1sim {

void CSVExporter::exportLapTimes(const SimulationResult& result, const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file '" << filename << "' for writing.\n";
        return;
    }

    file << "Lap,LapTime\n";
    for (std::size_t i = 0; i < result.lapTimes.size(); ++i) {
        file << (i + 1) << ',' << result.lapTimes[i] << '\n';
    }
}

void CSVExporter::exportMonteCarloResults(
    const std::vector<SimulationResult>& results, const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file '" << filename << "' for writing.\n";
        return;
    }

    file << "SimulationID,TotalRaceTime\n";
    for (std::size_t i = 0; i < results.size(); ++i) {
        file << i << ',' << results[i].totalRaceTime << '\n';
    }
}

} // namespace f1sim
