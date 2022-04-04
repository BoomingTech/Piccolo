#pragma once

#include "runtime/core/math/vector3.h"

namespace Pilot
{
    class GObject;
    enum SweepPass
    {
        SWEEP_PASS_UP,
        SWEEP_PASS_SIDE,
        SWEEP_PASS_DOWN,
        SWEEP_PASS_SENSOR
    };

    class CCTCapsule
    {
    public:
        CCTCapsule(float radius, float height) : m_radius(radius), m_height(height) {}
        ~CCTCapsule() {}

    protected:
        float m_radius;
        float m_height;
    };

    class CharacterController
    {
    public:
        CharacterController(const Vector3& half_extent) : m_half_extent(half_extent) {}
        ~CharacterController();

        Vector3 move(const Vector3& current_position, const Vector3& displacement);

    private:
        Vector3 m_half_extent;
    };
} // namespace Pilot
