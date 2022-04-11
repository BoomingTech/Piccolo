#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/core/math/math.h"

namespace Pilot
{
    enum class EditorCommand : unsigned int
    {
        camera_left      = 1 << 0,  // A
        camera_back      = 1 << 1,  // S
        camera_foward    = 1 << 2,  // W
        camera_right     = 1 << 3,  // D
        camera_up        = 1 << 4,  // Q
        camera_down      = 1 << 5,  // E
        translation_mode = 1 << 6,  // T
        rotation_mode    = 1 << 7,  // R
        scale_mode       = 1 << 8,  // C
        exit             = 1 << 9,  // Esc
        delete_object    = 1 << 10, // Delete
    };

    enum class GameCommand : unsigned int
    {
        forward  = 1 << 0,                 // W
        backward = 1 << 1,                 // S
        left     = 1 << 2,                 // A
        right    = 1 << 3,                 // D
        jump     = 1 << 4,                 // not implemented yet
        squat    = 1 << 5,                 // not implemented yet
        sprint   = 1 << 6,                 // LEFT SHIFT
        fire     = 1 << 7,                 // not implemented yet
        invalid  = (unsigned int)(1 << 31) // lost focus
    };

    class InputSystem : public PublicSingleton<InputSystem>
    {
        friend class PublicSingleton<InputSystem>;

        static unsigned int k_complement_control_command;

    public:
        void onKey(int key, int scancode, int action, int mods);
        void onCursorPos(double current_cursor_x, double current_cursor_y);

        void tick();
        void clear();

        int m_cursor_delta_x {0};
        int m_cursor_delta_y {0};

        Radian m_cursor_delta_yaw {0};
        Radian m_cursor_delta_pitch {0};

        unsigned int getGameCommand() const { return m_game_command; }
        unsigned int getEditorCommand() const { return m_editor_command; }

    protected:
        InputSystem() = default;

    private:
        void onKeyInEditorMode(int key, int scancode, int action, int mods);
        void onKeyInGameMode(int key, int scancode, int action, int mods);

        void calculateCursorDeltaAngles();

        unsigned int m_editor_command {0};
        unsigned int m_game_command {0};

        int m_last_cursor_x {0};
        int m_last_cursor_y {0};
    };
} // namespace Pilot
