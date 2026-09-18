#pragma once

#include "f1sim/Car.hpp"
#include "f1sim/Strategy.hpp"
#include "f1sim/SimulationResult.hpp"
#include <string>
#include <vector>

namespace f1sim {

/**
 * @struct Competitor
 * @brief Aggregates a driver's name, car physics, and assigned race strategy.
 */
struct Competitor {
    std::string name;
    Car car;
    Strategy strategy;
};

/**
 * @enum TrackCondition
 * @brief Represents the current real-world state of the circuit.
 */
enum class TrackCondition {
    GreenFlag,
    VirtualSafetyCar,
    SafetyCar
};

/**
 * @class RaceSession
 * @brief Manages the execution flow of a single race simulation instance.
 */
class RaceSession final {
public:
    /**
     * @brief Initializes a race session.
     * @param competitors List of drivers participating in the simulation.
     * @param totalLaps Total number of laps in the Grand Prix.
     * @param startingLap The lap from which the simulation begins (useful for live mid-race predictions).
     */
    RaceSession(std::vector<Competitor> competitors, int totalLaps,
                int startingLap = 1) noexcept;

    /**
     * @brief Executes the simulation loop from the starting lap to the end.
     * @return SimulationResult containing lap-by-lap telemetry and final times for all competitors.
     */
    [[nodiscard]] SimulationResult run();

    /**
     * @brief Overrides the track state to force a Safety Car period (Live Predictor trigger).
     * @param active True to deploy the Safety Car, False to resume racing.
     * @param scLaps Number of laps the Safety Car will remain deployed.
     */
    void setInitialSafetyCar(bool active, int scLaps = 3) noexcept;

private:
    std::vector<Competitor> competitors_;
    int totalLaps_;
    int startingLap_;

    TrackCondition currentCondition_{TrackCondition::GreenFlag};
    int scLapsRemaining_{0};

    double scProbability_{0.02}; 
    double vscProbability_{0.03};
};

} // namespace f1sim