#include "f1sim/TyreModel.hpp"

#include <cmath>

namespace f1sim {

TyreModel::TyreModel(double alpha, double beta, double lambda, double cliffLap) noexcept
    : alpha_(alpha)
    , beta_(beta)
    , lambda_(lambda)
    , cliffLap_(cliffLap)
{
}

double TyreModel::getDegradationTimeLoss(double currentLap) const noexcept
{
    double loss = alpha_ * currentLap;
    if (currentLap > cliffLap_) {
        loss += beta_ * std::exp(lambda_ * (currentLap - cliffLap_));
    }
    return loss;
}

} // namespace f1sim
