#pragma once

#include "editor/include/axis.h"

#include "runtime/core/math/vector2.h"

#include "runtime/function/framework/object/object.h"
#include "runtime/function/render/include/render/surface.h"
#include "runtime/function/scene/scene_object.h"

#include "editor/include/editor_file_service.h"

#include <chrono>
#include <map>
#include <vector>

namespace Pilot
{
    class PilotEditor;

    class EditorUI : public SurfaceUI
    {

    private:
        void        onFileContentItemClicked(EditorFileNode* node);
        void        updateCursorOnAxis(Vector2 cursor_uv);
        void        buildEditorFileAssetsUITree(EditorFileNode* node);
        void        processEditorCommand();
        void        drawAxisToggleButton(const char* string_id, bool check_state, int axis_mode);
        void        createComponentUI(Reflection::ReflectionInstance& instance);
        void        createLeafNodeUI(Reflection::ReflectionInstance& instance);
        std::string getLeafUINodeParentLabel();
        bool        isCursorInRect(Vector2 pos, Vector2 size) const;

        void showEditorUI();
        void showEditorMenu(bool* p_open);
        void showEditorWorldObjectsWindow(bool* p_open);
        void showEditorFileContentWindow(bool* p_open);
        void showEditorGameWindow(bool* p_open);
        void showEditorDetailWindow(bool* p_open);

        void onReset();
        void onCursorPos(double xpos, double ypos);
        void onCursorEnter(int entered);
        void onScroll(double xoffset, double yoffset);
        void onMouseButtonClicked(int key, int action);
        void onWindowClosed();

    public:
        EditorUI(PilotEditor* editor);

        void onTick(UIState* uistate) override;
        void registerInput() override;

    private:
        PilotEditor* m_editor{ nullptr };

        std::unordered_map<std::string, std::function<void(std::string, void*)>> m_editor_ui_creator;
        std::unordered_map<std::string, unsigned int>                            m_new_object_index_map;

        Vector2 m_engine_window_pos{ 0.0f, 0.0f };
        Vector2 m_engine_window_size{ 1280.0f, 768.0f };
        float   m_mouse_x{ 0.0f };
        float   m_mouse_y{ 0.0f };
        float   m_camera_speed{ 0.05f };

        size_t         m_cursor_on_axis{ 3 };

        EditorFileService                                  m_editor_file_service;
        std::chrono::time_point<std::chrono::steady_clock> m_last_file_tree_update;
    };
} // namespace Pilot
