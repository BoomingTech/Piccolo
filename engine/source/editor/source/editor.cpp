#include "editor//include/editor.h"

#include "runtime/engine.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_system.h"

#include "editor/include/editor_global_context.h"
#include "editor/include/editor_input_manager.h"
#include "editor/include/editor_scene_manager.h"
#include "editor/include/editor_ui.h"

namespace Piccolo
{
    void registerEdtorTickComponent(std::string component_type_name)
    {
        g_editor_tick_component_types.insert(component_type_name);
    }

    PiccoloEditor::PiccoloEditor()
    {
        registerEdtorTickComponent("TransformComponent");
        registerEdtorTickComponent("MeshComponent");
    }

    PiccoloEditor::~PiccoloEditor() {}

    void PiccoloEditor::initialize(PiccoloEngine* engine_runtime)
    {
        assert(engine_runtime);

        g_is_editor_mode = true;
        m_engine_runtime = engine_runtime;

        EditorGlobalContextInitInfo init_info = {g_runtime_global_context.m_window_system.get(),
                                                 g_runtime_global_context.m_render_system.get(),
                                                 engine_runtime};
        g_editor_global_context.initialize(init_info);
        g_editor_global_context.m_scene_manager->setEditorCamera(
            g_runtime_global_context.m_render_system->getRenderCamera());
        g_editor_global_context.m_scene_manager->uploadAxisResource();

        m_editor_ui                   = std::make_shared<EditorUI>();
        WindowUIInitInfo ui_init_info = {g_runtime_global_context.m_window_system,
                                         g_runtime_global_context.m_render_system};
        m_editor_ui->initialize(ui_init_info);
    }

    void PiccoloEditor::clear() { g_editor_global_context.clear(); }

    void PiccoloEditor::run()
    {
        assert(m_engine_runtime);
        assert(m_editor_ui);
        float delta_time;
        while (true)
        {
            delta_time = m_engine_runtime->calculateDeltaTime();
            g_editor_global_context.m_scene_manager->tick(delta_time);
            g_editor_global_context.m_input_manager->tick(delta_time);
            if (!m_engine_runtime->tickOneFrame(delta_time))
                return;
        }
    }
} // namespace Piccolo
