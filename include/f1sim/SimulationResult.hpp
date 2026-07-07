#pragma once

#include <vector>

namespace f1sim {

struct SimulationResult {
    double totalRaceTime{0.0};
    std::vector<double> lapTimes;
};

} // namespace f1sim
