#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>
namespace Piccolo
{

    REFLECTION_TYPE(AnimSkelMap)
    CLASS(AnimSkelMap, Fields)
    {
        REFLECTION_BODY(AnimSkelMap);

    public:
        std::vector<int> convert;
    };

} // namespace Piccolo