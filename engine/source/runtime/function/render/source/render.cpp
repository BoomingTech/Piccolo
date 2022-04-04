#include "runtime/function/render/include/render/render.h"

namespace Pilot
{
    void PilotRenderer::initialize()
    {
        auto framebuffer = f_get_framebuffer(this);
        m_surface->initialize(framebuffer);
    }

    void PilotRenderer::clear() { m_surface->clear(); }

    bool PilotRenderer::tick()
    {
        auto                framebuffer = f_get_framebuffer(this);
        SceneReleaseHandles release_handles;
        bool                tick_result = m_surface->tick(framebuffer, framebuffer->m_uistate.get(), release_handles);

        for (auto handle : release_handles.mesh_handles)
            f_add_release_mesh(handle);

        for (auto handle : release_handles.material_handles)
            f_add_release_material(handle);

        for (auto handle : release_handles.skeleton_binding_handles)
            f_add_release_skeleton_binding(handle);

        return tick_result;
    }

    void PilotRenderer::updateWindow(float pos_x, float pos_y, float width, float height) const
    {
        m_surface->updateWindow(pos_x, pos_y, width, height);
    }

    size_t PilotRenderer::updateCursorOnAxis(int axis_mode, const Vector2& cursor_uv, const Vector2& window_size) const
    {
        return m_surface->updateCursorOnAxis(axis_mode, cursor_uv, window_size);
    }

    size_t PilotRenderer::getGuidOfPickedMesh(const Vector2& picked_uv) const
    {
        return m_surface->getGuidOfPickedMesh(picked_uv);
    }

} // namespace Pilot
