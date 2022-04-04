#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

// semaphore : signal an image is ready for rendering // ready for presentation
// (m_vulkan_context._swapchain_images --> semaphores, fences)
bool Pilot::PVulkanManager::createSyncPrimitives()
{
    VkSemaphoreCreateInfo semaphore_create_info {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // the fence is initialized as signaled

    for (uint32_t i = 0; i < m_max_frames_in_flight; i++)
    {
        if (vkCreateSemaphore(m_vulkan_context._device,
                              &semaphore_create_info,
                              nullptr,
                              &m_image_available_for_render_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_vulkan_context._device,
                              &semaphore_create_info,
                              nullptr,
                              &m_image_finished_for_presentation_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_vulkan_context._device, &fence_create_info, nullptr, &m_is_frame_in_flight_fences[i]) !=
                VK_SUCCESS)
        {
            throw std::runtime_error("vk create semaphore & fence");
        }
    }
    return true;
}
