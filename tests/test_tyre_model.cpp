#include <gtest/gtest.h>
#include "f1sim/TyreModel.hpp"
#include <cmath>

namespace {
constexpr double ALPHA = 0.5;
constexpr double BETA = 2.0;
constexpr double LAMBDA = 0.3;
constexpr double CLIFF_LAP = 10.0;
constexpr double TOL = 1e-5;
} // anonymous namespace

TEST(TyreModelTest, DegradationBeforeCliffLap)
{
    const f1sim::TyreModel tyre(ALPHA, BETA, LAMBDA, CLIFF_LAP);
    const double result = tyre.getDegradationTimeLoss(5.0);
    const double expected = ALPHA * 5.0;
    EXPECT_NEAR(result, expected, TOL);
}

TEST(TyreModelTest, DegradationAtCliffLap)
{
    const f1sim::TyreModel tyre(ALPHA, BETA, LAMBDA, CLIFF_LAP);
    const double result = tyre.getDegradationTimeLoss(CLIFF_LAP);
    const double expected = ALPHA * CLIFF_LAP;
    EXPECT_NEAR(result, expected, TOL);
}

TEST(TyreModelTest, DegradationAfterCliffLap)
{
    const f1sim::TyreModel tyre(ALPHA, BETA, LAMBDA, CLIFF_LAP);
    const double lap = 12.0;
    const double result = tyre.getDegradationTimeLoss(lap);
    
    // Corected mathematical expectation to match C++ implementation std::expm1
    const double expected = (ALPHA * lap) + (BETA * std::expm1(LAMBDA * (lap - CLIFF_LAP)));
    EXPECT_NEAR(result, expected, TOL);
}