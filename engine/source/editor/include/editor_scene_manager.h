#pragma once
#include "render/render_camera.h"
#include "editor/include/axis.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include "runtime/function/framework/object/object.h"
#include "runtime/function/scene/scene_object.h"

namespace Pilot
{
    class PilotEditor;

    enum class EditorAxisMode : int
    {
        TranslateMode = 0,
        RotateMode = 1,
        ScaleMode = 2,
        Default = 3
    };

    class EditorSceneManager
    {
    public:
        void initialize();
        void tick(float delta_time);

    public:
        size_t   updateCursorOnAxis(
            Vector2 cursor_uv,
            Vector2 game_engine_window_size);
        void drawSelectedEntityAxis();
        std::weak_ptr<GObject> getSelectedGObject() const;
        RenderMesh* getAxisMeshByType(EditorAxisMode axis_mode);
        void onGObjectSelected(size_t selected_gobject_id);
        void onDeleteSelectedGObject();
        void moveEntity(float     new_mouse_pos_x,
            float     new_mouse_pos_y,
            float     last_mouse_pos_x,
            float     last_mouse_pos_y,
            Vector2   engine_window_pos,
            Vector2   engine_window_size,
            size_t    cursor_on_axis,
            Matrix4x4 model_matrix);

    public:
        std::shared_ptr<PCamera> getEditorCamera() { return m_camera; };

        size_t getSelectedObjectID() { return m_selected_gobject_id; };
        Matrix4x4 getSelectedObjectMatrix() { return m_selected_object_matrix; }
        EditorAxisMode getEditorAxisMode() { return m_axis_mode; }

        void setSelectedObjectID(size_t selected_gobject_id) { m_selected_gobject_id = selected_gobject_id; };
        void setSelectedObjectMatrix(Matrix4x4 new_object_matrix) { m_selected_object_matrix = new_object_matrix; }
        void setEditorAxisMode(EditorAxisMode new_axis_mode) { m_axis_mode = new_axis_mode; }
        void setSceneEditor(PilotEditor* editor) { m_editor = editor; }
    private:

        PilotEditor* m_editor{ nullptr };

        EditorTranslationAxis m_translation_axis;
        EditorRotationAxis    m_rotation_axis;
        EditorScaleAxis       m_scale_aixs;

        size_t    m_selected_gobject_id{ 0 };
        Matrix4x4 m_selected_object_matrix{ Matrix4x4::IDENTITY };

        EditorAxisMode m_axis_mode{ EditorAxisMode::TranslateMode };
        std::shared_ptr<PCamera> m_camera;

        size_t m_selected_axis;

        bool   m_is_show_axis = true;
    };
}