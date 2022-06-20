#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/basic_shape.h"

namespace Piccolo
{
    enum class ControllerType : unsigned char
    {
        none,
        physics,
        invalid
    };

    REFLECTION_TYPE(ControllerConfig)
    CLASS(ControllerConfig, Fields)
    {
        REFLECTION_BODY(ControllerConfig);

    public:
        virtual ~ControllerConfig() {}
    };

    REFLECTION_TYPE(PhysicsControllerConfig)
    CLASS(PhysicsControllerConfig : public ControllerConfig, Fields)
    {
        REFLECTION_BODY(PhysicsControllerConfig);

    public:
        PhysicsControllerConfig() {}
        ~PhysicsControllerConfig() {}
        Capsule m_capsule_shape;
    };

    REFLECTION_TYPE(MotorComponentRes)
    CLASS(MotorComponentRes, Fields)
    {
        REFLECTION_BODY(MotorComponentRes);

    public:
        MotorComponentRes() = default;
        ~MotorComponentRes();

        float m_move_speed { 0.f};
        float m_jump_height {0.f};
        float m_max_move_speed_ratio { 0.f};
        float m_max_sprint_speed_ratio { 0.f};
        float m_move_acceleration {0.f};
        float m_sprint_acceleration { 0.f};

        Reflection::ReflectionPtr<ControllerConfig> m_controller_config;
    };
} // namespace Piccolo