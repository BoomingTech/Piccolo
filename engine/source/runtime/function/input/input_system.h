#pragma once

#include "runtime/core/math/math.h"

namespace Piccolo
{
    enum class GameCommand : unsigned int
    {
        forward  = 1 << 0,                 // W
        backward = 1 << 1,                 // S
        left     = 1 << 2,                 // A
        right    = 1 << 3,                 // D
        jump     = 1 << 4,                 // SPACE
        squat    = 1 << 5,                 // not implemented yet
        sprint   = 1 << 6,                 // LEFT SHIFT
        fire     = 1 << 7,                 // not implemented yet
        free_carema = 1 << 8,              // F
        invalid  = (unsigned int)(1 << 31) // lost focus
    };

    extern unsigned int k_complement_control_command;

    class InputSystem
    {

    public:
        void onKey(int key, int scancode, int action, int mods);
        void onCursorPos(double current_cursor_x, double current_cursor_y);

        void initialize();
        void tick();
        void clear();

        int m_cursor_delta_x {0};
        int m_cursor_delta_y {0};

        Radian m_cursor_delta_yaw {0};
        Radian m_cursor_delta_pitch {0};

        void         resetGameCommand() { m_game_command = 0; }
        unsigned int getGameCommand() const { return m_game_command; }

    private:
        void onKeyInGameMode(int key, int scancode, int action, int mods);

        void calculateCursorDeltaAngles();

        unsigned int m_game_command {0};

        int m_last_cursor_x {0};
        int m_last_cursor_y {0};
    };
} // namespace Piccolo
