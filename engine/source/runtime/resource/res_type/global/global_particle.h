#pragma once
#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(GlobalParticleRes)

    CLASS(GlobalParticleRes, Fields)
    {
        REFLECTION_BODY(GlobalParticleRes)

    public:
        GlobalParticleRes() {}

        META(Enable)
        int         m_emit_gap;
        int         m_emit_count;
        float       m_time_step;
        float       m_max_life;
        Vector3     m_gravity;
        std::string m_particle_billboard_texture_path;
        std::string m_piccolo_logo_texture_path;
    };
} // namespace Piccolo