#include "runtime/function/framework/component/motor/motor_component.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/base/public_singleton.h"

#include "runtime/engine.h"
#include "runtime/function/controller/character_controller.h"
#include "runtime/function/framework/component/camera/camera_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/input/input_system.h"

namespace Pilot
{
    MotorComponent::MotorComponent(const MotorRes& motor_param, GObject* parent_object) :
        Component(parent_object), m_move_speed(motor_param.m_move_speed)
    {
        m_motor_res.m_move_speed = motor_param.m_move_speed;
        if (motor_param.m_controller_config.getTypeName() == "PhysicsControllerConfig")
        {
            auto controller_config                            = new PhysicsControllerConfig;
            m_motor_res.m_controller_config.getPtrReference() = controller_config;
            *controller_config = *static_cast<PhysicsControllerConfig*>(motor_param.m_controller_config.operator->());

            m_motor_res.m_controller_type = ControllerType::physics;
            m_controller                  = new CharacterController(controller_config->m_capsule_shape);
        }
        else if (motor_param.m_controller_config != nullptr)
        {
            m_motor_res.m_controller_type = ControllerType::invalid;
            LOG_ERROR("invalid controller type, not able to move");
        }
    }

    MotorComponent::~MotorComponent()
    {
        if (m_motor_res.m_controller_type == ControllerType::physics)
        {
            delete m_controller;
            m_controller = nullptr;
        }
    }

    void MotorComponent::tick(float delta_time)
    {
        if ((m_tick_in_editor_mode == false) && g_is_editor_mode)
            return;

        tickPlayerMotor(delta_time);
    }

    void MotorComponent::tickPlayerMotor(float delta_time)
    {
        TransformComponent* transform_component =
            m_parent_object->tryGetComponent<TransformComponent>("TransformComponent");

        Radian turn_angle_yaw = InputSystem::getInstance().m_cursor_delta_yaw;

        unsigned int command = InputSystem::getInstance().getGameCommand();

        if (command >= (unsigned int)GameCommand::invalid)
            return;

        calculatedDesiredMoveSpeedRatio(command);
        const Vector3&& desired_move_direction =
            calculatedDesiredMoveDirection(command, transform_component->getRotation());

        const Vector3 displacement = desired_move_direction * m_move_speed * m_move_speed_ratio * delta_time;

        Vector3 final_position = transform_component->getPosition();
        switch (m_motor_res.m_controller_type)
        {
            case ControllerType::none:
                final_position += displacement;
                break;
            case ControllerType::physics:
                final_position = m_controller->move(final_position, displacement);
                break;
            default:
                break;
        }
        transform_component->setPosition(final_position);
    }

    void MotorComponent::calculatedDesiredMoveSpeedRatio(unsigned int command)
    {
        m_move_speed_ratio = 1.0f;
        if ((unsigned int)GameCommand::sprint & command)
        {
            m_move_speed_ratio = 2.f;
        }
    }

    Vector3 MotorComponent::calculatedDesiredMoveDirection(unsigned int command, const Quaternion& object_rotation)
    {
        Vector3 forward_dir = object_rotation * Vector3::NEGATIVE_UNIT_Y;
        Vector3 left_dir    = object_rotation * Vector3::UNIT_X;

        Vector3 desired_move_direction;
        if ((unsigned int)GameCommand::forward & command)
        {
            desired_move_direction += forward_dir;
        }
        if ((unsigned int)GameCommand::backward & command)
        {
            desired_move_direction -= forward_dir;
        }
        if ((unsigned int)GameCommand::left & command)
        {
            desired_move_direction += left_dir;
        }
        if ((unsigned int)GameCommand::right & command)
        {
            desired_move_direction -= left_dir;
        }

        desired_move_direction.normalise();

        return desired_move_direction;
    }

} // namespace Pilot
