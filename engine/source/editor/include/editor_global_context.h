#pragma once

namespace Pilot
{
    class EditorGlobalContext
    {
    public:
        class EditorSceneManager* m_scene_manager;
    public:
        void initialize();
        void clear();
    };

    extern EditorGlobalContext g_editor_global_context;
}