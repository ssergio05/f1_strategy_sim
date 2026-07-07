#pragma once

#include <memory>

namespace f1sim {

class TyreModel;

class Car final {
public:
    Car(double baseLapTime, double initialFuelMass, double fuelDelta,
        std::shared_ptr<TyreModel> tyreModel) noexcept;

    [[nodiscard]] double computeLapTime(double currentLap) const noexcept;

    void updateFuel() noexcept;

    void changeTyre(std::shared_ptr<TyreModel> newTyre) noexcept;

private:
    double baseLapTime_;
    double currentFuelMass_;
    double fuelDelta_;
    std::shared_ptr<TyreModel> tyreModel_;
};

} // namespace f1sim
