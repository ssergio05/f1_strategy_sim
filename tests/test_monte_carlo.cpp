#include <gtest/gtest.h>
#include "f1sim/TyreModel.hpp"
#include "f1sim/Car.hpp"
#include "f1sim/Strategy.hpp"
#include "f1sim/RaceSession.hpp"
#include "f1sim/MonteCarloEngine.hpp"

using namespace f1sim;

TEST(MonteCarloTest, BasicSimulationRun) {
    const double baseLapTime = 90.0;
    const double initialFuelMass = 110.0;
    const double fuelDelta = 0.03;
    const int totalLaps = 10;

    auto tyre = std::make_shared<TyreModel>(0.1, 0.05, 0.2, 15.0);
    Car car(baseLapTime, initialFuelMass, fuelDelta, tyre);
    Strategy strategy{5, tyre, tyre};

    std::vector<Competitor> grid;
    grid.push_back({"TestDriver", car, strategy});

    RaceSession session(grid, totalLaps);
    SimulationResult result = session.run();

    ASSERT_FALSE(result.competitorResults.empty());
    const auto& compResult = result.competitorResults[0];

    EXPECT_GT(compResult.totalRaceTime, 0.0);
    EXPECT_EQ(compResult.lapTimes.size(), static_cast<size_t>(totalLaps));
}