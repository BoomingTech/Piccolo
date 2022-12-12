#pragma once

#include "runtime/resource/res_type/components/motor.h"

#include "runtime/function/controller/character_controller.h"
#include "runtime/function/framework/component/component.h"

namespace Piccolo
{
    enum class MotorState : unsigned char
    {
        moving,
        jumping
    };

    enum class JumpState : unsigned char
    {
        idle,
        rising,
        falling
    };

    REFLECTION_TYPE(MotorComponent)
    CLASS(MotorComponent : public Component, WhiteListFields,WhiteListMethods)
    {
        REFLECTION_BODY(MotorComponent)
    public:
        MotorComponent() = default;

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        ~MotorComponent() override;

        void tick(float delta_time) override;
        void tickPlayerMotor(float delta_time);

        const Vector3& getTargetPosition() const { return m_target_position; }

        float getSpeedRatio() const { return m_move_speed_ratio; }
        bool  getIsMoving() const { return m_is_moving; }

        META(Enable)
        void getOffStuckDead();

    private:
        void calculatedDesiredHorizontalMoveSpeed(unsigned int command, float delta_time);
        void calculatedDesiredVerticalMoveSpeed(unsigned int command, float delta_time);
        void calculatedDesiredMoveDirection(unsigned int command, const Quaternion& object_rotation);
        void calculateDesiredDisplacement(float delta_time);
        void calculateTargetPosition(const Vector3&& current_position);

        META(Enable)
        MotorComponentRes m_motor_res;

        float m_move_speed_ratio {0.f};
        float m_vertical_move_speed {0.f};
        float m_jump_horizontal_speed_ratio {0.f};

        Vector3 m_desired_displacement;
        Vector3 m_desired_horizontal_move_direction;
        Vector3 m_jump_initial_velocity;
        Vector3 m_target_position;

        MotorState m_motor_state {MotorState::moving};
        JumpState  m_jump_state {JumpState::idle};

        ControllerType m_controller_type {ControllerType::none};
        Controller*    m_controller {nullptr};

        META(Enable)
        bool m_is_moving {false};
    };
} // namespace Piccolo
