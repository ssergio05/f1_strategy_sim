#pragma once

#include <memory>

namespace f1sim {

class TyreModel;

/**
 * @struct Strategy
 * @brief Defines the pit-stop plan and compound allocation for a competitor.
 */
struct Strategy {
    int pitLap{0}; ///< The target lap for the scheduled pit stop.

    std::shared_ptr<TyreModel> firstTyre;  ///< Compound used from the start (or current live stint).
    std::shared_ptr<TyreModel> secondTyre; ///< Compound to be fitted during the pit stop.
};

} // namespace f1sim