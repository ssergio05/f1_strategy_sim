#pragma once
#include <memory>
#include "f1sim/TyreModel.hpp"

namespace f1sim {

/**
 * @class Car
 * @brief Represents a Formula 1 car, managing fuel burn, tyre degradation, and lap time computation.
 */
class Car {
public:
    /**
     * @brief Constructs a new Car instance.
     * @param baseLapTime Theoretical best lap time (in seconds) with zero fuel and fresh tyres.
     * @param initialFuelMass Starting fuel load (in kg).
     * @param fuelDelta Time penalty (in seconds) added per kg of fuel.
     * @param tyreModel Shared pointer to the initial tyre compound model.
     */
    Car(double baseLapTime, double initialFuelMass, double fuelDelta,
        std::shared_ptr<TyreModel> tyreModel) noexcept;

    /**
     * @brief Computes the expected lap time for the current simulation step.
     * @param currentLap The absolute lap number of the race session.
     * @return The computed lap time in seconds.
     */
    double computeLapTime(double currentLap) noexcept;

    /**
     * @brief Updates the vehicle's fuel mass after completing a lap.
     */
    void updateFuel() noexcept;

    /**
     * @brief Executes a pit stop, replacing the current tyre compound.
     * @param newTyre Shared pointer to the newly fitted tyre model.
     */
    void changeTyre(std::shared_ptr<TyreModel> newTyre) noexcept;

    /**
     * @brief Injects live AWS telemetry data to override the car's state mid-race.
     * @param currentFuel Current estimated fuel mass (in kg).
     * @param currentTyreAge Laps completed on the currently fitted tyre.
     */
    void setMidRaceState(double currentFuel, double currentTyreAge) noexcept;

private:
    double baseLapTime_;
    double currentFuelMass_;
    double fuelDelta_;
    std::shared_ptr<TyreModel> tyreModel_;
    double lapsOnTyre_{0.0}; ///< Internal counter tracking the age of the current tyre set.
};

} // namespace f1sim