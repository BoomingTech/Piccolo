#include "runtime/function/input/input_system.h"

#include "runtime/engine.h"
#include "runtime/function/render/include/render/surface_io.h"
#include "runtime/function/scene/scene_manager.h"

#include <GLFW/glfw3.h>

namespace Pilot
{
    unsigned int InputSystem::k_complement_control_command = 0xFFFFFFFF;

    void InputSystem::onKey(int key, int scancode, int action, int mods)
    {
        if (g_is_editor_mode)
        {
            m_game_command = 0;
            onKeyInEditorMode(key, scancode, action, mods);
        }
        else
        {
            m_editor_command = 0;
            onKeyInGameMode(key, scancode, action, mods);
        }
    }

    void InputSystem::onKeyInEditorMode(int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
                case GLFW_KEY_A:
                    m_editor_command |= (unsigned int)EditorCommand::camera_left;
                    break;
                case GLFW_KEY_S:
                    m_editor_command |= (unsigned int)EditorCommand::camera_back;
                    break;
                case GLFW_KEY_W:
                    m_editor_command |= (unsigned int)EditorCommand::camera_foward;
                    break;
                case GLFW_KEY_D:
                    m_editor_command |= (unsigned int)EditorCommand::camera_right;
                    break;
                case GLFW_KEY_Q:
                    m_editor_command |= (unsigned int)EditorCommand::camera_up;
                    break;
                case GLFW_KEY_E:
                    m_editor_command |= (unsigned int)EditorCommand::camera_down;
                    break;
                case GLFW_KEY_T:
                    m_editor_command |= (unsigned int)EditorCommand::translation_mode;
                    break;
                case GLFW_KEY_R:
                    m_editor_command |= (unsigned int)EditorCommand::rotation_mode;
                    break;
                case GLFW_KEY_C:
                    m_editor_command |= (unsigned int)EditorCommand::scale_mode;
                    break;
                case GLFW_KEY_DELETE:
                    m_editor_command |= (unsigned int)EditorCommand::delete_object;
                    break;
                default:
                    break;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            switch (key)
            {
                case GLFW_KEY_ESCAPE:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::exit);
                    break;
                case GLFW_KEY_A:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_left);
                    break;
                case GLFW_KEY_S:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_back);
                    break;
                case GLFW_KEY_W:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_foward);
                    break;
                case GLFW_KEY_D:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_right);
                    break;
                case GLFW_KEY_Q:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_up);
                    break;
                case GLFW_KEY_E:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_down);
                    break;
                case GLFW_KEY_T:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::translation_mode);
                    break;
                case GLFW_KEY_R:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::rotation_mode);
                    break;
                case GLFW_KEY_C:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::scale_mode);
                    break;
                case GLFW_KEY_DELETE:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::delete_object);
                    break;
                default:
                    break;
            }
        }
    }

    void InputSystem::onKeyInGameMode(int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
                case GLFW_KEY_ESCAPE:
                    // close();
                    break;
                case GLFW_KEY_R:
                    break;
                case GLFW_KEY_A:
                    m_game_command |= (unsigned int)GameCommand::left;
                    break;
                case GLFW_KEY_S:
                    m_game_command |= (unsigned int)GameCommand::backward;
                    break;
                case GLFW_KEY_W:
                    m_game_command |= (unsigned int)GameCommand::forward;
                    break;
                case GLFW_KEY_D:
                    m_game_command |= (unsigned int)GameCommand::right;
                    break;
                case GLFW_KEY_LEFT_CONTROL:
                    m_game_command |= (unsigned int)GameCommand::squat;
                    break;
                case GLFW_KEY_LEFT_ALT: {
                    std::shared_ptr<SurfaceIO> surface_io = PilotEngine::getInstance().getSurfaceIO();
                    surface_io->setFocusMode(!surface_io->m_is_focus_mode);
                }
                break;
                case GLFW_KEY_LEFT_SHIFT:
                    m_game_command |= (unsigned int)GameCommand::sprint;
                    break;
                default:
                    break;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            switch (key)
            {
                case GLFW_KEY_ESCAPE:
                    // close();
                    break;
                case GLFW_KEY_R:
                    break;
                case GLFW_KEY_W:
                    m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::forward);
                    break;
                case GLFW_KEY_S:
                    m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::backward);
                    break;
                case GLFW_KEY_A:
                    m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::left);
                    break;
                case GLFW_KEY_D:
                    m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::right);
                    break;
                case GLFW_KEY_LEFT_CONTROL:
                    m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::squat);
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::sprint);
                    break;
                default:
                    break;
            }
        }
    }

    void InputSystem::onCursorPos(double current_cursor_x, double current_cursor_y)
    {
        if (PilotEngine::getInstance().getSurfaceIO()->m_is_focus_mode)
        {
            m_cursor_delta_x = m_last_cursor_x - current_cursor_x;
            m_cursor_delta_y = m_last_cursor_y - current_cursor_y;
        }

        m_last_cursor_x = current_cursor_x;
        m_last_cursor_y = current_cursor_y;
    }

    void InputSystem::clear()
    {
        m_cursor_delta_x = 0;
        m_cursor_delta_y = 0;
    }

    void InputSystem::calculateCursorDeltaAngles()
    {
        SceneManager&  scene_manager = SceneManager::getInstance();
        const Vector2& window_size   = scene_manager.getWindowSize();

        if (window_size.x < std::numeric_limits<float>::epsilon() ||
            window_size.y < std::numeric_limits<float>::epsilon())
        {
            return;
        }

        const auto& fov = scene_manager.getFOV();

        Radian cursor_delta_x(Math::degreesToRadians(m_cursor_delta_x));
        Radian cursor_delta_y(Math::degreesToRadians(m_cursor_delta_y));

        m_cursor_delta_yaw   = (cursor_delta_x / window_size.x) * fov.x;
        m_cursor_delta_pitch = -(cursor_delta_y / window_size.y) * fov.y;
    }

    void InputSystem::tick()
    {
        calculateCursorDeltaAngles();
        clear();

        std::shared_ptr<SurfaceIO> surface_io = PilotEngine::getInstance().getSurfaceIO();
        if (surface_io->m_is_focus_mode)
        {
            m_game_command &= (k_complement_control_command ^ (unsigned int)GameCommand::invalid);
        }
        else
        {
            m_game_command |= (unsigned int)GameCommand::invalid;
        }

        static bool inited = false;
        if (!inited)
        {
            Pilot::PilotEngine::getInstance().getSurfaceIO()->registerOnKeyFunc(std::bind(&InputSystem::onKey,
                                                                                          this,
                                                                                          std::placeholders::_1,
                                                                                          std::placeholders::_2,
                                                                                          std::placeholders::_3,
                                                                                          std::placeholders::_4));
            Pilot::PilotEngine::getInstance().getSurfaceIO()->registerOnCursorPosFunc(
                std::bind(&InputSystem::onCursorPos, this, std::placeholders::_1, std::placeholders::_2));
            inited = true;
        }

        // components->getWritable<MotorInputDataT>(0)->move_type        = (Pilot::MotorMotivation)m_control_command;
        // components->getWritable<MotorInputDataT>(0)->turn_angle_yaw   = m_turn_angle_yaw;
        // components->getWritable<MotorInputDataT>(0)->turn_angle_pitch = m_trun_angle_pitch;
    }
} // namespace Pilot
