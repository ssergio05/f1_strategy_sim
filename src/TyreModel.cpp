#include "f1sim/TyreModel.hpp"

#include <cmath>
#include <algorithm> 

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
    // Linear base degradation
    double loss = alpha_ * currentLap;

    // Apply exponential thermal degradation penalty if the cliff threshold is surpassed
    if (currentLap > cliffLap_) {
        double excess = currentLap - cliffLap_;
        double cliffLoss = beta_ * std::expm1(lambda_ * excess); 
        
        // Optional: Cap the maximum time loss per lap to prevent unrealistic infinite degradation
        // cliffLoss = std::min(cliffLoss, 8.0); 

        loss += cliffLoss;
    }
    
    return loss;
}

} // namespace f1sim