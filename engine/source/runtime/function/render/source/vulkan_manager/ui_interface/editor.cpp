#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include "runtime/function/render/include/render/glm_wrapper.h"

#include <cstring>

namespace Pilot
{
    void PVulkanManager::updateUIRenderSceneViewport(VkViewport render_scene_viewport)
    {
        m_viewport = render_scene_viewport;

        m_scissor.offset = {0, 0};
        m_scissor.extent = m_vulkan_context._swapchain_extent;
    }

    uint32_t PVulkanManager::getGuidOfPickedMesh(Vector2 picked_uv)
    {
        return m_mouse_pick_pass.pick(glm::vec2(picked_uv.x, picked_uv.y));
    }


    
} // namespace Pilot
