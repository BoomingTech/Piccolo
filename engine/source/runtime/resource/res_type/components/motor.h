#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/basic_shape.h"

namespace Pilot
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
        MotorComponentRes(const MotorComponentRes& res);
        ~MotorComponentRes();

        META(Disable)
        ControllerType m_controller_type {ControllerType::none};

        float m_move_speed;
        float m_jump_height;
        float m_max_move_speed_ratio;
        float m_max_sprint_speed_ratio;
        float m_move_acceleration;
        float m_sprint_acceleration;

        Reflection::ReflectionPtr<ControllerConfig> m_controller_config;
    };
} // namespace Pilot