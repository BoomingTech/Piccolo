#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Pilot
{
    REFLECTION_TYPE(MotorRes)
    CLASS(MotorRes, Fields)
    {
        REFLECTION_BODY(MotorRes);

    public:
        float   m_move_speed;
        Vector3 m_half_extent;
    };
} // namespace Pilot