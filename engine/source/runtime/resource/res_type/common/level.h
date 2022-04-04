#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/resource/res_type/common/object.h"

namespace Pilot
{
    REFLECTION_TYPE(LevelRes)
    CLASS(LevelRes, Fields)
    {
        REFLECTION_BODY(LevelRes);

    public:
        std::vector<ObjectInstanceRes> m_objects;
    };
} // namespace Pilot