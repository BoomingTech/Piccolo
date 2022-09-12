#include "runtime/function/framework/component/camera/camera_component.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/math/math_headers.h"

#include "runtime/function/character/character.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/input/input_system.h"

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_swap_context.h"
#include "runtime/function/render/render_system.h"

namespace Piccolo
{
    void CameraComponent::postLoadResource(std::weak_ptr<GObject> parent_object)
    {
        m_parent_object = parent_object;

        const std::string& camera_type_name = m_camera_res.m_parameter.getTypeName();
        if (camera_type_name == "FirstPersonCameraParameter")
        {
            m_camera_mode = CameraMode::first_person;
        }
        else if (camera_type_name == "ThirdPersonCameraParameter")
        {
            m_camera_mode = CameraMode::third_person;
        }
        else if (camera_type_name == "FreeCameraParameter")
        {
            m_camera_mode = CameraMode::free;
        }
        else
        {
            LOG_ERROR("invalid camera type");
        }

        RenderSwapContext& swap_context = g_runtime_global_context.m_render_system->getSwapContext();
        CameraSwapData     camera_swap_data;
        camera_swap_data.m_fov_x                           = m_camera_res.m_parameter->m_fov;
        swap_context.getLogicSwapData().m_camera_swap_data = camera_swap_data;
    }

    void CameraComponent::tick(float delta_time)
    {
        if (!m_parent_object.lock())
            return;

        std::shared_ptr<Level> current_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
        std::shared_ptr<Character> current_character = current_level->getCurrentActiveCharacter().lock();
        if (current_character == nullptr)
            return;

        if (current_character->getObjectID() != m_parent_object.lock()->getID())
            return;

        switch (m_camera_mode)
        {
            case CameraMode::first_person:
                tickFirstPersonCamera(delta_time);
                break;
            case CameraMode::third_person:
                tickThirdPersonCamera(delta_time);
                break;
            case CameraMode::free:
                tickFreeCamera(delta_time);
                break;
            default:
                break;
        }
    }

    void CameraComponent::tickFirstPersonCamera(float delta_time)
    {
        std::shared_ptr<Level> current_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
        std::shared_ptr<Character> current_character = current_level->getCurrentActiveCharacter().lock();
        if (current_character == nullptr)
            return;

        Quaternion q_yaw, q_pitch;

        q_yaw.fromAngleAxis(g_runtime_global_context.m_input_system->m_cursor_delta_yaw, Vector3::UNIT_Z);
        q_pitch.fromAngleAxis(g_runtime_global_context.m_input_system->m_cursor_delta_pitch, m_left);

        const float offset  = static_cast<FirstPersonCameraParameter*>(m_camera_res.m_parameter)->m_vertical_offset;
        m_position = current_character->getPosition() + offset * Vector3::UNIT_Z;

        m_forward = q_yaw * q_pitch * m_forward;
        m_left    = q_yaw * q_pitch * m_left;
        m_up      = m_forward.crossProduct(m_left);

        Matrix4x4 desired_mat = Math::makeLookAtMatrix(m_position, m_position + m_forward, m_up);

        RenderSwapContext& swap_context = g_runtime_global_context.m_render_system->getSwapContext();
        CameraSwapData     camera_swap_data;
        camera_swap_data.m_camera_type                     = RenderCameraType::Motor;
        camera_swap_data.m_view_matrix                     = desired_mat;
        swap_context.getLogicSwapData().m_camera_swap_data = camera_swap_data;

        Vector3    object_facing = m_forward - m_forward.dotProduct(Vector3::UNIT_Z) * Vector3::UNIT_Z;
        Vector3    object_left   = Vector3::UNIT_Z.crossProduct(object_facing);
        Quaternion object_rotation;
        object_rotation.fromAxes(object_left, -object_facing, Vector3::UNIT_Z);
        current_character->setRotation(object_rotation);
    }

