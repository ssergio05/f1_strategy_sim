#include <gtest/gtest.h>

#include "f1sim/Car.hpp"
#include "f1sim/Strategy.hpp"
#include "f1sim/TyreModel.hpp"
#include "f1sim/RaceSession.hpp"
#include "f1sim/MonteCarloEngine.hpp"
#include "f1sim/SimulationResult.hpp"

#include <memory>

namespace {

constexpr double TOL = 1e-5;

} // anonymous namespace

TEST(MonteCarloTest, ReturnsCorrectNumberOfSimulations)
{
    auto firstTyre = std::make_shared<f1sim::TyreModel>(0.3, 0.5, 0.1, 15.0);
    auto secondTyre = std::make_shared<f1sim::TyreModel>(0.4, 0.8, 0.2, 12.0);

    f1sim::Car car(85.0, 110.0, 0.035, firstTyre);

    f1sim::Strategy strategy{};
    strategy.pitLap = 20;
    strategy.firstTyre = firstTyre;
    strategy.secondTyre = secondTyre;

    const f1sim::RaceSession session(std::move(car), strategy, 60);

    const uint32_t numSims = 100;
    const auto results = f1sim::MonteCarloEngine::runSimulations(numSims, session);

    EXPECT_EQ(results.size(), numSims);

    for (const auto& res : results) {
        EXPECT_GT(res.totalRaceTime, 0.0);
        EXPECT_EQ(res.lapTimes.size(), 60);
    }
}
