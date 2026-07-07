#pragma once

namespace f1sim {

class TyreModel final {
public:
    TyreModel(double alpha, double beta, double lambda, double cliffLap) noexcept;

    [[nodiscard]] double getDegradationTimeLoss(double currentLap) const noexcept;

private:
    double alpha_;
    double beta_;
    double lambda_;
    double cliffLap_;
};

} // namespace f1sim
