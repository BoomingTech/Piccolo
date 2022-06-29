#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/resource/res_type/components/rigid_body.h"
#include "runtime/resource/res_type/data/basic_shape.h"

namespace Piccolo
{
    enum SweepPass
    {
        SWEEP_PASS_UP,
        SWEEP_PASS_SIDE,
        SWEEP_PASS_DOWN,
        SWEEP_PASS_SENSOR
    };

    class Controller
    {
    public:
        virtual ~Controller() = default;

        virtual Vector3 move(const Vector3& current_position, const Vector3& displacement) = 0;
    };

    class CharacterController : public Controller
    {
    public:
        CharacterController(const Capsule& capsule);
        ~CharacterController() = default;

        Vector3 move(const Vector3& current_position, const Vector3& displacement) override;

    private:
        Capsule        m_capsule;
        RigidBodyShape m_rigidbody_shape;
    };
} // namespace Piccolo
