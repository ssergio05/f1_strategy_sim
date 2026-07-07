#include "f1sim/Car.hpp"
#include "f1sim/TyreModel.hpp"

namespace f1sim {

Car::Car(double baseLapTime, double initialFuelMass, double fuelDelta,
         std::shared_ptr<TyreModel> tyreModel) noexcept
    : baseLapTime_(baseLapTime)
    , currentFuelMass_(initialFuelMass)
    , fuelDelta_(fuelDelta)
    , tyreModel_(std::move(tyreModel))
{
}

double Car::computeLapTime(double currentLap) const noexcept
{
    return baseLapTime_ - (fuelDelta_ * currentFuelMass_)
         + tyreModel_->getDegradationTimeLoss(currentLap);
}

void Car::updateFuel() noexcept
{
    currentFuelMass_ -= fuelDelta_;
}

void Car::changeTyre(std::shared_ptr<TyreModel> newTyre) noexcept
{
    tyreModel_ = std::move(newTyre);
}

} // namespace f1sim
