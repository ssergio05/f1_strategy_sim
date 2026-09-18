#include "f1sim/CSVExporter.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace f1sim {

void CSVExporter::exportLapTimes(const SimulationResult& result, const std::string& filename,
                                 int startingLap)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file '" << filename << "' for writing.\n";
        return;
    }

    if (result.competitorResults.empty()) return;
    
    size_t numCompetitors = result.competitorResults.size();
    size_t numLaps = result.competitorResults[0].lapTimes.size();

    // 1. Write dynamic CSV header
    file << "Lap";
    for (size_t c = 0; c < numCompetitors; ++c) {
        file << ",Comp" << c;
    }
    file << "\n";

    // 2. Dump telemetry matrix
    for (size_t lapIdx = 0; lapIdx < numLaps; ++lapIdx) {
        file << (lapIdx + startingLap);
        for (size_t c = 0; c < numCompetitors; ++c) {
            file << "," << result.competitorResults[c].lapTimes[lapIdx];
        }
        file << "\n";
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

    if (results.empty() || results[0].competitorResults.empty()) return;
    
    size_t numCompetitors = results[0].competitorResults.size();

    // Pre-allocate buffer to minimize I/O bottleneck during massive Monte Carlo dumps
    std::string buffer;
    buffer.reserve(results.size() * (15 + numCompetitors * 15)); 

    // Header construction
    buffer += "SimulationID";
    for (size_t c = 0; c < numCompetitors; ++c) {
        buffer += ",Comp" + std::to_string(c) + "_TotalTime";
    }
    buffer += "\n";

    // Build data matrix in RAM
    for (std::size_t i = 0; i < results.size(); ++i) {
        buffer += std::to_string(i);
        for (size_t c = 0; c < numCompetitors; ++c) {
            buffer += "," + std::to_string(results[i].competitorResults[c].totalRaceTime);
        }
        buffer += "\n";
    }

    // Single flush to disk
    file.write(buffer.data(), buffer.size());
}

} // namespace f1sim