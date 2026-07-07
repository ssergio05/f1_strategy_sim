#include "f1sim/RaceSession.hpp"
#include "f1sim/TyreModel.hpp"

#include <random>

namespace f1sim {

RaceSession::RaceSession(Car car, Strategy strategy, int totalLaps) noexcept
    : car_(std::move(car))
    , strategy_(strategy)
    , totalLaps_(totalLaps)
{
}

SimulationResult RaceSession::run()
{
    SimulationResult result;
    result.lapTimes.reserve(totalLaps_);

    thread_local std::mt19937 rng(std::random_device{}());
    thread_local std::uniform_real_distribution<double> noise(-0.1, 0.2);

    car_.changeTyre(strategy_.firstTyre);

    for (int lap = 1; lap <= totalLaps_; ++lap) {
        double lapTime = car_.computeLapTime(static_cast<double>(lap));
        lapTime += noise(rng);

        if (lap == strategy_.pitLap) {
            lapTime += 25.0;
            car_.changeTyre(strategy_.secondTyre);
        }

        result.lapTimes.push_back(lapTime);
        result.totalRaceTime += lapTime;

        car_.updateFuel();
    }

    return result;
}

} // namespace f1sim
