#include <gtest/gtest.h>
#include "f1sim/Car.hpp"
#include "f1sim/TyreModel.hpp"
#include <memory>

namespace {
constexpr double BASE_LAP_TIME = 90.0;
constexpr double INITIAL_FUEL = 100.0;
constexpr double FUEL_DELTA = 0.03;
constexpr double TOL = 1e-5;
} // anonymous namespace

TEST(CarTest, ComputeLapTime)
{
    auto tyre = std::make_shared<f1sim::TyreModel>(0.5, 0.0, 0.0, 50.0);
    f1sim::Car car(BASE_LAP_TIME, INITIAL_FUEL, FUEL_DELTA, tyre);

    const double result = car.computeLapTime(1.0);
    const double expected = BASE_LAP_TIME - (FUEL_DELTA * INITIAL_FUEL);

    EXPECT_NEAR(result, expected, TOL);
}

TEST(CarTest, UpdateFuel)
{
    // Cambiamos el parámetro ALPHA a 0.0 para aislar el efecto del combustible
    auto tyre = std::make_shared<f1sim::TyreModel>(0.0, 0.0, 0.0, 50.0);
    f1sim::Car car(BASE_LAP_TIME, INITIAL_FUEL, FUEL_DELTA, tyre);

    const double before = car.computeLapTime(1.0); 
    car.updateFuel();
    const double after = car.computeLapTime(1.0);  

    const double expectedDiff = FUEL_DELTA * FUEL_DELTA;
    EXPECT_NEAR(after - before, expectedDiff, TOL);
}

TEST(CarTest, ChangeTyre)
{
    auto highDegTyre = std::make_shared<f1sim::TyreModel>(1.0, 0.0, 0.0, 50.0);
    auto lowDegTyre = std::make_shared<f1sim::TyreModel>(0.1, 0.0, 0.0, 50.0);

    f1sim::Car car(BASE_LAP_TIME, INITIAL_FUEL, FUEL_DELTA, highDegTyre);

    const double before = car.computeLapTime(1.0); 
    car.changeTyre(lowDegTyre);
    const double after = car.computeLapTime(1.0); 

    const double expectedBefore = BASE_LAP_TIME - (FUEL_DELTA * INITIAL_FUEL);
    const double expectedAfter = BASE_LAP_TIME - (FUEL_DELTA * INITIAL_FUEL);

    EXPECT_NEAR(before, expectedBefore, TOL);
    EXPECT_NEAR(after, expectedAfter, TOL);
}