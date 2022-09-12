#pragma once
#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/vector2.h"
#include "runtime/core/math/vector4.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(ParticleComponentRes)
        CLASS(ParticleComponentRes, Fields)
    {
        REFLECTION_BODY(ParticleComponentRes);

    public:
        Vector3    m_local_translation; // local translation
        Quaternion m_local_rotation;    // local rotation
        Vector4    m_velocity;          // velocity base & variance
        Vector4    m_acceleration;      // acceleration base & variance
        Vector3    m_size;              // size base & variance
        int        m_emitter_type;
        Vector2    m_life;  // life base & variance
        Vector4    m_color; // color rgba
    };
} // namespace Piccolo