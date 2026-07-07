#pragma once

#include "f1sim/Car.hpp"
#include "f1sim/Strategy.hpp"
#include "f1sim/SimulationResult.hpp"

namespace f1sim {

class RaceSession final {
public:
    RaceSession(Car car, Strategy strategy, int totalLaps) noexcept;

    [[nodiscard]] SimulationResult run();

private:
    Car car_;
    Strategy strategy_;
    int totalLaps_;
};

} // namespace f1sim
