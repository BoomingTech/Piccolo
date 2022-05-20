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
    public:
        EditorUI();
        void onTick(UIState* uistate) override;

    private:
        void        onFileContentItemClicked(EditorFileNode* node);
        void        buildEditorFileAssetsUITree(EditorFileNode* node);
        void        drawAxisToggleButton(const char* string_id, bool check_state, int axis_mode);
        void        createComponentUI(Reflection::ReflectionInstance& instance);
        void        createLeafNodeUI(Reflection::ReflectionInstance& instance);
        std::string getLeafUINodeParentLabel();

        void showEditorUI();
        void showEditorMenu(bool* p_open);
        void showEditorWorldObjectsWindow(bool* p_open);
        void showEditorFileContentWindow(bool* p_open);
        void showEditorGameWindow(bool* p_open);
        void showEditorDetailWindow(bool* p_open);

    private:
        std::unordered_map<std::string, std::function<void(std::string, void*)>> m_editor_ui_creator;
        std::unordered_map<std::string, unsigned int>                            m_new_object_index_map;
        EditorFileService                                  m_editor_file_service;
        std::chrono::time_point<std::chrono::steady_clock> m_last_file_tree_update;
    };
} // namespace Pilot
