#pragma once

#include "f1sim/SimulationResult.hpp"
#include "f1sim/RaceSession.hpp"

#include <vector>
#include <cstdint>

namespace f1sim {

/**
 * @class MonteCarloEngine
 * @brief Executes parallel race sessions to generate probabilistic strategy distributions.
 */
class MonteCarloEngine final {
public:
    /**
     * @brief Runs multiple variations of a race session introducing random variables (e.g., traffic, VSC).
     * @param numSimulations Number of iterations to compute.
     * @param baseSession The baseline race session configuration.
     * @return A collection of simulation results for statistical analysis.
     */
    [[nodiscard]] static std::vector<SimulationResult> runSimulations(
        uint32_t numSimulations, const RaceSession& baseSession);
};

} // namespace f1sim