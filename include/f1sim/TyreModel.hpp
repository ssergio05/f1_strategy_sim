#pragma once

namespace f1sim {

/**
 * @class TyreModel
 * @brief Mathematical representation of tyre degradation behavior over time.
 */
class TyreModel final {
public:
    /**
     * @brief Constructs a tyre degradation model based on empirical compound data.
     * @param alpha Initial grip scalar.
     * @param beta Linear degradation coefficient.
     * @param lambda Exponential degradation factor (thermal degradation).
     * @param cliffLap The lap count at which the tyre experiences a critical loss of performance.
     */
    TyreModel(double alpha, double beta, double lambda, double cliffLap) noexcept;

    /**
     * @brief Calculates the time penalty introduced by tyre wear.
     * @param currentLap The current age of the tyre (in laps).
     * @return Time loss in seconds.
     */
    [[nodiscard]] double getDegradationTimeLoss(double currentLap) const noexcept;

private:
    double alpha_;
    double beta_;
    double lambda_;
    double cliffLap_;
};

} // namespace f1sim