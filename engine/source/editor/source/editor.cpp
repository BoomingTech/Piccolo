#include "editor//include/editor.h"
#include "editor/include/editor_ui.h"
#include "runtime/engine.h"
#include "runtime/function/render/include/render/render.h"

#include <cassert>

namespace Pilot
{
    PilotEditor::PilotEditor() {}
    PilotEditor::~PilotEditor() {}

    void PilotEditor::initialize(PilotEngine* engine_runtime)
    {
        assert(engine_runtime);

        m_engine_runtime = engine_runtime;
        m_editor_ui      = std::make_shared<EditorUI>(this);

        std::shared_ptr<PilotRenderer> render = m_engine_runtime->getRender();
        assert(render);

        render->setSurfaceUI(m_editor_ui);
    }

    void PilotEditor::clear() {}

    void PilotEditor::run()
    {
        assert(m_engine_runtime);
        assert(m_editor_ui);

        m_engine_runtime->run();
    }

    void PilotEditor::onWindowChanged(float pos_x, float pos_y, float width, float height) const
    {
        std::shared_ptr<PilotRenderer> render = m_engine_runtime->getRender();
        assert(render);

        render->updateWindow(pos_x, pos_y, width, height);
    }

    size_t PilotEditor::onUpdateCursorOnAxis(int axis_mode, const Vector2& cursor_uv, const Vector2& window_size) const
    {
        std::shared_ptr<PilotRenderer> render = m_engine_runtime->getRender();
        assert(render);

        return render->updateCursorOnAxis(axis_mode, cursor_uv, window_size);
    }

    size_t PilotEditor::getGuidOfPickedMesh(const Vector2& picked_uv) const
    {
        std::shared_ptr<PilotRenderer> render = m_engine_runtime->getRender();
        assert(render);

        return render->getGuidOfPickedMesh(picked_uv);
    }

} // namespace Pilot
