#pragma once

#include <memory>

namespace f1sim {

class TyreModel;

struct Strategy {
    int pitLap{0};

    std::shared_ptr<TyreModel> firstTyre;
    std::shared_ptr<TyreModel> secondTyre;
};

} // namespace f1sim
