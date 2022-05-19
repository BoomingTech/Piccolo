#include "runtime/function/framework/component/motor/motor_component.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/base/public_singleton.h"

#include "runtime/engine.h"
#include "runtime/function/character/character.h"
#include "runtime/function/controller/character_controller.h"
#include "runtime/function/framework/component/animation/animation_component.h"
#include "runtime/function/framework/component/camera/camera_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"

namespace Pilot
{
    MotorComponent::MotorComponent(const MotorRes& motor_param, GObject* parent_object) :
        Component(parent_object), m_motor_res(motor_param)
    {
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

        const TransformComponent* transform_component = parent_object->tryGetComponentConst(TransformComponent);

        m_target_position = transform_component->getPosition();
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
        Level*     current_level     = WorldManager::getInstance().getCurrentActiveLevel();
        Character* current_character = current_level->getCurrentActiveCharacter();
        if (current_character == nullptr)
            return;

        if (current_character->getObject() != m_parent_object)
            return;

        TransformComponent* transform_component =
            m_parent_object->tryGetComponent<TransformComponent>("TransformComponent");

        Radian turn_angle_yaw = InputSystem::getInstance().m_cursor_delta_yaw;

        unsigned int command = InputSystem::getInstance().getGameCommand();

        if (command >= (unsigned int)GameCommand::invalid)
            return;

        calculatedDesiredHorizontalMoveSpeed(command, delta_time);
        calculatedDesiredVerticalMoveSpeed(command, delta_time);
        calculatedDesiredMoveDirection(command, transform_component->getRotation());
        calculateDesiredDisplacement(delta_time);
        calculateTargetPosition(transform_component->getPosition());

        transform_component->setPosition(m_target_position);

    }

    void MotorComponent::calculatedDesiredHorizontalMoveSpeed(unsigned int command, float delta_time)
    {
        bool has_move_command = ((unsigned int)GameCommand::forward | (unsigned int)GameCommand::backward |
                                 (unsigned int)GameCommand::left | (unsigned int)GameCommand::right) &
                                command;
        bool has_sprint_command = (unsigned int)GameCommand::sprint & command;

        bool  is_acceleration    = false;
        float final_acceleration = m_motor_res.m_move_acceleration;
        float min_speed_ratio    = 0.f;
        float max_speed_ratio    = 0.f;
        if (has_move_command)
        {
            is_acceleration = true;
            max_speed_ratio = m_motor_res.m_max_move_speed_ratio;
            if (m_move_speed_ratio >= m_motor_res.m_max_move_speed_ratio)
            {
                final_acceleration = m_motor_res.m_sprint_acceleration;
                is_acceleration    = has_sprint_command;
                min_speed_ratio    = m_motor_res.m_max_move_speed_ratio;
                max_speed_ratio    = m_motor_res.m_max_sprint_speed_ratio;
            }
        }
        else
        {
            is_acceleration = false;
            min_speed_ratio = 0.f;
            max_speed_ratio = m_motor_res.m_max_sprint_speed_ratio;
        }

        m_move_speed_ratio += (is_acceleration ? 1.0f : -1.0f) * final_acceleration * delta_time;
        m_move_speed_ratio = std::clamp(m_move_speed_ratio, min_speed_ratio, max_speed_ratio);
    }

    void MotorComponent::calculatedDesiredVerticalMoveSpeed(unsigned int command, float delta_time)
    {
    }

    void MotorComponent::calculatedDesiredMoveDirection(unsigned int command, const Quaternion& object_rotation)
    {
        if (m_jump_state == JumpState::idle)
        {
            Vector3 forward_dir = object_rotation * Vector3::NEGATIVE_UNIT_Y;
            Vector3 left_dir    = object_rotation * Vector3::UNIT_X;

            if (command > 0)
            {
                m_desired_horizontal_move_direction = Vector3::ZERO;
            }

            if ((unsigned int)GameCommand::forward & command)
            {
                m_desired_horizontal_move_direction += forward_dir;
            }

            if ((unsigned int)GameCommand::backward & command)
            {
                m_desired_horizontal_move_direction -= forward_dir;
            }

            if ((unsigned int)GameCommand::left & command)
            {
                m_desired_horizontal_move_direction += left_dir;
            }

            if ((unsigned int)GameCommand::right & command)
            {
                m_desired_horizontal_move_direction -= left_dir;
            }

            m_desired_horizontal_move_direction.normalise();
        }
    }

    void MotorComponent::calculateDesiredDisplacement(float delta_time)
    {
        float horizontal_speed_ratio =
            m_jump_state == JumpState::idle ? m_move_speed_ratio : m_jump_horizontal_speed_ratio;
        m_desired_displacement =
            m_desired_horizontal_move_direction * m_motor_res.m_move_speed * horizontal_speed_ratio * delta_time +
            Vector3::UNIT_Z * m_vertical_move_speed * delta_time;
    }

    void MotorComponent::calculateTargetPosition(const Vector3&& current_position)
    {
        Vector3 final_position = current_position;

        switch (m_motor_res.m_controller_type)
        {
            case ControllerType::none:
                final_position += m_desired_displacement;
                break;
            case ControllerType::physics:
                final_position = m_controller->move(final_position, m_desired_displacement);
                break;
            default:
                break;
        }

        // Pilot-hack: motor level simulating jump, character always above z-plane
        if (m_jump_state == JumpState::falling && m_target_position.z <= 0.f)
        {
            final_position.z = 0.f;
            m_jump_state     = JumpState::idle;
        }

        m_is_moving       = (final_position - current_position).squaredLength() > 0.f;
        m_target_position = final_position;
    }

} // namespace Pilot
