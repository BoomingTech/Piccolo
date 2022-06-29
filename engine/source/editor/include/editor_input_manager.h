#pragma once

#include "runtime/core/math/vector2.h"

#include "runtime/function/render/render_camera.h"

#include <vector>

namespace Piccolo
{
    class PiccoloEditor;

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

    class EditorInputManager
    {
    public:
        void initialize();
        void tick(float delta_time);

    public:
        void registerInput();
        void updateCursorOnAxis(Vector2 cursor_uv);
        void processEditorCommand();
        void onKeyInEditorMode(int key, int scancode, int action, int mods);

        void onKey(int key, int scancode, int action, int mods);
        void onReset();
        void onCursorPos(double xpos, double ypos);
        void onCursorEnter(int entered);
        void onScroll(double xoffset, double yoffset);
        void onMouseButtonClicked(int key, int action);
        void onWindowClosed();

        bool isCursorInRect(Vector2 pos, Vector2 size) const;

    public:
        Vector2 getEngineWindowPos() const { return m_engine_window_pos; };
        Vector2 getEngineWindowSize() const { return m_engine_window_size; };
        float   getCameraSpeed() const { return m_camera_speed; };

        void setEngineWindowPos(Vector2 new_window_pos) { m_engine_window_pos = new_window_pos; };
        void setEngineWindowSize(Vector2 new_window_size) { m_engine_window_size = new_window_size; };
        void resetEditorCommand() { m_editor_command = 0; }

    private:
        Vector2 m_engine_window_pos {0.0f, 0.0f};
        Vector2 m_engine_window_size {1280.0f, 768.0f};
        float   m_mouse_x {0.0f};
        float   m_mouse_y {0.0f};
        float   m_camera_speed {0.05f};

        size_t       m_cursor_on_axis {3};
        unsigned int m_editor_command {0};
    };
} // namespace Piccolo
