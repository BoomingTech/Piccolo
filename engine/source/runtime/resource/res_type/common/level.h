#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/core/math/vector3.h"

#include "runtime/resource/res_type/common/object.h"

namespace Piccolo
{
    REFLECTION_TYPE(LevelRes)
    CLASS(LevelRes, Fields)
    {
        REFLECTION_BODY(LevelRes);

    public:
        Vector3     m_gravity {0.f, 0.f, -9.8f};
        std::string m_character_name;

        std::vector<ObjectInstanceRes> m_objects;
    };
} // namespace Piccolo