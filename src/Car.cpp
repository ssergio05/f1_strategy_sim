#include "f1sim/Car.hpp"
#include "f1sim/TyreModel.hpp"

namespace f1sim {

Car::Car(double baseLapTime, double initialFuelMass, double fuelDelta,
         std::shared_ptr<TyreModel> tyreModel) noexcept
    : baseLapTime_(baseLapTime)
    , currentFuelMass_(initialFuelMass)
    , fuelDelta_(fuelDelta)
    , tyreModel_(std::move(tyreModel))
    , lapsOnTyre_(0.0)
{
}

double Car::computeLapTime(double /*currentLap*/) noexcept
{
    // 1. Calculate base time penalty due to fuel load and tyre degradation
    double lapTime = baseLapTime_ - (fuelDelta_ * currentFuelMass_)
                   + tyreModel_->getDegradationTimeLoss(lapsOnTyre_);

    // 2. Increment tyre age for the upcoming lap
    lapsOnTyre_ += 1.0;

    return lapTime;
}

void Car::updateFuel() noexcept
{
    currentFuelMass_ -= fuelDelta_;
}

void Car::changeTyre(std::shared_ptr<TyreModel> newTyre) noexcept
{
    tyreModel_ = std::move(newTyre);
    lapsOnTyre_ = 0.0; // CRITICAL: Reset tyre age counter upon pitting
}

void Car::setMidRaceState(double currentFuel, double currentTyreAge) noexcept
{
    currentFuelMass_ = currentFuel;
    lapsOnTyre_ = currentTyreAge;
}

} // namespace f1sim