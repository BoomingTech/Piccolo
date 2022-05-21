#include "editor//include/editor.h"
#include "editor/include/editor_ui.h"
#include "editor/include/editor_scene_manager.h"
#include "editor/include/editor_input_manager.h"
#include "editor/include/editor_global_context.h"
#include "runtime/engine.h"
#include "runtime/function/render/include/render/render.h"

#include <cassert>

namespace Pilot
{
    void registerEdtorTickComponent(std::string component_type_name)
    {
        g_editor_tick_component_types.insert(component_type_name);
    }

    PilotEditor::PilotEditor()
    {
        registerEdtorTickComponent("TransformComponent");
        registerEdtorTickComponent("MeshComponent");
    }

    PilotEditor::~PilotEditor() {}


    void PilotEditor::initialize(PilotEngine* engine_runtime)
    {
        assert(engine_runtime);

        g_is_editor_mode = true;
        m_engine_runtime = engine_runtime;
        g_editor_global_context.initialize();
        m_editor_ui = std::make_shared<EditorUI>();

        std::shared_ptr<PilotRenderer> render = m_engine_runtime->getRender();
        assert(render);

        render->setSurfaceUI(m_editor_ui);
    }

    void PilotEditor::clear()
    {
        g_editor_global_context.clear();
    }

    void PilotEditor::run()
    {
        assert(m_engine_runtime);
        assert(m_editor_ui);
        float delta_time;
        while (true)
        {
            delta_time = m_engine_runtime->getDeltaTime();
            g_editor_global_context.m_scene_manager->tick(delta_time);
            g_editor_global_context.m_input_manager->tick(delta_time);
            if (!m_engine_runtime->tickOneFrame(delta_time))
                return;
        }
    }
} // namespace Pilot
