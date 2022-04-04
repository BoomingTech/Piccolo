#include "runtime/function/render/include/render/surface.h"

namespace Pilot
{
    bool Surface::tick(const FrameBuffer* framebuffer, UIState* uistate, SceneReleaseHandles& release_handles)
    {
        if (m_io->tick_NotQuit())
        {
            if (framebuffer && framebuffer->m_scene && framebuffer->m_scene->m_loaded)
            {
                m_rhi->tick_pre(framebuffer, release_handles);
                m_ui->tick_pre(uistate);

                m_ui->tick_post(uistate);
                m_rhi->tick_post(framebuffer);
            }
            if (m_io->m_is_editor_mode == false && m_io->m_is_focus_mode == true)
            {
                glfwSetInputMode(m_io->m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else
            {
                glfwSetInputMode(m_io->m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    void Surface::updateWindow(float pos_x, float pos_y, float width, float height) const
    {
        VkViewport viewport;
        viewport.x        = pos_x;
        viewport.y        = pos_y;
        viewport.width    = width;
        viewport.height   = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        m_ui->m_tmp_uistate->m_editor_camera->setAspect(width / height);
        m_rhi->m_vulkan_manager->updateUIRenderSceneViewport(viewport);
    }

    size_t Surface::updateCursorOnAxis(int axis_mode, const Vector2& cursor_uv, const Vector2& window_size) const
    {
        const std::shared_ptr<PCamera>& editor_camera = m_ui->m_tmp_uistate->m_editor_camera;

        return m_rhi->m_vulkan_manager->updateCursorOnAxis(axis_mode,
                                                           cursor_uv,
                                                           window_size,
                                                           editor_camera->getFovYDeprecated(),
                                                           editor_camera->forward(),
                                                           editor_camera->up(),
                                                           editor_camera->right(),
                                                           editor_camera->position());
    }

    size_t Surface::getGuidOfPickedMesh(const Vector2& picked_uv) const
    {
        return m_rhi->m_vulkan_manager->getGuidOfPickedMesh(picked_uv);
    }

} // namespace Pilot
