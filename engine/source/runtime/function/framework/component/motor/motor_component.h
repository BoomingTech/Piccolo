#pragma once

#include "runtime/resource/res_type/components/motor.h"

#include "runtime/function/controller/character_controller.h"
#include "runtime/function/framework/component/component.h"

namespace Pilot
{
    REFLECTION_TYPE(MotorComponent)
    CLASS(MotorComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(MotorComponent)
    public:
        MotorComponent() {}
        MotorComponent(const MotorRes& motor_param, GObject* parent_object);

        ~MotorComponent() override;

        Vector3 getDesiredPosition() const { return m_displacement; }

        void tick(float delta_time) override;
        void tickPlayerMotor(float delta_time);
        void destroy() override {}

    private:
        void    calculatedDesiredMoveSpeedRatio(unsigned int command);
        Vector3 calculatedDesiredMoveDirection(unsigned int command, const Quaternion& object_rotation);

        META(Enable)
        MotorRes m_motor_res;

        float   m_move_speed {0.f};
        Vector3 m_displacement;

        Controller* m_controller {nullptr};
        Vector3     m_desired_move_direction;
        float       m_move_speed_ratio {0.f};
    };
} // namespace Pilot
