#pragma once

#include <vector>

namespace f1sim {

/**
 * @struct CompetitorResult
 * @brief Encapsulates the performance data of a single driver within a simulation.
 */
struct CompetitorResult {
    double totalRaceTime{0.0};      ///< Cumulative race time (in seconds).
    std::vector<double> lapTimes;   ///< Historical array of individual lap times.
};

/**
 * @struct SimulationResult
 * @brief Aggregates the final standings and telemetry for all competitors in a race instance.
 */
struct SimulationResult {
    std::vector<CompetitorResult> competitorResults; ///< Results mapped per competitor.
};

} // namespace f1sim