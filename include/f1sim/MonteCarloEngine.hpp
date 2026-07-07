#pragma once

#include "f1sim/SimulationResult.hpp"
#include "f1sim/RaceSession.hpp"

#include <vector>
#include <cstdint>

namespace f1sim {

class MonteCarloEngine final {
public:
    [[nodiscard]] static std::vector<SimulationResult> runSimulations(
        uint32_t numSimulations, const RaceSession& baseSession);
};

} // namespace f1sim
