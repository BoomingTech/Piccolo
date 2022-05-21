#pragma once

namespace Pilot
{
    class EditorGlobalContext
    {
    public:
        class EditorSceneManager* m_scene_manager{ nullptr };
        class EditorInputManager* m_input_manager{ nullptr };
    public:
        void initialize();
        void clear();
    };

    extern EditorGlobalContext g_editor_global_context;
}