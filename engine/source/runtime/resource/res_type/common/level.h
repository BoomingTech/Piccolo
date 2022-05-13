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
        std::string m_character_name;

        std::vector<ObjectInstanceRes> m_objects;
    };
} // namespace Pilot