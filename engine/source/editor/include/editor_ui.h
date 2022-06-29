#pragma once

#include "editor/include/axis.h"

#include "runtime/core/math/vector2.h"

#include "runtime/function/framework/object/object.h"
#include "runtime/function/ui/window_ui.h"

#include "editor/include/editor_file_service.h"

#include <chrono>
#include <map>
#include <vector>

namespace Piccolo
{
    class PiccoloEditor;
    class WindowSystem;
    class RenderSystem;

    class EditorUI : public WindowUI
    {
    public:
        EditorUI();

    private:
        void        onFileContentItemClicked(EditorFileNode* node);
        void        buildEditorFileAssetsUITree(EditorFileNode* node);
        void        drawAxisToggleButton(const char* string_id, bool check_state, int axis_mode);
        void        createClassUI(Reflection::ReflectionInstance& instance);
        void        createLeafNodeUI(Reflection::ReflectionInstance& instance);
        std::string getLeafUINodeParentLabel();

        void showEditorUI();
        void showEditorMenu(bool* p_open);
        void showEditorWorldObjectsWindow(bool* p_open);
        void showEditorFileContentWindow(bool* p_open);
        void showEditorGameWindow(bool* p_open);
        void showEditorDetailWindow(bool* p_open);

        void setUIColorStyle();

    public:
        virtual void initialize(WindowUIInitInfo init_info) override final;
        virtual void preRender() override final;

    private:
        std::unordered_map<std::string, std::function<void(std::string, void*)>> m_editor_ui_creator;
        std::unordered_map<std::string, unsigned int>                            m_new_object_index_map;
        EditorFileService                                                        m_editor_file_service;
        std::chrono::time_point<std::chrono::steady_clock>                       m_last_file_tree_update;

        bool m_editor_menu_window_open       = true;
        bool m_asset_window_open             = true;
        bool m_game_engine_window_open       = true;
        bool m_file_content_window_open      = true;
        bool m_detail_window_open            = true;
        bool m_scene_lights_window_open      = true;
        bool m_scene_lights_data_window_open = true;
    };
} // namespace Piccolo
