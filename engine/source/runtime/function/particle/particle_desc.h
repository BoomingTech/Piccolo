#pragma once

#include "runtime/function/particle/emitter_id_allocator.h"

#include "runtime/resource/res_type/components/emitter.h"

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/vector2.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

namespace Piccolo
{
    struct ParticleEmitterTransformDesc
    {
        ParticleEmitterID m_id;
        Vector4           m_position;
        Matrix4x4         m_rotation;
    };

    struct ParticleEmitterDesc
    {
        Vector4   m_position;
        Matrix4x4 m_rotation;
        Vector4   m_velocity;
        Vector4   m_acceleration;
        Vector3   m_size;
        int       m_emitter_type;
        Vector2   m_life;
        Vector2   m_padding;
        Vector4   m_color;

        ParticleEmitterDesc() = default;

        ParticleEmitterDesc(const ParticleComponentRes& component_res, ParticleEmitterTransformDesc& transform_desc) :
            m_position(transform_desc.m_position), m_rotation(transform_desc.m_rotation),
            m_velocity(component_res.m_velocity), m_acceleration(component_res.m_acceleration),
            m_size(component_res.m_size), m_emitter_type(component_res.m_emitter_type), m_life(component_res.m_life),
            m_color(component_res.m_color)
        {}
    };
} // namespace Piccolo
