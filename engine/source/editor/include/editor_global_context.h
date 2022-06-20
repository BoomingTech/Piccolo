#pragma once

namespace Piccolo
{
    struct EditorGlobalContextInitInfo
    {
        class WindowSystem* window_system;
        class RenderSystem* render_system;
        class PiccoloEngine*  engine_runtime;
    };

    class EditorGlobalContext
    {
    public:
        class EditorSceneManager* m_scene_manager {nullptr};
        class EditorInputManager* m_input_manager {nullptr};
        class RenderSystem*       m_render_system {nullptr};
        class WindowSystem*       m_window_system {nullptr};
        class PiccoloEngine*        m_engine_runtime {nullptr};

    public:
        void initialize(const EditorGlobalContextInitInfo& init_info);
        void clear();
    };

    extern EditorGlobalContext g_editor_global_context;
} // namespace Piccolo