#pragma once

#include "f1sim/SimulationResult.hpp"

#include <string>
#include <vector>

namespace f1sim {

class CSVExporter final {
public:
    CSVExporter() = delete;

    static void exportLapTimes(const SimulationResult& result, const std::string& filename);

    static void exportMonteCarloResults(const std::vector<SimulationResult>& results,
                                        const std::string& filename);
};

} // namespace f1sim