    void CameraComponent::tickThirdPersonCamera(float delta_time)
    {
        std::shared_ptr<Level> current_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
        std::shared_ptr<Character> current_character = current_level->getCurrentActiveCharacter().lock();
        if (current_character == nullptr)
            return;

        ThirdPersonCameraParameter* param = static_cast<ThirdPersonCameraParameter*>(m_camera_res.m_parameter);

        Quaternion q_yaw, q_pitch;

        q_yaw.fromAngleAxis(g_runtime_global_context.m_input_system->m_cursor_delta_yaw, Vector3::UNIT_Z);
        q_pitch.fromAngleAxis(g_runtime_global_context.m_input_system->m_cursor_delta_pitch, Vector3::UNIT_X);

        param->m_cursor_pitch = q_pitch * param->m_cursor_pitch;

        const float vertical_offset   = param->m_vertical_offset;
        const float horizontal_offset = param->m_horizontal_offset;
        Vector3     offset            = Vector3(0, horizontal_offset, vertical_offset);

        Vector3 center_pos = current_character->getPosition() + Vector3::UNIT_Z * vertical_offset;
        m_position =
            current_character->getRotation() * param->m_cursor_pitch * offset + current_character->getPosition();

        m_forward = center_pos - m_position;
        m_up = current_character->getRotation() * param->m_cursor_pitch * Vector3::UNIT_Z;
        m_left = m_up.crossProduct(m_forward);

        current_character->setRotation(q_yaw * current_character->getRotation());

        Matrix4x4 desired_mat = Math::makeLookAtMatrix(m_position, m_position + m_forward, m_up);

        RenderSwapContext& swap_context = g_runtime_global_context.m_render_system->getSwapContext();
        CameraSwapData     camera_swap_data;
        camera_swap_data.m_camera_type                     = RenderCameraType::Motor;
        camera_swap_data.m_view_matrix                     = desired_mat;
        swap_context.getLogicSwapData().m_camera_swap_data = camera_swap_data;
    }

    void CameraComponent::tickFreeCamera(float delta_time)
    {
        unsigned int command = g_runtime_global_context.m_input_system->getGameCommand();
        if (command >= (unsigned int)GameCommand::invalid) return;

        std::shared_ptr<Level> current_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
        std::shared_ptr<Character> current_character = current_level->getCurrentActiveCharacter().lock();
        if (current_character == nullptr)
            return;

        Quaternion q_yaw, q_pitch;

        q_yaw.fromAngleAxis(g_runtime_global_context.m_input_system->m_cursor_delta_yaw, Vector3::UNIT_Z);
        q_pitch.fromAngleAxis(g_runtime_global_context.m_input_system->m_cursor_delta_pitch, m_left);

        m_forward = q_yaw * q_pitch * m_forward;
        m_left = q_yaw * q_pitch * m_left;
        m_up = m_forward.crossProduct(m_left);

        bool has_move_command = ((unsigned int)GameCommand::forward | (unsigned int)GameCommand::backward |
                                 (unsigned int)GameCommand::left | (unsigned int)GameCommand::right) & command;
        if (has_move_command)
        {
            Vector3 move_direction = Vector3::ZERO;

            if ((unsigned int)GameCommand::forward & command)
            {
                move_direction += m_forward;
            }

            if ((unsigned int)GameCommand::backward & command)
            {
                move_direction -= m_forward;
            }

            if ((unsigned int)GameCommand::left & command)
            {
                move_direction += m_left;
            }

            if ((unsigned int)GameCommand::right & command)
            {
                move_direction -= m_left;
            }

            m_position += move_direction * 2.0f * delta_time;
        }

        Matrix4x4 desired_mat = Math::makeLookAtMatrix(m_position, m_position + m_forward, m_up);

        RenderSwapContext& swap_context = g_runtime_global_context.m_render_system->getSwapContext();
        CameraSwapData     camera_swap_data;
        camera_swap_data.m_camera_type = RenderCameraType::Motor;
        camera_swap_data.m_view_matrix = desired_mat;
        swap_context.getLogicSwapData().m_camera_swap_data = camera_swap_data;
    }
} // namespace Piccolo
