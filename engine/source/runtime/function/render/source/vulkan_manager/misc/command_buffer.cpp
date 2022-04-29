#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

// command pool for submitting drawing commands
bool Pilot::PVulkanManager::initializeCommandPool()
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext            = NULL;
    command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    command_pool_create_info.queueFamilyIndex = m_vulkan_context._queue_indices.graphicsFamily.value();

    for (uint32_t i = 0; i < m_max_frames_in_flight; ++i)
    {
        if (vkCreateCommandPool(m_vulkan_context._device, &command_pool_create_info, NULL, &m_command_pools[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("vk create command pool");
        }
    }

    return true;
}

// allocate command buffer for drawing commands
bool Pilot::PVulkanManager::initializeCommandBuffers()
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info {};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1U;

    for (uint32_t i = 0; i < m_max_frames_in_flight; ++i)
    {
        command_buffer_allocate_info.commandPool = m_command_pools[i];

        if (vkAllocateCommandBuffers(m_vulkan_context._device, &command_buffer_allocate_info, &m_command_buffers[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("vk allocate command buffers");
        }
    }

    return true;
}
