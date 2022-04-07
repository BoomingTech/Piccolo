#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"
#include "runtime/function/render/include/render/render.h"

// TODO: remove this as the vmaAllocator
uint32_t Pilot::PVulkanManager::m_max_vertex_blending_mesh_count = 256;
uint32_t Pilot::PVulkanManager::m_max_material_count             = 256;

#ifndef NDEBUG
bool Pilot::PVulkanManager::m_enable_validation_Layers  = true;
bool Pilot::PVulkanManager::m_enable_debug_untils_label = true;
#else
bool Pilot::PVulkanManager::m_enable_validation_Layers  = false;
bool Pilot::PVulkanManager::m_enable_debug_untils_label = false;
#endif

#if defined(__GNUC__) && defined(__MACH__)
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html
bool Pilot::PVulkanManager::m_enable_point_light_shadow = false;
#else
bool Pilot::PVulkanManager::m_enable_point_light_shadow = true;
#endif

// initialize vulkan from io->window
Pilot::PVulkanManager::PVulkanManager(GLFWwindow* window, class Scene& scene, PilotRenderer* pilot_renderer)
{
    initialize(window, scene, pilot_renderer);
}

void Pilot::PVulkanManager::beginFrame()
{
    // reset storage buffer offset
    m_global_render_resource._storage_buffer._global_upload_ringbuffers_end[m_current_frame_index] =
        m_global_render_resource._storage_buffer._global_upload_ringbuffers_begin[m_current_frame_index];

    VkResult res_wait_for_fences = m_vulkan_context._vkWaitForFences(
        m_vulkan_context._device, 1, &m_is_frame_in_flight_fences[m_current_frame_index], VK_TRUE, UINT64_MAX);
    assert(VK_SUCCESS == res_wait_for_fences);

    VkResult res_reset_command_pool =
        m_vulkan_context._vkResetCommandPool(m_vulkan_context._device, m_command_pools[m_current_frame_index], 0);
    assert(VK_SUCCESS == res_reset_command_pool);

    VkResult acquire_image_result =
        vkAcquireNextImageKHR(m_vulkan_context._device,
                              m_vulkan_context._swapchain,
                              UINT64_MAX,
                              m_image_available_for_render_semaphores[m_current_frame_index],
                              VK_NULL_HANDLE,
                              &m_current_swapchain_image_index);

    if (acquire_image_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        m_frame_swapchain_image_acquired[m_current_frame_index] = false;
        return;
    }
    else if (acquire_image_result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapChain();
        m_frame_swapchain_image_acquired[m_current_frame_index] = false;
        return;
    }
    else
    {
        m_frame_swapchain_image_acquired[m_current_frame_index] = true;
        assert(VK_SUBOPTIMAL_KHR == acquire_image_result || VK_SUCCESS == acquire_image_result);
    }

    VkCommandBufferBeginInfo command_buffer_begin_info {};
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags            = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    VkResult res_begin_command_buffer =
        m_vulkan_context._vkBeginCommandBuffer(m_command_buffers[m_current_frame_index], &command_buffer_begin_info);
    assert(VK_SUCCESS == res_begin_command_buffer);

    PRenderPassBase::m_command_info._current_frame_index       = m_current_frame_index;
    PRenderPassBase::m_command_info._p_current_frame_index     = &m_current_frame_index;
    PRenderPassBase::m_command_info._current_command_buffer    = m_command_buffers[m_current_frame_index];
    PRenderPassBase::m_command_info._p_current_command_buffer  = m_command_buffers;
    PRenderPassBase::m_command_info._current_command_pool      = m_command_pools[m_current_frame_index];
    PRenderPassBase::m_command_info._p_command_pools           = m_command_pools;
    PRenderPassBase::m_command_info._current_fence             = m_is_frame_in_flight_fences[m_current_frame_index];
    PRenderPassBase::m_command_info._viewport                  = m_viewport;
    PRenderPassBase::m_command_info._scissor                   = m_scissor;
    PRenderPassBase::m_command_info._max_frames_in_flight      = m_max_frames_in_flight;
    PRenderPassBase::m_command_info._is_frame_in_flight_fences = m_is_frame_in_flight_fences;

    PRenderPassBase::m_visiable_nodes.p_directional_light_visible_mesh_nodes = &m_directional_light_visible_mesh_nodes;
    PRenderPassBase::m_visiable_nodes.p_point_lights_visible_mesh_nodes      = &m_point_lights_visible_mesh_nodes;
    PRenderPassBase::m_visiable_nodes.p_main_camera_visible_mesh_nodes       = &m_main_camera_visible_mesh_nodes;
    PRenderPassBase::m_visiable_nodes.p_axis_node                            = &m_axis_node;
    PRenderPassBase::m_visiable_nodes.p_main_camera_visible_particlebillboard_nodes =
        &m_main_camera_visible_particlebillboard_nodes;

    m_directional_light_pass.draw();

    m_point_light_pass.draw();

    // Lighting begin pass
    {
        VkRenderPassBeginInfo renderpass_begin_info {};
        renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderPass        = m_mesh_lighting_pass._framebuffer.render_pass;
        renderpass_begin_info.framebuffer       = m_swapchain_framebuffers[m_current_swapchain_image_index];
        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = m_vulkan_context._swapchain_extent;

        VkClearValue clear_values[3];
        clear_values[0].color                 = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[1].depthStencil          = {1.0f, 0};
        clear_values[2].color                 = {{0.0f, 0.0f, 0.0f, 1.0f}};
        renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
        renderpass_begin_info.pClearValues    = clear_values;

        m_vulkan_context._vkCmdBeginRenderPass(
            m_command_buffers[m_current_frame_index], &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    m_mesh_lighting_pass.draw();

    m_vulkan_context._vkCmdNextSubpass(m_command_buffers[m_current_frame_index], VK_SUBPASS_CONTENTS_INLINE);

    m_postprocess_pass.draw();
}

void Pilot::PVulkanManager::endFrame()
{
    if (m_frame_swapchain_image_acquired[m_current_frame_index])
    {
        // Lighting end pass
        {
            m_vulkan_context._vkCmdEndRenderPass(m_command_buffers[m_current_frame_index]);
        }

        // end command buffer
        VkResult res_end_command_buffer =
            m_vulkan_context._vkEndCommandBuffer(m_command_buffers[m_current_frame_index]);
        assert(VK_SUCCESS == res_end_command_buffer);

        // submit commands
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo         submit_info   = {};
        submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount     = 1;
        submit_info.pWaitSemaphores        = &m_image_available_for_render_semaphores[m_current_frame_index];
        submit_info.pWaitDstStageMask      = wait_stages;
        submit_info.commandBufferCount     = 1;
        submit_info.pCommandBuffers        = &m_command_buffers[m_current_frame_index];
        submit_info.signalSemaphoreCount   = 1;
        submit_info.pSignalSemaphores      = &m_image_finished_for_presentation_semaphores[m_current_frame_index];

        VkResult res_reset_fences = m_vulkan_context._vkResetFences(
            m_vulkan_context._device, 1, &m_is_frame_in_flight_fences[m_current_frame_index]);
        assert(VK_SUCCESS == res_reset_fences);

        VkResult res_queue_submit = vkQueueSubmit(
            m_vulkan_context._graphics_queue, 1, &submit_info, m_is_frame_in_flight_fences[m_current_frame_index]);
        assert(VK_SUCCESS == res_queue_submit);

        // present
        VkPresentInfoKHR present_info   = {};
        present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = &m_image_finished_for_presentation_semaphores[m_current_frame_index];
        present_info.swapchainCount     = 1;
        present_info.pSwapchains        = &m_vulkan_context._swapchain;
        present_info.pImageIndices      = &m_current_swapchain_image_index;

        VkResult present_result = vkQueuePresentKHR(m_vulkan_context._present_queue, &present_info);
        // check if swapchain out of date
        if (present_result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
        }
        else if (present_result == VK_SUBOPTIMAL_KHR)
        {
            recreateSwapChain();
        }
        else
        {
            assert(VK_SUBOPTIMAL_KHR == present_result || VK_SUCCESS == present_result);
        }
    }

    m_current_frame_index = (m_current_frame_index + 1) % m_max_frames_in_flight;
}

void Pilot::PVulkanManager::clear()
{
    // cleanup is incomplete, only for demonstration
    for (uint32_t i = 0; i < m_max_frames_in_flight; ++i)
    {
        VkResult res_wait_for_fences =
            vkWaitForFences(m_vulkan_context._device, 1, &m_is_frame_in_flight_fences[i], VK_TRUE, UINT64_MAX);
        assert(VK_SUCCESS == res_wait_for_fences);
    }

    for (auto& pair1 : m_vulkan_pbr_materials)
    {
        VulkanPBRMaterial& material = pair1.second;

        vkDestroyImageView(m_vulkan_context._device, material.base_color_image_view, nullptr);
        vmaDestroyImage(m_vulkan_context._assets_allocator,
                        material.base_color_texture_image,
                        material.base_color_image_allocation);

        vkDestroyImageView(m_vulkan_context._device, material.metallic_roughness_image_view, nullptr);
        vmaDestroyImage(m_vulkan_context._assets_allocator,
                        material.metallic_roughness_texture_image,
                        material.metallic_roughness_image_allocation);

        vkDestroyImageView(m_vulkan_context._device, material.normal_image_view, nullptr);
        vmaDestroyImage(
            m_vulkan_context._assets_allocator, material.normal_texture_image, material.normal_image_allocation);

        vkDestroyImageView(m_vulkan_context._device, material.occlusion_image_view, nullptr);
        vmaDestroyImage(
            m_vulkan_context._assets_allocator, material.occlusion_texture_image, material.occlusion_image_allocation);

        vkDestroyImageView(m_vulkan_context._device, material.emissive_image_view, nullptr);
        vmaDestroyImage(
            m_vulkan_context._assets_allocator, material.emissive_texture_image, material.emissive_image_allocation);

        vmaDestroyBuffer(m_vulkan_context._assets_allocator,
                         material.material_uniform_buffer,
                         material.material_uniform_buffer_allocation);
    }
    vkResetDescriptorPool(m_vulkan_context._device, m_descriptor_pool, 0U);

    for (auto& pair : m_vulkan_meshes)
    {
        VulkanMesh& mesh = pair.second;

        vmaDestroyBuffer(m_vulkan_context._assets_allocator,
                         mesh.mesh_vertex_position_buffer,
                         mesh.mesh_vertex_position_buffer_allocation);
        vmaDestroyBuffer(m_vulkan_context._assets_allocator,
                         mesh.mesh_vertex_varying_enable_blending_buffer,
                         mesh.mesh_vertex_varying_enable_blending_buffer_allocation);

        if (VK_NULL_HANDLE != mesh.mesh_vertex_joint_binding_buffer)
        {
            vmaDestroyBuffer(m_vulkan_context._assets_allocator,
                             mesh.mesh_vertex_joint_binding_buffer,
                             mesh.mesh_vertex_joint_binding_buffer_allocation);
        }

        vmaDestroyBuffer(m_vulkan_context._assets_allocator,
                         mesh.mesh_vertex_varying_buffer,
                         mesh.mesh_vertex_varying_buffer_allocation);
        vmaDestroyBuffer(m_vulkan_context._assets_allocator, mesh.mesh_index_buffer, mesh.mesh_index_buffer_allocation);
    }

    if (m_global_render_resource.isIBLDataLoaded())
    {
        vmaDestroyImage(m_vulkan_context._assets_allocator,
                        m_global_render_resource._ibl_resource._brdfLUT_texture_image,
                        m_global_render_resource._ibl_resource._brdfLUT_texture_image_allocation);
        vmaDestroyImage(m_vulkan_context._assets_allocator,
                        m_global_render_resource._ibl_resource._irradiance_texture_image,
                        m_global_render_resource._ibl_resource._irradiance_texture_image_allocation);
        vmaDestroyImage(m_vulkan_context._assets_allocator,
                        m_global_render_resource._ibl_resource._specular_texture_image,
                        m_global_render_resource._ibl_resource._specular_texture_image_allocation);
    }

    vmaDestroyAllocator(m_vulkan_context._assets_allocator);

    clearSwapChain(); // needs commandpool to freecommandframebuffers()

    vkUnmapMemory(m_vulkan_context._device, m_global_render_resource._storage_buffer._global_upload_ringbuffer_memory);
    vkDestroyBuffer(
        m_vulkan_context._device, m_global_render_resource._storage_buffer._global_upload_ringbuffer, nullptr);
    vkFreeMemory(
        m_vulkan_context._device, m_global_render_resource._storage_buffer._global_upload_ringbuffer_memory, nullptr);

    vkUnmapMemory(m_vulkan_context._device,
                  m_global_render_resource._storage_buffer._axis_inefficient_storage_buffer_memory);
    vkDestroyBuffer(
        m_vulkan_context._device, m_global_render_resource._storage_buffer._axis_inefficient_storage_buffer, nullptr);
    vkFreeMemory(m_vulkan_context._device,
                 m_global_render_resource._storage_buffer._axis_inefficient_storage_buffer_memory,
                 nullptr);

    vkDestroyBuffer(m_vulkan_context._device,
                    m_global_render_resource._storage_buffer._global_null_descriptor_storage_buffer,
                    nullptr);
    vkFreeMemory(m_vulkan_context._device,
                 m_global_render_resource._storage_buffer._global_null_descriptor_storage_buffer_memory,
                 nullptr);

    PVulkanUtil::destroyMipmappedSampler(m_vulkan_context._device);
    PVulkanUtil::destroyNearestSampler(m_vulkan_context._device);

    for (uint32_t i = 0; i < m_max_frames_in_flight; i++)
    {
        vkDestroySemaphore(m_vulkan_context._device, m_image_available_for_render_semaphores[i], nullptr);
        vkDestroySemaphore(m_vulkan_context._device, m_image_finished_for_presentation_semaphores[i], nullptr);
        vkDestroyFence(m_vulkan_context._device, m_is_frame_in_flight_fences[i], nullptr);

        vkFreeCommandBuffers(m_vulkan_context._device, m_command_pools[i], 1, &m_command_buffers[i]);
        vkDestroyCommandPool(m_vulkan_context._device, m_command_pools[i], NULL);
    }

    vkDestroyCommandPool(m_vulkan_context._device, m_vulkan_context._command_pool, nullptr);

    vkDestroyDevice(m_vulkan_context._device,
                    nullptr); // when vulkan logical device is cleared, so is m_vulkan_context._graphics_queue
    vkDestroySurfaceKHR(m_vulkan_context._instance, m_vulkan_context._surface, nullptr);
    m_vulkan_context.clear();
    vkDestroyInstance(m_vulkan_context._instance,
                      nullptr); // when vulkan instance is cleared, so is
                                // m_vulkan_context._physical_device
}
