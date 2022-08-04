#pragma once

#include <cstdint>

#include "core/math/vector3.h"

namespace Piccolo
{
    class PhysicsConfig
    {
    public:
        // scene setting
        uint32_t m_max_body_count {10240};
        uint32_t m_body_mutex_count {0} ;
        uint32_t m_max_body_pairs {65536};
        uint32_t m_max_contact_constraints {10240};

        // job setting
        uint32_t m_max_job_count {1024};
        uint32_t m_max_barrier_count {8};
        uint32_t m_max_concurrent_job_count {4};

        Vector3 m_gravity {0.f, 0.f, -9.8f};

        float m_update_frequency {60.f};
    };
} // namespace Piccolo
