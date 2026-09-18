#include "f1sim/RaceSession.hpp"
#include "f1sim/TyreModel.hpp"

#include <random>

namespace f1sim {

RaceSession::RaceSession(std::vector<Competitor> competitors, int totalLaps,
                         int startingLap) noexcept
    : competitors_(std::move(competitors))
    , totalLaps_(totalLaps)
    , startingLap_(startingLap)
{
}

void RaceSession::setInitialSafetyCar(bool active, int scLaps) noexcept
{
    if (active) {
        currentCondition_ = TrackCondition::SafetyCar;
        scLapsRemaining_ = scLaps;
    }
}

SimulationResult RaceSession::run()
{
    SimulationResult result;
    
    // 1. Pre-allocate memory to avoid dynamic reallocations during simulation
    result.competitorResults.resize(competitors_.size());
    const int remainingLaps = totalLaps_ - startingLap_ + 1;
    for (auto& cr : result.competitorResults) {
        cr.lapTimes.reserve(remainingLaps);
    }

    thread_local std::minstd_rand rng(std::random_device{}());
    thread_local std::uniform_real_distribution<double> noise(-0.1, 0.2);   
    thread_local std::uniform_real_distribution<double> eventDist(0.0, 1.0);
    thread_local std::uniform_int_distribution<int> scLapsDist(3, 5);
    thread_local std::uniform_int_distribution<int> vscLapsDist(1, 2);

    // 2. Equip starting tyres (Ignored if injecting mid-race live data)
    if (startingLap_ == 1) {
        for (auto& comp : competitors_) {
            comp.car.changeTyre(comp.strategy.firstTyre);
        }
    }

    // 3. MAIN LOOP: Advance the session lap by lap
    for (int lap = startingLap_; lap <= totalLaps_; ++lap) {
        
        // --- A. TRACK MANAGEMENT (Global events) ---
        if (currentCondition_ != TrackCondition::GreenFlag) {
            scLapsRemaining_--;
            if (scLapsRemaining_ <= 0) {
                currentCondition_ = TrackCondition::GreenFlag;
            }
        } else {
            double roll = eventDist(rng);
            if (roll < scProbability_) {
                currentCondition_ = TrackCondition::SafetyCar;
                scLapsRemaining_ = scLapsDist(rng);
            } else if (roll < (scProbability_ + vscProbability_)) {
                currentCondition_ = TrackCondition::VirtualSafetyCar;
                scLapsRemaining_ = vscLapsDist(rng);
            }
        }

        // Apply track conditions (FIXED LOGIC)
        double conditionTimePenalty = 0.0;
        double pitLoss = 25.0; // Standard green-flag pit loss

        if (currentCondition_ == TrackCondition::SafetyCar) {
            conditionTimePenalty = 30.0; // Laps are significantly slower
            pitLoss = 12.0;              // "Cheap" pit stop delta
        } else if (currentCondition_ == TrackCondition::VirtualSafetyCar) {
            conditionTimePenalty = 15.0; // VSC delta time
            pitLoss = 16.0;              // "Cheap" pit stop delta
        }

        // --- B. CAR MANAGEMENT (Individual telemetry) ---
        for (size_t i = 0; i < competitors_.size(); ++i) {
            auto& comp = competitors_[i];
            auto& compResult = result.competitorResults[i];

            double lapTime = comp.car.computeLapTime(static_cast<double>(lap));
            lapTime += noise(rng);
            lapTime += conditionTimePenalty; 

            // Execute pit strategy
            if (lap == comp.strategy.pitLap) {
                lapTime += pitLoss;
                comp.car.changeTyre(comp.strategy.secondTyre);
            }

            compResult.lapTimes.push_back(lapTime);
            compResult.totalRaceTime += lapTime;

            comp.car.updateFuel();
        }
    }

    return result;
}

} // namespace f1sim