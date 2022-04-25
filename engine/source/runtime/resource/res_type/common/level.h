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
        float m_gravity {9.8f};
        int   m_character_index {0};

        std::vector<ObjectInstanceRes> m_objects;
    };
} // namespace Pilot