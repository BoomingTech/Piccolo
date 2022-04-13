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

    REFLECTION_TYPE(MotorRes)
    CLASS(MotorRes, WhiteListFields)
    {
        REFLECTION_BODY(MotorRes);

    public:
        ~MotorRes() { PILOT_REFLECTION_DELETE(m_controller_config); }

        ControllerType m_controller_type {ControllerType::none};

        META(Enable)
        float m_move_speed;
        META(Enable)
        Reflection::ReflectionPtr<ControllerConfig> m_controller_config;
    };
} // namespace Pilot