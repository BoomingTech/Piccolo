#pragma once

#include "runtime/core/math/vector3.h"

namespace Piccolo
{
    struct RayCollision
    {
        void*   m_node;
        Vector3 m_collided_point;
        float   m_ray_distance;

        RayCollision(void* node, Vector3 collided_point)
        {
            this->m_node           = node;
            this->m_collided_point = collided_point;
            this->m_ray_distance   = 0.0f;
        }

        RayCollision()
        {
            m_node         = nullptr;
            m_ray_distance = FLT_MAX;
        }
    };

    class Ray
    {
    public:
        Ray(Vector3 start_point, Vector3 direction);
        ~Ray();

        Vector3 getStartPoint() const;
        Vector3 getDirection() const;

    private:
        Vector3 m_start_point;
        Vector3 m_direction;
    };
} // namespace Piccolo