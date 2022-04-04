#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include <vector>

bool Pilot::PVulkanManager::initializeSwapchainFramebuffers()
{
    m_swapchain_framebuffers.resize(m_vulkan_context._swapchain_imageviews.size());

    // create frame buffer for every imageview
    for (size_t i = 0; i < m_vulkan_context._swapchain_imageviews.size(); i++)
    {
        // first image view is used in post process
        VkImageView framebuffer_attachments_for_image_view[3] = {m_postprocess_pass._framebuffer.attachments[0].view,
                                                                 m_vulkan_context._depth_image_view,
                                                                 m_vulkan_context._swapchain_imageviews[i]};

        VkFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.flags      = 0U;
        framebuffer_create_info.renderPass = m_mesh_lighting_pass._framebuffer.render_pass;
        framebuffer_create_info.attachmentCount =
            (sizeof(framebuffer_attachments_for_image_view) / sizeof(framebuffer_attachments_for_image_view[0]));
        framebuffer_create_info.pAttachments = framebuffer_attachments_for_image_view;
        framebuffer_create_info.width        = m_vulkan_context._swapchain_extent.width;
        framebuffer_create_info.height       = m_vulkan_context._swapchain_extent.height;
        framebuffer_create_info.layers       = 1;

        if (vkCreateFramebuffer(
                m_vulkan_context._device, &framebuffer_create_info, nullptr, &m_swapchain_framebuffers[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create normal framebuffer");
        }
    }

    return true;
}

bool Pilot::PVulkanManager::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_vulkan_context._window, &width, &height);
    while (width == 0 || height == 0) // minimized 0,0 ,pause for now
    {
        glfwGetFramebufferSize(m_vulkan_context._window, &width, &height);
        glfwWaitEvents();
    }

    VkResult res_wait_for_fences = m_vulkan_context._vkWaitForFences(
        m_vulkan_context._device, m_max_frames_in_flight, m_is_frame_in_flight_fences, VK_TRUE, UINT64_MAX);
    assert(VK_SUCCESS == res_wait_for_fences);

    clearSwapChain();

    m_vulkan_context.createSwapchain();
    m_vulkan_context.createSwapchainImageViews();
    m_vulkan_context.createFramebufferImageAndView();

    m_postprocess_pass.updateAfterFramebufferRecreate();
    m_mouse_pick_pass.recreateFramebuffer();

    initializeSwapchainFramebuffers();

    return true;
}

void Pilot::PVulkanManager::clearSwapChain()
{
    for (auto framebuffer : m_swapchain_framebuffers)
    {
        vkDestroyFramebuffer(m_vulkan_context._device, framebuffer, NULL);
    }

    vkDestroyImageView(m_vulkan_context._device, m_vulkan_context._depth_image_view, NULL);
    vkDestroyImage(m_vulkan_context._device, m_vulkan_context._depth_image, NULL);
    vkFreeMemory(m_vulkan_context._device, m_vulkan_context._depth_image_memory, NULL);

    m_vulkan_context.clearSwapchain();
}
