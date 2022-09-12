#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"
#include "runtime/function/render/rhi/vulkan/vulkan_util.h"

#include "runtime/function/global/global_context.h"
#include "runtime/function/render/passes/particle_pass.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_system.h"

#include "core/base/macro.h"
#include <fstream>

#include "particle_emit_comp.h"
#include "particle_kickoff_comp.h"
#include "particle_simulate_comp.h"
#include <particlebillboard_frag.h>
#include <particlebillboard_vert.h>

namespace Piccolo
{
    void ParticleEmitterBufferBatch::freeUpBatch(VkDevice device)
    {
        vkFreeMemory(device, m_counter_host_memory, nullptr);
        vkFreeMemory(device, m_position_host_memory, nullptr);
        vkFreeMemory(device, m_position_device_memory, nullptr);
        vkFreeMemory(device, m_counter_device_memory, nullptr);
        vkFreeMemory(device, m_indirect_dispatch_argument_memory, nullptr);
        vkFreeMemory(device, m_alive_list_memory, nullptr);
        vkFreeMemory(device, m_alive_list_next_memory, nullptr);
        vkFreeMemory(device, m_dead_list_memory, nullptr);
        vkFreeMemory(device, m_particle_component_res_memory, nullptr);
        vkFreeMemory(device, m_position_render_memory, nullptr);

        vkDestroyBuffer(device, m_particle_storage_buffer, nullptr);
        vkDestroyBuffer(device, m_position_render_buffer, nullptr);
        vkDestroyBuffer(device, m_position_device_buffer, nullptr);
        vkDestroyBuffer(device, m_position_host_buffer, nullptr);
        vkDestroyBuffer(device, m_counter_device_buffer, nullptr);
        vkDestroyBuffer(device, m_counter_host_buffer, nullptr);
        vkDestroyBuffer(device, m_indirect_dispatch_argument_buffer, nullptr);
        vkDestroyBuffer(device, m_alive_list_buffer, nullptr);
        vkDestroyBuffer(device, m_alive_list_next_buffer, nullptr);
        vkDestroyBuffer(device, m_dead_list_buffer, nullptr);
        vkDestroyBuffer(device, m_particle_component_res_buffer, nullptr);
    }

    void ParticlePass::copyNormalAndDepthImage()
    {
        uint8_t index = (m_vulkan_rhi->m_current_frame_index + m_vulkan_rhi->s_max_frames_in_flight - 1) %
                        m_vulkan_rhi->s_max_frames_in_flight;

        if (VK_SUCCESS !=
            vkWaitForFences(
                m_vulkan_rhi->m_device, 1, &m_vulkan_rhi->m_is_frame_in_flight_fences[index], VK_TRUE, UINT64_MAX))
            throw std::runtime_error("wait for fence");

        VkCommandBufferBeginInfo command_buffer_begin_info {};
        command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags            = 0;
        command_buffer_begin_info.pInheritanceInfo = nullptr;

        VkResult res_begin_command_buffer =
            m_vulkan_rhi->m_vk_begin_command_buffer(m_copy_command_buffer, &command_buffer_begin_info);
        assert(VK_SUCCESS == res_begin_command_buffer);

        if (m_vulkan_rhi->isDebugLabelEnabled())
        {
            VkDebugUtilsLabelEXT label_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                               NULL,
                                               "Copy Depth Image for Particle",
                                               {1.0f, 1.0f, 1.0f, 1.0f}};
            m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_copy_command_buffer, &label_info);
        }

        // depth image
        VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
        VkImageMemoryBarrier    imagememorybarrier {};
        imagememorybarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imagememorybarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imagememorybarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imagememorybarrier.subresourceRange    = subresourceRange;
        {
            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imagememorybarrier.srcAccessMask = 0;
            imagememorybarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imagememorybarrier.image         = m_dst_depth_image;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);

            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imagememorybarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            imagememorybarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imagememorybarrier.image         = m_src_depth_image;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);

            VkImageCopy imagecopyRegion    = {};
            imagecopyRegion.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
            imagecopyRegion.srcOffset      = {0, 0, 0};
            imagecopyRegion.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
            imagecopyRegion.dstOffset      = {0, 0, 0};
            imagecopyRegion.extent         = {
                m_vulkan_rhi->m_swapchain_extent.width, m_vulkan_rhi->m_swapchain_extent.height, 1};

            vkCmdCopyImage(m_copy_command_buffer,
                           m_src_depth_image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_dst_depth_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &imagecopyRegion);

            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            imagememorybarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imagememorybarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);

            imagememorybarrier.image         = m_dst_depth_image;
            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imagememorybarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imagememorybarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);
        }

        if (m_vulkan_rhi->isDebugLabelEnabled())
        {
            m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_copy_command_buffer);
        }

        if (m_vulkan_rhi->isDebugLabelEnabled())
        {
            VkDebugUtilsLabelEXT label_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                               NULL,
                                               "Copy Normal Image for Particle",
                                               {1.0f, 1.0f, 1.0f, 1.0f}};
            m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_copy_command_buffer, &label_info);
        }

        // color image
        subresourceRange                    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imagememorybarrier.subresourceRange = subresourceRange;
        {
            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imagememorybarrier.srcAccessMask = 0;
            imagememorybarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imagememorybarrier.image         = m_dst_normal_image;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);

            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imagememorybarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imagememorybarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imagememorybarrier.image         = m_src_normal_image;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);

            VkImageCopy imagecopyRegion    = {};
            imagecopyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            imagecopyRegion.srcOffset      = {0, 0, 0};
            imagecopyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            imagecopyRegion.dstOffset      = {0, 0, 0};
            imagecopyRegion.extent         = {
                m_vulkan_rhi->m_swapchain_extent.width, m_vulkan_rhi->m_swapchain_extent.height, 1};

            vkCmdCopyImage(m_copy_command_buffer,
                           m_src_normal_image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_dst_normal_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &imagecopyRegion);

            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imagememorybarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imagememorybarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);

            imagememorybarrier.image         = m_dst_normal_image;
            imagememorybarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imagememorybarrier.newLayout     = VK_IMAGE_LAYOUT_GENERAL;
            imagememorybarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imagememorybarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(m_copy_command_buffer,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imagememorybarrier);
        }

        if (m_vulkan_rhi->isDebugLabelEnabled())
        {
            m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_copy_command_buffer);
        }

        VkResult res_end_command_buffer = m_vulkan_rhi->m_vk_end_command_buffer(m_copy_command_buffer);
        assert(VK_SUCCESS == res_end_command_buffer);

        VkResult res_reset_fences = m_vulkan_rhi->m_vk_reset_fences(
            m_vulkan_rhi->m_device, 1, &m_vulkan_rhi->m_is_frame_in_flight_fences[index]);
        assert(VK_SUCCESS == res_reset_fences);

        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo         submit_info   = {};
        submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount     = 1;
        submit_info.pWaitSemaphores        = &m_vulkan_rhi->m_image_available_for_texturescopy_semaphores[index];
        submit_info.pWaitDstStageMask      = wait_stages;
        submit_info.commandBufferCount     = 1;
        submit_info.pCommandBuffers        = &m_copy_command_buffer;
        submit_info.signalSemaphoreCount   = 0;
        submit_info.pSignalSemaphores      = nullptr;
        VkResult res_queue_submit          = vkQueueSubmit(
            m_vulkan_rhi->m_graphics_queue, 1, &submit_info, m_vulkan_rhi->m_is_frame_in_flight_fences[index]);
        assert(VK_SUCCESS == res_queue_submit);

        vkQueueWaitIdle(m_vulkan_rhi->m_graphics_queue);
    }

    void ParticlePass::updateAfterFramebufferRecreate()
    {
        vkDestroyImage(m_vulkan_rhi->m_device, m_dst_depth_image, nullptr);
        vkFreeMemory(m_vulkan_rhi->m_device, m_dst_depth_image_memory, nullptr);

        VulkanUtil::createImage(m_vulkan_rhi->m_physical_device,
                                m_vulkan_rhi->m_device,
                                m_vulkan_rhi->m_swapchain_extent.width,
                                m_vulkan_rhi->m_swapchain_extent.height,
                                m_vulkan_rhi->m_depth_image_format,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_dst_depth_image,
                                m_dst_depth_image_memory,
                                0,
                                1,
                                1);

        vkDestroyImage(m_vulkan_rhi->m_device, m_dst_normal_image, nullptr);
        vkFreeMemory(m_vulkan_rhi->m_device, m_dst_normal_image_memory, nullptr);

        VulkanUtil::createImage(m_vulkan_rhi->m_physical_device,
                                m_vulkan_rhi->m_device,
                                m_vulkan_rhi->m_swapchain_extent.width,
                                m_vulkan_rhi->m_swapchain_extent.height,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_dst_normal_image,
                                m_dst_normal_image_memory,
                                0,
                                1,
                                1);

        m_src_depth_image_view  = VulkanUtil::createImageView(m_vulkan_rhi->m_device,
                                                             m_dst_depth_image,
                                                             m_vulkan_rhi->m_depth_image_format,
                                                             VK_IMAGE_ASPECT_DEPTH_BIT,
                                                             VK_IMAGE_VIEW_TYPE_2D,
                                                             1,
                                                             1);
        m_src_normal_image_view = VulkanUtil::createImageView(m_vulkan_rhi->m_device,
                                                              m_dst_normal_image,
                                                              VK_FORMAT_R8G8B8A8_UNORM,
                                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                                              VK_IMAGE_VIEW_TYPE_2D,
                                                              1,
                                                              1);

        updateDescriptorSet();
    }

    void ParticlePass::draw()
    {
        for (int i = 0; i < m_emitter_count; ++i)
        {
            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "ParticleBillboard", {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_render_command_buffer, &label_info);
            }

            m_vulkan_rhi->m_vk_cmd_bind_pipeline(
                m_render_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_render_pipelines[1].pipeline);
            m_vulkan_rhi->m_vk_cmd_set_viewport(m_render_command_buffer, 0, 1, &m_vulkan_rhi->m_viewport);
            m_vulkan_rhi->m_vk_cmd_set_scissor(m_render_command_buffer, 0, 1, &m_vulkan_rhi->m_scissor);

            m_vulkan_rhi->m_vk_cmd_bind_descriptor_sets(m_render_command_buffer,
                                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                        m_render_pipelines[1].layout,
                                                        0,
                                                        1,
                                                        &m_descriptor_infos[i * 3 + 2].descriptor_set,
                                                        0,
                                                        nullptr);

            vkCmdDraw(m_render_command_buffer, 4, m_emitter_buffer_batches[i].m_num_particle, 0, 0);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_render_command_buffer);
            }
        }
    }

    void ParticlePass::setupAttachments()
    {
        // billboard texture
        {
            std::shared_ptr<TextureData> m_particle_billboard_texture_resource = m_render_resource->loadTextureHDR(
                m_particle_manager->getGlobalParticleRes().m_particle_billboard_texture_path);
            VulkanUtil::createGlobalImage(m_vulkan_rhi.get(),
                                          m_particle_billboard_texture_image,
                                          m_particle_billboard_texture_image_view,
                                          m_particle_billboard_texture_vma_allocation,
                                          m_particle_billboard_texture_resource->m_width,
                                          m_particle_billboard_texture_resource->m_height,
                                          m_particle_billboard_texture_resource->m_pixels,
                                          m_particle_billboard_texture_resource->m_format);
        }

        // piccolo texture
        {
            std::shared_ptr<TextureData> m_piccolo_logo_texture_resource = m_render_resource->loadTexture(
                m_particle_manager->getGlobalParticleRes().m_piccolo_logo_texture_path, true);
            VulkanUtil::createGlobalImage(m_vulkan_rhi.get(),
                                          m_piccolo_logo_texture_image,
                                          m_piccolo_logo_texture_image_view,
                                          m_piccolo_logo_texture_vma_allocation,
                                          m_piccolo_logo_texture_resource->m_width,
                                          m_piccolo_logo_texture_resource->m_height,
                                          m_piccolo_logo_texture_resource->m_pixels,
                                          m_piccolo_logo_texture_resource->m_format);
        }

        VulkanUtil::createImage(m_vulkan_rhi->m_physical_device,
                                m_vulkan_rhi->m_device,
                                m_vulkan_rhi->m_swapchain_extent.width,
                                m_vulkan_rhi->m_swapchain_extent.height,
                                m_vulkan_rhi->m_depth_image_format,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_dst_depth_image,
                                m_dst_depth_image_memory,
                                0,
                                1,
                                1);

        VulkanUtil::createImage(m_vulkan_rhi->m_physical_device,
                                m_vulkan_rhi->m_device,
                                m_vulkan_rhi->m_swapchain_extent.width,
                                m_vulkan_rhi->m_swapchain_extent.height,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_dst_normal_image,
                                m_dst_normal_image_memory,
                                0,
                                1,
                                1);

        m_src_depth_image_view  = VulkanUtil::createImageView(m_vulkan_rhi->m_device,
                                                             m_dst_depth_image,
                                                             m_vulkan_rhi->m_depth_image_format,
                                                             VK_IMAGE_ASPECT_DEPTH_BIT,
                                                             VK_IMAGE_VIEW_TYPE_2D,
                                                             1,
                                                             1);
        m_src_normal_image_view = VulkanUtil::createImageView(m_vulkan_rhi->m_device,
                                                              m_dst_normal_image,
                                                              VK_FORMAT_R8G8B8A8_UNORM,
                                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                                              VK_IMAGE_VIEW_TYPE_2D,
                                                              1,
                                                              1);
    }

    void ParticlePass::setupParticleDescriptorSet()
    {
        for (int eid = 0; eid < m_emitter_count; ++eid)
        {
            VkDescriptorSetAllocateInfo particlebillboard_global_descriptor_set_alloc_info;
            particlebillboard_global_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            particlebillboard_global_descriptor_set_alloc_info.pNext = NULL;
            particlebillboard_global_descriptor_set_alloc_info.descriptorPool     = m_vulkan_rhi->m_descriptor_pool;
            particlebillboard_global_descriptor_set_alloc_info.descriptorSetCount = 1;
            particlebillboard_global_descriptor_set_alloc_info.pSetLayouts        = &m_descriptor_infos[2].layout;

            if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_rhi->m_device,
                                                       &particlebillboard_global_descriptor_set_alloc_info,
                                                       &m_descriptor_infos[eid * 3 + 2].descriptor_set))
            {
                throw std::runtime_error("allocate particle billboard global descriptor set");
            }

            VkDescriptorBufferInfo particlebillboard_perframe_storage_buffer_info = {};
            particlebillboard_perframe_storage_buffer_info.offset                 = 0;
            particlebillboard_perframe_storage_buffer_info.range                  = VK_WHOLE_SIZE;
            particlebillboard_perframe_storage_buffer_info.buffer                 = m_particle_billboard_uniform_buffer;

            VkDescriptorBufferInfo particlebillboard_perdrawcall_storage_buffer_info = {};
            particlebillboard_perdrawcall_storage_buffer_info.offset                 = 0;
            particlebillboard_perdrawcall_storage_buffer_info.range                  = VK_WHOLE_SIZE;
            particlebillboard_perdrawcall_storage_buffer_info.buffer =
                m_emitter_buffer_batches[eid].m_position_render_buffer;

            VkWriteDescriptorSet particlebillboard_descriptor_writes_info[3];

            particlebillboard_descriptor_writes_info[0].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            particlebillboard_descriptor_writes_info[0].pNext      = NULL;
            particlebillboard_descriptor_writes_info[0].dstSet     = m_descriptor_infos[eid * 3 + 2].descriptor_set;
            particlebillboard_descriptor_writes_info[0].dstBinding = 0;
            particlebillboard_descriptor_writes_info[0].dstArrayElement = 0;
            particlebillboard_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            particlebillboard_descriptor_writes_info[0].descriptorCount = 1;
            particlebillboard_descriptor_writes_info[0].pBufferInfo = &particlebillboard_perframe_storage_buffer_info;

            particlebillboard_descriptor_writes_info[1].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            particlebillboard_descriptor_writes_info[1].pNext      = NULL;
            particlebillboard_descriptor_writes_info[1].dstSet     = m_descriptor_infos[eid * 3 + 2].descriptor_set;
            particlebillboard_descriptor_writes_info[1].dstBinding = 1;
            particlebillboard_descriptor_writes_info[1].dstArrayElement = 0;
            particlebillboard_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            particlebillboard_descriptor_writes_info[1].descriptorCount = 1;
            particlebillboard_descriptor_writes_info[1].pBufferInfo =
                &particlebillboard_perdrawcall_storage_buffer_info;

            VkSampler           sampler;
            VkSamplerCreateInfo samplerCreateInfo {};
            samplerCreateInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.maxAnisotropy    = 1.0f;
            samplerCreateInfo.anisotropyEnable = true;
            samplerCreateInfo.magFilter        = VK_FILTER_LINEAR;
            samplerCreateInfo.minFilter        = VK_FILTER_LINEAR;
            samplerCreateInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCreateInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.mipLodBias       = 0.0f;
            samplerCreateInfo.compareOp        = VK_COMPARE_OP_NEVER;
            samplerCreateInfo.minLod           = 0.0f;
            samplerCreateInfo.maxLod           = 0.0f;
            samplerCreateInfo.borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            if (VK_SUCCESS != vkCreateSampler(m_vulkan_rhi->m_device, &samplerCreateInfo, nullptr, &sampler))
            {
                throw std::runtime_error("create sampler error");
            }

            VkDescriptorImageInfo particle_texture_image_info = {};
            particle_texture_image_info.sampler               = sampler;
            particle_texture_image_info.imageView             = m_particle_billboard_texture_image_view;
            particle_texture_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            particlebillboard_descriptor_writes_info[2].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            particlebillboard_descriptor_writes_info[2].pNext      = NULL;
            particlebillboard_descriptor_writes_info[2].dstSet     = m_descriptor_infos[eid * 3 + 2].descriptor_set;
            particlebillboard_descriptor_writes_info[2].dstBinding = 2;
            particlebillboard_descriptor_writes_info[2].dstArrayElement = 0;
            particlebillboard_descriptor_writes_info[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            particlebillboard_descriptor_writes_info[2].descriptorCount = 1;
            particlebillboard_descriptor_writes_info[2].pImageInfo      = &particle_texture_image_info;

            vkUpdateDescriptorSets(m_vulkan_rhi->m_device, 3, particlebillboard_descriptor_writes_info, 0, NULL);
        }
    }

    void ParticlePass::setEmitterCount(int count)
    {
        for (int i = 0; i < m_emitter_buffer_batches.size(); ++i)
        {
            m_emitter_buffer_batches[i].freeUpBatch(m_vulkan_rhi->m_device);
        }

        m_emitter_count = count;
        m_emitter_buffer_batches.resize(m_emitter_count);
    }

    void ParticlePass::createEmitter(int id, const ParticleEmitterDesc& desc)
    {
        const VkDeviceSize counterBufferSize = sizeof(ParticleCounter);
        ParticleCounter    counter;
        counter.alive_count           = m_emitter_buffer_batches[id].m_num_particle;
        counter.dead_count            = s_max_particles - m_emitter_buffer_batches[id].m_num_particle;
        counter.emit_count            = 0;
        counter.alive_count_after_sim = m_emitter_buffer_batches[id].m_num_particle;

        if constexpr (s_verbose_particle_alive_info)
        {
            LOG_INFO("Emitter {} info:", id);
            LOG_INFO("Dead {}, Alive {}, After sim {}, Emit {}",
                     counter.dead_count,
                     counter.alive_count,
                     counter.alive_count_after_sim,
                     counter.emit_count);
        }

        {
            const VkDeviceSize      indirectArgumentSize = sizeof(IndirectArgumemt);
            struct IndirectArgumemt indirectargument     = {};
            indirectargument.alive_flap_bit              = 1;
            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                      VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                  &m_emitter_buffer_batches[id].m_indirect_dispatch_argument_buffer,
                                                  &m_emitter_buffer_batches[id].m_indirect_dispatch_argument_memory,
                                                  indirectArgumentSize,
                                                  &indirectargument,
                                                  indirectArgumentSize);

            const VkDeviceSize aliveListSize = 4 * sizeof(uint32_t) * s_max_particles;
            std::vector<int>   aliveindices(s_max_particles * 4, 0);
            for (int i = 0; i < s_max_particles; ++i)
                aliveindices[i * 4] = i;
            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                  &m_emitter_buffer_batches[id].m_alive_list_buffer,
                                                  &m_emitter_buffer_batches[id].m_alive_list_memory,
                                                  aliveListSize,
                                                  aliveindices.data(),
                                                  aliveListSize);

            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  &m_emitter_buffer_batches[id].m_alive_list_next_buffer,
                                                  &m_emitter_buffer_batches[id].m_alive_list_next_memory,
                                                  aliveListSize);

            const VkDeviceSize   deadListSize = 4 * sizeof(uint32_t) * s_max_particles;
            std::vector<int32_t> deadindices(s_max_particles * 4, 0);
            for (int32_t i = 0; i < s_max_particles; ++i)
                deadindices[i * 4] = s_max_particles - 1 - i;
            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                  &m_emitter_buffer_batches[id].m_dead_list_buffer,
                                                  &m_emitter_buffer_batches[id].m_dead_list_memory,
                                                  deadListSize,
                                                  deadindices.data(),
                                                  deadListSize);
        }

        VkFence         fence;
        ParticleCounter counterNext {};
        {

            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                  &m_emitter_buffer_batches[id].m_counter_host_buffer,
                                                  &m_emitter_buffer_batches[id].m_counter_host_memory,
                                                  counterBufferSize,
                                                  &counter,
                                                  sizeof(counter));

            // Flush writes to host visible buffer
            void* mapped;

            vkMapMemory(m_vulkan_rhi->m_device,
                        m_emitter_buffer_batches[id].m_counter_host_memory,
                        0,
                        VK_WHOLE_SIZE,
                        0,
                        &mapped);
            VkMappedMemoryRange mappedRange {};
            mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = m_emitter_buffer_batches[id].m_counter_host_memory;
            mappedRange.offset = 0;
            mappedRange.size   = VK_WHOLE_SIZE;
            vkFlushMappedMemoryRanges(m_vulkan_rhi->m_device, 1, &mappedRange);
            vkUnmapMemory(m_vulkan_rhi->m_device, m_emitter_buffer_batches[id].m_counter_host_memory);

            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  &m_emitter_buffer_batches[id].m_counter_device_buffer,
                                                  &m_emitter_buffer_batches[id].m_counter_device_memory,
                                                  counterBufferSize);

            // Copy to staging buffer
            VkCommandBufferAllocateInfo cmdBufAllocateInfo {};
            cmdBufAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocateInfo.commandPool        = m_vulkan_rhi->m_command_pool;
            cmdBufAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocateInfo.commandBufferCount = 1;
            VkCommandBuffer copyCmd;
            if (VK_SUCCESS != vkAllocateCommandBuffers(m_vulkan_rhi->m_device, &cmdBufAllocateInfo, &copyCmd))
            {
                throw std::runtime_error("alloc command buffer");
            }

            VkCommandBufferBeginInfo cmdBufInfo {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (VK_SUCCESS != vkBeginCommandBuffer(copyCmd, &cmdBufInfo))
            {
                throw std::runtime_error("begin command buffer");
            }

            VkBufferCopy copyRegion = {};
            copyRegion.size         = counterBufferSize;
            vkCmdCopyBuffer(copyCmd,
                            m_emitter_buffer_batches[id].m_counter_host_buffer,
                            m_emitter_buffer_batches[id].m_counter_device_buffer,
                            1,
                            &copyRegion);
            if (VK_SUCCESS != vkEndCommandBuffer(copyCmd))
            {
                throw std::runtime_error("buffer copy");
            }

            VkSubmitInfo submitInfo {};
            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &copyCmd;
            VkFenceCreateInfo fenceInfo {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = 0;
            if (VK_SUCCESS != vkCreateFence(m_vulkan_rhi->m_device, &fenceInfo, nullptr, &fence))
            {
                throw std::runtime_error("create fence");
            }

            // Submit to the queue
            if (VK_SUCCESS != vkQueueSubmit(m_vulkan_rhi->m_compute_queue, 1, &submitInfo, fence))
            {
                throw std::runtime_error("queue submit");
            }
            if (VK_SUCCESS != vkWaitForFences(m_vulkan_rhi->m_device, 1, &fence, VK_TRUE, UINT64_MAX))
            {
                throw std::runtime_error("wait fence submit");
            }

            vkDestroyFence(m_vulkan_rhi->m_device, fence, nullptr);
            vkFreeCommandBuffers(m_vulkan_rhi->m_device, m_vulkan_rhi->m_command_pool, 1, &copyCmd);
        }

        const VkDeviceSize staggingBuferSize        = s_max_particles * sizeof(Particle);
        m_emitter_buffer_batches[id].m_emitter_desc = desc;

        // fill in data
        {
            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                  &m_emitter_buffer_batches[id].m_particle_component_res_buffer,
                                                  &m_emitter_buffer_batches[id].m_particle_component_res_memory,
                                                  sizeof(ParticleEmitterDesc),
                                                  &m_emitter_buffer_batches[id].m_emitter_desc,
                                                  sizeof(ParticleEmitterDesc));

            if (VK_SUCCESS != vkMapMemory(m_vulkan_rhi->m_device,
                                          m_emitter_buffer_batches[id].m_particle_component_res_memory,
                                          0,
                                          VK_WHOLE_SIZE,
                                          0,
                                          &m_emitter_buffer_batches[id].m_emitter_desc_mapped))
            {
                throw std::runtime_error("map emitter component res buffer");
            }

            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                  &m_emitter_buffer_batches[id].m_position_host_buffer,
                                                  &m_emitter_buffer_batches[id].m_position_host_memory,
                                                  staggingBuferSize);

            // Flush writes to host visible buffer
            void* mapped;
            vkMapMemory(m_vulkan_rhi->m_device,
                        m_emitter_buffer_batches[id].m_position_host_memory,
                        0,
                        VK_WHOLE_SIZE,
                        0,
                        &mapped);
            VkMappedMemoryRange mappedRange {};
            mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = m_emitter_buffer_batches[id].m_position_host_memory;
            mappedRange.offset = 0;
            mappedRange.size   = VK_WHOLE_SIZE;
            vkFlushMappedMemoryRanges(m_vulkan_rhi->m_device, 1, &mappedRange);
            vkUnmapMemory(m_vulkan_rhi->m_device, m_emitter_buffer_batches[id].m_position_host_memory);

            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  &m_emitter_buffer_batches[id].m_position_device_buffer,
                                                  &m_emitter_buffer_batches[id].m_position_device_memory,
                                                  staggingBuferSize);

            VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                                  m_vulkan_rhi->m_physical_device,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  &m_emitter_buffer_batches[id].m_position_render_buffer,
                                                  &m_emitter_buffer_batches[id].m_position_render_memory,
                                                  staggingBuferSize);

            // Copy to staging buffer
            VkCommandBufferAllocateInfo cmdBufAllocateInfo {};
            cmdBufAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocateInfo.commandPool        = m_vulkan_rhi->m_command_pool;
            cmdBufAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocateInfo.commandBufferCount = 1;
            VkCommandBuffer copyCmd;
            if (VK_SUCCESS != vkAllocateCommandBuffers(m_vulkan_rhi->m_device, &cmdBufAllocateInfo, &copyCmd))
            {
                throw std::runtime_error("alloc command buffer");
            }
            VkCommandBufferBeginInfo cmdBufInfo {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (VK_SUCCESS != vkBeginCommandBuffer(copyCmd, &cmdBufInfo))
            {
                throw std::runtime_error("begin command buffer");
            }

            VkBufferCopy copyRegion = {};
            copyRegion.size         = staggingBuferSize;
            vkCmdCopyBuffer(copyCmd,
                            m_emitter_buffer_batches[id].m_position_host_buffer,
                            m_emitter_buffer_batches[id].m_position_device_buffer,
                            1,
                            &copyRegion);
            if (VK_SUCCESS != vkEndCommandBuffer(copyCmd))
            {
                throw std::runtime_error("buffer copy");
            }

            VkSubmitInfo submitInfo {};
            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &copyCmd;
            VkFenceCreateInfo fenceInfo {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = 0;
            if (VK_SUCCESS != vkCreateFence(m_vulkan_rhi->m_device, &fenceInfo, nullptr, &fence))
            {
                throw std::runtime_error("create fence");
            }

            // Submit to the queue
            if (VK_SUCCESS != vkQueueSubmit(m_vulkan_rhi->m_compute_queue, 1, &submitInfo, fence))
            {
                throw std::runtime_error("queue submit");
            }
            if (VK_SUCCESS != vkWaitForFences(m_vulkan_rhi->m_device, 1, &fence, VK_TRUE, UINT64_MAX))
            {
                throw std::runtime_error("wait fence submit");
            }

            vkDestroyFence(m_vulkan_rhi->m_device, fence, nullptr);
            vkFreeCommandBuffers(m_vulkan_rhi->m_device, m_vulkan_rhi->m_command_pool, 1, &copyCmd);
        }
    }

    void ParticlePass::initializeEmitters()
    {
        allocateDescriptorSet();
        updateDescriptorSet();
        setupParticleDescriptorSet();
    }

    void ParticlePass::setupParticlePass()
    {
        prepareUniformBuffer();
        setupDescriptorSetLayout();
        setupPipelines();
        setupAttachments();

        VkCommandBufferAllocateInfo cmdBufAllocateInfo {};
        cmdBufAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool        = m_vulkan_rhi->m_command_pool;
        cmdBufAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;
        if (VK_SUCCESS !=
            vkAllocateCommandBuffers(m_vulkan_rhi->m_device, &cmdBufAllocateInfo, &m_compute_command_buffer))
            throw std::runtime_error("alloc compute command buffer");
        if (VK_SUCCESS != vkAllocateCommandBuffers(m_vulkan_rhi->m_device, &cmdBufAllocateInfo, &m_copy_command_buffer))
            throw std::runtime_error("alloc copyt command buffer");

        VkFenceCreateInfo fenceCreateInfo {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        if (VK_SUCCESS != vkCreateFence(m_vulkan_rhi->m_device, &fenceCreateInfo, nullptr, &m_fence))
            throw std::runtime_error("create fence");
    }

    void ParticlePass::initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::initialize(nullptr);

        const ParticlePassInitInfo* _init_info = static_cast<const ParticlePassInitInfo*>(init_info);
        m_particle_manager                     = _init_info->m_particle_manager;
    }

    void ParticlePass::setupDescriptorSetLayout()
    {
        m_descriptor_infos.resize(3);

        // compute descriptor sets
        {
            VkDescriptorSetLayoutBinding particle_layout_bindings[11] = {};
            {
                VkDescriptorSetLayoutBinding& uniform_layout_bingding = particle_layout_bindings[0];
                uniform_layout_bingding.binding                       = 0;
                uniform_layout_bingding.descriptorType                = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uniform_layout_bingding.descriptorCount               = 1;
                uniform_layout_bingding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& storage_position_layout_binding = particle_layout_bindings[1];
                storage_position_layout_binding.binding                       = 1;
                storage_position_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                storage_position_layout_binding.descriptorCount               = 1;
                storage_position_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& storage_counter_layout_binding = particle_layout_bindings[2];
                storage_counter_layout_binding.binding                       = 2;
                storage_counter_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                storage_counter_layout_binding.descriptorCount               = 1;
                storage_counter_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& storage_indirectargument_layout_binding = particle_layout_bindings[3];
                storage_indirectargument_layout_binding.binding                       = 3;
                storage_indirectargument_layout_binding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                storage_indirectargument_layout_binding.descriptorCount = 1;
                storage_indirectargument_layout_binding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& alive_list_layout_binding = particle_layout_bindings[4];
                alive_list_layout_binding.binding                       = 4;
                alive_list_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                alive_list_layout_binding.descriptorCount               = 1;
                alive_list_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& dead_list_layout_binding = particle_layout_bindings[5];
                dead_list_layout_binding.binding                       = 5;
                dead_list_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                dead_list_layout_binding.descriptorCount               = 1;
                dead_list_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& alive_list_next_layout_binding = particle_layout_bindings[6];
                alive_list_next_layout_binding.binding                       = 6;
                alive_list_next_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                alive_list_next_layout_binding.descriptorCount               = 1;
                alive_list_next_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& particle_res_layout_binding = particle_layout_bindings[7];
                particle_res_layout_binding.binding                       = 7;
                particle_res_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                particle_res_layout_binding.descriptorCount               = 1;
                particle_res_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& scene_uniformbuffer_layout_binding = particle_layout_bindings[8];
                scene_uniformbuffer_layout_binding.binding                       = 8;
                scene_uniformbuffer_layout_binding.descriptorType                = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                scene_uniformbuffer_layout_binding.descriptorCount               = 1;
                scene_uniformbuffer_layout_binding.stageFlags                    = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& storage_render_position_layout_binding = particle_layout_bindings[9];
                storage_render_position_layout_binding.binding                       = 9;
                storage_render_position_layout_binding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                storage_render_position_layout_binding.descriptorCount = 1;
                storage_render_position_layout_binding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            {
                VkDescriptorSetLayoutBinding& piccolo_texture_layout_binding = particle_layout_bindings[10];
                piccolo_texture_layout_binding.binding                       = 10;
                piccolo_texture_layout_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                piccolo_texture_layout_binding.descriptorCount = 1;
                piccolo_texture_layout_binding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            VkDescriptorSetLayoutCreateInfo particle_descriptor_layout_create_info;
            particle_descriptor_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            particle_descriptor_layout_create_info.pNext = NULL;
            particle_descriptor_layout_create_info.flags = 0;
            particle_descriptor_layout_create_info.bindingCount =
                sizeof(particle_layout_bindings) / sizeof(particle_layout_bindings[0]);
            particle_descriptor_layout_create_info.pBindings = particle_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_vulkan_rhi->m_device,
                                                          &particle_descriptor_layout_create_info,
                                                          NULL,
                                                          &m_descriptor_infos[0].layout))
            {
                throw std::runtime_error("setup particle compute Descriptor done");
            }
            LOG_INFO("setup particle compute Descriptor done");
        }
        // scene depth and normal binding
        {
            VkDescriptorSetLayoutBinding scene_global_layout_bindings[2] = {};

            VkDescriptorSetLayoutBinding& gbuffer_normal_global_layout_input_attachment_binding =
                scene_global_layout_bindings[0];
            gbuffer_normal_global_layout_input_attachment_binding.binding         = 0;
            gbuffer_normal_global_layout_input_attachment_binding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            gbuffer_normal_global_layout_input_attachment_binding.descriptorCount = 1;
            gbuffer_normal_global_layout_input_attachment_binding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutBinding& gbuffer_depth_global_layout_input_attachment_binding =
                scene_global_layout_bindings[1];
            gbuffer_depth_global_layout_input_attachment_binding.binding = 1;
            gbuffer_depth_global_layout_input_attachment_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            gbuffer_depth_global_layout_input_attachment_binding.descriptorCount = 1;
            gbuffer_depth_global_layout_input_attachment_binding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutCreateInfo gbuffer_lighting_global_layout_create_info;
            gbuffer_lighting_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            gbuffer_lighting_global_layout_create_info.pNext = NULL;
            gbuffer_lighting_global_layout_create_info.flags = 0;
            gbuffer_lighting_global_layout_create_info.bindingCount =
                sizeof(scene_global_layout_bindings) / sizeof(scene_global_layout_bindings[0]);
            gbuffer_lighting_global_layout_create_info.pBindings = scene_global_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_vulkan_rhi->m_device,
                                                          &gbuffer_lighting_global_layout_create_info,
                                                          NULL,
                                                          &m_descriptor_infos[1].layout))
                throw std::runtime_error("create scene normal and depth global layout");
        }

        {
            VkDescriptorSetLayoutBinding particlebillboard_global_layout_bindings[3];

            VkDescriptorSetLayoutBinding& particlebillboard_global_layout_perframe_storage_buffer_binding =
                particlebillboard_global_layout_bindings[0];
            particlebillboard_global_layout_perframe_storage_buffer_binding.binding = 0;
            particlebillboard_global_layout_perframe_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            particlebillboard_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            particlebillboard_global_layout_perframe_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            particlebillboard_global_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& particlebillboard_global_layout_perdrawcall_storage_buffer_binding =
                particlebillboard_global_layout_bindings[1];
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& particlebillboard_global_layout_texture_binding =
                particlebillboard_global_layout_bindings[2];
            particlebillboard_global_layout_texture_binding.binding         = 2;
            particlebillboard_global_layout_texture_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            particlebillboard_global_layout_texture_binding.descriptorCount = 1;
            particlebillboard_global_layout_texture_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
            particlebillboard_global_layout_texture_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutCreateInfo particlebillboard_global_layout_create_info;
            particlebillboard_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            particlebillboard_global_layout_create_info.pNext = NULL;
            particlebillboard_global_layout_create_info.flags = 0;
            particlebillboard_global_layout_create_info.bindingCount = 3;
            particlebillboard_global_layout_create_info.pBindings    = particlebillboard_global_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_vulkan_rhi->m_device,
                                                          &particlebillboard_global_layout_create_info,
                                                          NULL,
                                                          &m_descriptor_infos[2].layout))
            {
                throw std::runtime_error("create particle billboard global layout");
            }
        }
    }

    void ParticlePass::setupPipelines()
    {
        m_render_pipelines.resize(2);

        // compute pipeline
        {
            VkDescriptorSetLayout      descriptorset_layouts[2] = {m_descriptor_infos[0].layout,
                                                              m_descriptor_infos[1].layout};
            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount =
                sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]);
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (vkCreatePipelineLayout(
                    m_vulkan_rhi->m_device, &pipeline_layout_create_info, nullptr, &m_render_pipelines[0].layout) !=
                VK_SUCCESS)
                throw std::runtime_error("create compute pass pipe layout");
            LOG_INFO("compute pipe layout done");
        }

        VkPipelineCache           pipelineCache;
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        if (VK_SUCCESS !=
            vkCreatePipelineCache(m_vulkan_rhi->m_device, &pipelineCacheCreateInfo, nullptr, &pipelineCache))
        {
            throw std::runtime_error("create particle cache");
        }

        struct SpecializationData
        {
            uint32_t BUFFER_ELEMENT_COUNT = 32;
        } specializationData;

        VkSpecializationMapEntry specializationMapEntry {};
        specializationMapEntry.constantID = 0;
        specializationMapEntry.offset     = 0;
        specializationMapEntry.size       = sizeof(uint32_t);

        VkSpecializationInfo specializationInfo {};
        specializationInfo.mapEntryCount = 1;
        specializationInfo.pMapEntries   = &specializationMapEntry;
        specializationInfo.dataSize      = sizeof(specializationData);
        specializationInfo.pData         = &specializationData;

        VkComputePipelineCreateInfo computePipelineCreateInfo {};

        computePipelineCreateInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.layout = m_render_pipelines[0].layout;
        computePipelineCreateInfo.flags  = 0;

        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage                           = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.pName                           = "main";

        {
            shaderStage.module = VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, PARTICLE_KICKOFF_COMP);
            shaderStage.pSpecializationInfo = nullptr;
            assert(shaderStage.module != VK_NULL_HANDLE);

            computePipelineCreateInfo.stage = shaderStage;
            if (VK_SUCCESS !=
                vkCreateComputePipelines(
                    m_vulkan_rhi->m_device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &m_kickoff_pipeline))
            {
                throw std::runtime_error("create particle kickoff pipe");
            }
        }

        {
            shaderStage.module = VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, PARTICLE_EMIT_COMP);
            shaderStage.pSpecializationInfo = nullptr;
            assert(shaderStage.module != VK_NULL_HANDLE);

            computePipelineCreateInfo.stage = shaderStage;
            if (VK_SUCCESS !=
                vkCreateComputePipelines(
                    m_vulkan_rhi->m_device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &m_emit_pipeline))
            {
                throw std::runtime_error("create particle emit pipe");
            }
        }

        {
            shaderStage.module = VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, PARTICLE_SIMULATE_COMP);
            shaderStage.pSpecializationInfo = nullptr;
            assert(shaderStage.module != VK_NULL_HANDLE);

            computePipelineCreateInfo.stage = shaderStage;
            if (VK_SUCCESS != vkCreateComputePipelines(m_vulkan_rhi->m_device,
                                                       pipelineCache,
                                                       1,
                                                       &computePipelineCreateInfo,
                                                       nullptr,
                                                       &m_simulate_pipeline))
            {
                throw std::runtime_error("create particle simulate pipe");
            }
        }

        // particle billboard
        {
            VkDescriptorSetLayout      descriptorset_layouts[1] = {m_descriptor_infos[2].layout};
            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

            if (vkCreatePipelineLayout(
                    m_vulkan_rhi->m_device, &pipeline_layout_create_info, nullptr, &m_render_pipelines[1].layout) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("create particle billboard pipeline layout");
            }

            VkShaderModule vert_shader_module =
                VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, PARTICLEBILLBOARD_VERT);
            VkShaderModule frag_shader_module =
                VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, PARTICLEBILLBOARD_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
            vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName  = "main";

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
            frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName  = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info};

            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
            vertex_input_state_create_info.pVertexBindingDescriptions      = NULL;
            vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
            vertex_input_state_create_info.pVertexAttributeDescriptions    = NULL;

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
            input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info {};
            viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports    = &m_vulkan_rhi->m_viewport;
            viewport_state_create_info.scissorCount  = 1;
            viewport_state_create_info.pScissors     = &m_vulkan_rhi->m_scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable        = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth               = 1.0f;
            rasterization_state_create_info.cullMode                = VK_CULL_MODE_NONE;
            rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp          = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachments[1] = {};
            color_blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachments[0].blendEnable         = VK_TRUE;
            color_blend_attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            color_blend_attachments[0].colorBlendOp        = VK_BLEND_OP_ADD;
            color_blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachments[0].alphaBlendOp        = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
            color_blend_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp       = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount =
                sizeof(color_blend_attachments) / sizeof(color_blend_attachments[0]);
            color_blend_state_create_info.pAttachments      = &color_blend_attachments[0];
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info {};
            depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable  = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_FALSE;
            depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS;
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable     = VK_FALSE;

            VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

            VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
            dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.dynamicStateCount = 2;
            dynamic_state_create_info.pDynamicStates    = dynamic_states;

            VkGraphicsPipelineCreateInfo pipelineInfo {};
            pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount          = 2;
            pipelineInfo.pStages             = shader_stages;
            pipelineInfo.pVertexInputState   = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
            pipelineInfo.pViewportState      = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &rasterization_state_create_info;
            pipelineInfo.pMultisampleState   = &multisample_state_create_info;
            pipelineInfo.pColorBlendState    = &color_blend_state_create_info;
            pipelineInfo.pDepthStencilState  = &depth_stencil_create_info;
            pipelineInfo.layout              = m_render_pipelines[1].layout;
            pipelineInfo.renderPass          = m_render_pass;
            pipelineInfo.subpass             = _main_camera_subpass_forward_lighting;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState       = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_vulkan_rhi->m_device,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &m_render_pipelines[1].pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create particle billboard graphics pipeline");
            }

            vkDestroyShaderModule(m_vulkan_rhi->m_device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_vulkan_rhi->m_device, frag_shader_module, nullptr);
        }
    }

    void ParticlePass::allocateDescriptorSet()
    {
        VkDescriptorSetAllocateInfo particle_descriptor_set_alloc_info;
        particle_descriptor_set_alloc_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        particle_descriptor_set_alloc_info.descriptorPool = m_vulkan_rhi->m_descriptor_pool;

        m_descriptor_infos.resize(3 * m_emitter_count);
        for (int eid = 0; eid < m_emitter_count; ++eid)
        {
            particle_descriptor_set_alloc_info.pSetLayouts        = &m_descriptor_infos[0].layout;
            particle_descriptor_set_alloc_info.descriptorSetCount = 1;
            particle_descriptor_set_alloc_info.pNext              = NULL;
            if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_rhi->m_device,
                                                       &particle_descriptor_set_alloc_info,
                                                       &m_descriptor_infos[eid * 3].descriptor_set))
                throw std::runtime_error("allocate compute descriptor set");
            particle_descriptor_set_alloc_info.pSetLayouts        = &m_descriptor_infos[1].layout;
            particle_descriptor_set_alloc_info.descriptorSetCount = 1;
            particle_descriptor_set_alloc_info.pNext              = NULL;

            if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_rhi->m_device,
                                                       &particle_descriptor_set_alloc_info,
                                                       &m_descriptor_infos[eid * 3 + 1].descriptor_set))
                LOG_INFO("allocate normal and depth descriptor set done");
        }
    }

    void ParticlePass::updateDescriptorSet()
    {
        for (int eid = 0; eid < m_emitter_count; ++eid)
        {
            // compute part
            {
                std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets {
                    {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};

                VkDescriptorBufferInfo uniformbufferDescriptor = {m_compute_uniform_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[0];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorset.dstBinding            = 0;
                    descriptorset.pBufferInfo           = &uniformbufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo positionBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_position_device_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[1];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 1;
                    descriptorset.pBufferInfo           = &positionBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo counterBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_counter_device_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[2];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 2;
                    descriptorset.pBufferInfo           = &counterBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo indirectArgumentBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_indirect_dispatch_argument_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[3];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 3;
                    descriptorset.pBufferInfo           = &indirectArgumentBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo aliveListBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_alive_list_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[4];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 4;
                    descriptorset.pBufferInfo           = &aliveListBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo deadListBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_dead_list_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[5];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 5;
                    descriptorset.pBufferInfo           = &deadListBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo aliveListNextBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_alive_list_next_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[6];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 6;
                    descriptorset.pBufferInfo           = &aliveListNextBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo particleComponentResBufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_particle_component_res_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[7];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 7;
                    descriptorset.pBufferInfo           = &particleComponentResBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo particleSceneUniformBufferDescriptor = {
                    m_scene_uniform_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[8];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorset.dstBinding            = 8;
                    descriptorset.pBufferInfo           = &particleSceneUniformBufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkDescriptorBufferInfo positionRenderbufferDescriptor = {
                    m_emitter_buffer_batches[eid].m_position_render_buffer, 0, VK_WHOLE_SIZE};
                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[9];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorset.dstBinding            = 9;
                    descriptorset.pBufferInfo           = &positionRenderbufferDescriptor;
                    descriptorset.descriptorCount       = 1;
                }

                VkSampler           sampler;
                VkSamplerCreateInfo samplerCreateInfo {};
                samplerCreateInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerCreateInfo.maxAnisotropy    = 1.0f;
                samplerCreateInfo.anisotropyEnable = true;
                samplerCreateInfo.magFilter        = VK_FILTER_LINEAR;
                samplerCreateInfo.minFilter        = VK_FILTER_LINEAR;
                samplerCreateInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerCreateInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerCreateInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerCreateInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerCreateInfo.mipLodBias       = 0.0f;
                samplerCreateInfo.compareOp        = VK_COMPARE_OP_NEVER;
                samplerCreateInfo.minLod           = 0.0f;
                samplerCreateInfo.maxLod           = 0.0f;
                samplerCreateInfo.borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                if (VK_SUCCESS != vkCreateSampler(m_vulkan_rhi->m_device, &samplerCreateInfo, nullptr, &sampler))
                {
                    throw std::runtime_error("create sampler error");
                }

                VkDescriptorImageInfo piccolo_texture_image_info = {};
                piccolo_texture_image_info.sampler               = sampler;
                piccolo_texture_image_info.imageView             = m_piccolo_logo_texture_image_view;
                piccolo_texture_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                {
                    VkWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[10];
                    descriptorset.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorset.dstSet                = m_descriptor_infos[eid * 3].descriptor_set;
                    descriptorset.descriptorType        = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorset.dstBinding            = 10;
                    descriptorset.pImageInfo            = &piccolo_texture_image_info;
                    descriptorset.descriptorCount       = 1;
                }

                vkUpdateDescriptorSets(m_vulkan_rhi->m_device,
                                       static_cast<uint32_t>(computeWriteDescriptorSets.size()),
                                       computeWriteDescriptorSets.data(),
                                       0,
                                       NULL);
            }
            {
                VkWriteDescriptorSet descriptor_input_attachment_writes_info[2] = {{}, {}};

                VkDescriptorImageInfo gbuffer_normal_descriptor_image_info = {};
                gbuffer_normal_descriptor_image_info.sampler               = nullptr;
                gbuffer_normal_descriptor_image_info.imageView             = m_src_normal_image_view;
                gbuffer_normal_descriptor_image_info.imageLayout           = VK_IMAGE_LAYOUT_GENERAL;
                {

                    VkWriteDescriptorSet& gbuffer_normal_descriptor_input_attachment_write_info =
                        descriptor_input_attachment_writes_info[0];
                    gbuffer_normal_descriptor_input_attachment_write_info.sType =
                        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    gbuffer_normal_descriptor_input_attachment_write_info.pNext = NULL;
                    gbuffer_normal_descriptor_input_attachment_write_info.dstSet =
                        m_descriptor_infos[eid * 3 + 1].descriptor_set;
                    gbuffer_normal_descriptor_input_attachment_write_info.dstBinding      = 0;
                    gbuffer_normal_descriptor_input_attachment_write_info.dstArrayElement = 0;
                    gbuffer_normal_descriptor_input_attachment_write_info.descriptorType =
                        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    gbuffer_normal_descriptor_input_attachment_write_info.descriptorCount = 1;
                    gbuffer_normal_descriptor_input_attachment_write_info.pImageInfo =
                        &gbuffer_normal_descriptor_image_info;
                }

                VkSampler           sampler;
                VkSamplerCreateInfo samplerCreateInfo {};
                samplerCreateInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerCreateInfo.maxAnisotropy    = 1.0f;
                samplerCreateInfo.anisotropyEnable = true;
                samplerCreateInfo.magFilter        = VK_FILTER_NEAREST;
                samplerCreateInfo.minFilter        = VK_FILTER_NEAREST;
                samplerCreateInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerCreateInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerCreateInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerCreateInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerCreateInfo.mipLodBias       = 0.0f;
                samplerCreateInfo.compareOp        = VK_COMPARE_OP_NEVER;
                samplerCreateInfo.minLod           = 0.0f;
                samplerCreateInfo.maxLod           = 0.0f;
                samplerCreateInfo.borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                if (VK_SUCCESS != vkCreateSampler(m_vulkan_rhi->m_device, &samplerCreateInfo, nullptr, &sampler))
                {
                    throw std::runtime_error("create sampler error");
                }

                VkDescriptorImageInfo depth_descriptor_image_info = {};
                depth_descriptor_image_info.sampler               = sampler;
                depth_descriptor_image_info.imageView             = m_src_depth_image_view;
                depth_descriptor_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                {
                    VkWriteDescriptorSet& depth_descriptor_input_attachment_write_info =
                        descriptor_input_attachment_writes_info[1];
                    depth_descriptor_input_attachment_write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    depth_descriptor_input_attachment_write_info.pNext = NULL;
                    depth_descriptor_input_attachment_write_info.dstSet =
                        m_descriptor_infos[eid * 3 + 1].descriptor_set;
                    depth_descriptor_input_attachment_write_info.dstBinding      = 1;
                    depth_descriptor_input_attachment_write_info.dstArrayElement = 0;
                    depth_descriptor_input_attachment_write_info.descriptorType =
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    depth_descriptor_input_attachment_write_info.descriptorCount = 1;
                    depth_descriptor_input_attachment_write_info.pImageInfo      = &depth_descriptor_image_info;
                }

                vkUpdateDescriptorSets(m_vulkan_rhi->m_device,
                                       sizeof(descriptor_input_attachment_writes_info) /
                                           sizeof(descriptor_input_attachment_writes_info[0]),
                                       descriptor_input_attachment_writes_info,
                                       0,
                                       NULL);
            }
        }
    }

    void ParticlePass::simulate()
    {
        for (auto i : m_emitter_tick_indices)
        {
            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Particlecompute", {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_compute_command_buffer, &label_info);
            }

            VkCommandBufferBeginInfo cmdBufInfo {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            VkSubmitInfo computeSubmitInfo {};
            computeSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            computeSubmitInfo.pWaitDstStageMask  = 0;
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers    = &m_compute_command_buffer;

            // particle compute pass
            if (VK_SUCCESS != vkBeginCommandBuffer(m_compute_command_buffer, &cmdBufInfo))
            {
                throw std::runtime_error("begin command buffer");
            }

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Particle Kickoff", {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_compute_command_buffer, &label_info);
            }

            vkCmdBindPipeline(m_compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_kickoff_pipeline);
            VkDescriptorSet descriptorsets[2] = {m_descriptor_infos[i * 3].descriptor_set,
                                                 m_descriptor_infos[i * 3 + 1].descriptor_set};
            vkCmdBindDescriptorSets(m_compute_command_buffer,
                                    VK_PIPELINE_BIND_POINT_COMPUTE,
                                    m_render_pipelines[0].layout,
                                    0,
                                    2,
                                    descriptorsets,
                                    0,
                                    0);
            vkCmdDispatch(m_compute_command_buffer, 1, 1, 1);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_compute_command_buffer);
            }

            VkBufferMemoryBarrier bufferBarrier {};
            bufferBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_counter_device_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_indirect_dispatch_argument_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Particle Emit", {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_compute_command_buffer, &label_info);
            }

            vkCmdBindPipeline(m_compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_emit_pipeline);
            vkCmdDispatchIndirect(m_compute_command_buffer,
                                  m_emitter_buffer_batches[i].m_indirect_dispatch_argument_buffer,
                                  s_argument_offset_emit);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_compute_command_buffer);
            }

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_position_device_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_position_render_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_counter_device_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_alive_list_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_dead_list_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_alive_list_next_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Particle Simulate", {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_compute_command_buffer, &label_info);
            }

            vkCmdBindPipeline(m_compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_simulate_pipeline);
            vkCmdDispatchIndirect(m_compute_command_buffer,
                                  m_emitter_buffer_batches[i].m_indirect_dispatch_argument_buffer,
                                  s_argument_offset_simulate);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_compute_command_buffer);
            }

            if (VK_SUCCESS != vkEndCommandBuffer(m_compute_command_buffer))
            {
                throw std::runtime_error("end command buffer");
            }
            vkResetFences(m_vulkan_rhi->m_device, 1, &m_fence);
            computeSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            computeSubmitInfo.pWaitDstStageMask  = 0;
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers    = &m_compute_command_buffer;
            if (VK_SUCCESS != vkQueueSubmit(m_vulkan_rhi->m_compute_queue, 1, &computeSubmitInfo, m_fence))
            {
                throw std::runtime_error("compute queue submit");
            }

            if (VK_SUCCESS != vkWaitForFences(m_vulkan_rhi->m_device, 1, &m_fence, VK_TRUE, UINT64_MAX))
            {
                throw std::runtime_error("wait for fence");
            }

            if (VK_SUCCESS != vkBeginCommandBuffer(m_compute_command_buffer, &cmdBufInfo))
            {
                throw std::runtime_error("begin command buffer");
            }

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                                   NULL,
                                                   "Copy Particle Counter Buffer",
                                                   {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_compute_command_buffer, &label_info);
            }

            // Barrier to ensure that shader writes are finished before buffer is read back from GPU
            bufferBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_counter_device_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);
            // Read back to host visible buffer
            VkBufferCopy copyRegion = {};
            copyRegion.size         = sizeof(ParticleCounter);
            vkCmdCopyBuffer(m_compute_command_buffer,
                            m_emitter_buffer_batches[i].m_counter_device_buffer,
                            m_emitter_buffer_batches[i].m_counter_host_buffer,
                            1,
                            &copyRegion);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_compute_command_buffer);
            }

            // Barrier to ensure that buffer copy is finished before host reading from it
            bufferBarrier.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
            bufferBarrier.dstAccessMask       = VK_ACCESS_HOST_READ_BIT;
            bufferBarrier.buffer              = m_emitter_buffer_batches[i].m_counter_host_buffer;
            bufferBarrier.size                = VK_WHOLE_SIZE;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(m_compute_command_buffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_HOST_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &bufferBarrier,
                                 0,
                                 nullptr);
            if (VK_SUCCESS != vkEndCommandBuffer(m_compute_command_buffer))
            {
                throw std::runtime_error("end command buffer");
            }

            // Submit compute work
            vkResetFences(m_vulkan_rhi->m_device, 1, &m_fence);
            computeSubmitInfo                        = {};
            const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            computeSubmitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            computeSubmitInfo.pWaitDstStageMask      = &waitStageMask;
            computeSubmitInfo.commandBufferCount     = 1;
            computeSubmitInfo.pCommandBuffers        = &m_compute_command_buffer;
            if (VK_SUCCESS != vkQueueSubmit(m_vulkan_rhi->m_compute_queue, 1, &computeSubmitInfo, m_fence))
            {
                throw std::runtime_error("compute queue submit");
            }

            if (VK_SUCCESS != vkWaitForFences(m_vulkan_rhi->m_device, 1, &m_fence, VK_TRUE, UINT64_MAX))
            {
                throw std::runtime_error("wait for fence");
            }

            vkQueueWaitIdle(m_vulkan_rhi->m_compute_queue);

            // Make device writes visible to the host
            void* mapped;
            vkMapMemory(m_vulkan_rhi->m_device,
                        m_emitter_buffer_batches[i].m_counter_host_memory,
                        0,
                        VK_WHOLE_SIZE,
                        0,
                        &mapped);
            VkMappedMemoryRange mappedRange {};
            mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = m_emitter_buffer_batches[i].m_counter_host_memory;
            mappedRange.offset = 0;
            mappedRange.size   = VK_WHOLE_SIZE;
            vkInvalidateMappedMemoryRanges(m_vulkan_rhi->m_device, 1, &mappedRange);

            // Copy to output
            ParticleCounter counterNext {};
            memcpy(&counterNext, mapped, sizeof(ParticleCounter));
            vkUnmapMemory(m_vulkan_rhi->m_device, m_emitter_buffer_batches[i].m_counter_host_memory);
            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_compute_command_buffer);
            }
            if constexpr (s_verbose_particle_alive_info)
                LOG_INFO("{} {} {} {}",
                         counterNext.dead_count,
                         counterNext.alive_count,
                         counterNext.alive_count_after_sim,
                         counterNext.emit_count);
            m_emitter_buffer_batches[i].m_num_particle = counterNext.alive_count_after_sim;
        }
        m_emitter_tick_indices.clear();
        m_emitter_transform_indices.clear();
    }

    void ParticlePass::prepareUniformBuffer()
    {
        VkDeviceMemory d_mem;
        VulkanUtil::createBuffer(m_vulkan_rhi->m_physical_device,
                                 m_vulkan_rhi->m_device,
                                 sizeof(m_particle_collision_perframe_storage_buffer_object),
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 m_scene_uniform_buffer,
                                 d_mem);
        if (VK_SUCCESS !=
            vkMapMemory(m_vulkan_rhi->m_device, d_mem, 0, VK_WHOLE_SIZE, 0, &m_scene_uniform_buffer_mapped))
        {
            throw std::runtime_error("map billboard uniform buffer");
        }
        VkDeviceMemory d_uniformdmemory;
        VulkanUtil::createBufferAndInitialize(m_vulkan_rhi->m_device,
                                              m_vulkan_rhi->m_physical_device,
                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                              &m_compute_uniform_buffer,
                                              &d_uniformdmemory,
                                              sizeof(m_ubo));
        if (VK_SUCCESS !=
            vkMapMemory(
                m_vulkan_rhi->m_device, d_uniformdmemory, 0, VK_WHOLE_SIZE, 0, &m_particle_compute_buffer_mapped))
        {
            throw std::runtime_error("map buffer");
        }

        const GlobalParticleRes& global_res = m_particle_manager->getGlobalParticleRes();

        m_ubo.emit_gap  = global_res.m_emit_gap;
        m_ubo.time_step = global_res.m_time_step;
        m_ubo.max_life  = global_res.m_max_life;
        m_ubo.gravity   = global_res.m_gravity;
        std::random_device r;
        std::seed_seq      seed {r()};
        m_random_engine.seed(seed);
        float rnd0        = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
        float rnd1        = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
        float rnd2        = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
        m_ubo.pack        = Vector4 {rnd0, static_cast<float>(m_vulkan_rhi->m_current_frame_index), rnd1, rnd2};
        m_ubo.xemit_count = 100000;

        m_viewport_params = m_vulkan_rhi->m_viewport;
        m_ubo.viewport.x  = m_viewport_params.x;
        m_ubo.viewport.y  = m_viewport_params.y;
        m_ubo.viewport.z  = m_viewport_params.width;
        m_ubo.viewport.w  = m_viewport_params.height;
        m_ubo.extent.x    = m_vulkan_rhi->m_scissor.extent.width;
        m_ubo.extent.y    = m_vulkan_rhi->m_scissor.extent.height;

        memcpy(m_particle_compute_buffer_mapped, &m_ubo, sizeof(m_ubo));

        {
            VkDeviceMemory d_mem;
            VulkanUtil::createBuffer(m_vulkan_rhi->m_physical_device,
                                     m_vulkan_rhi->m_device,
                                     sizeof(m_particlebillboard_perframe_storage_buffer_object),
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     m_particle_billboard_uniform_buffer,
                                     d_mem);
            if (VK_SUCCESS !=
                vkMapMemory(
                    m_vulkan_rhi->m_device, d_mem, 0, VK_WHOLE_SIZE, 0, &m_particle_billboard_uniform_buffer_mapped))
            {
                throw std::runtime_error("map billboard uniform buffer");
            }
        }
    }

    void ParticlePass::updateEmitterTransform()
    {
        for (ParticleEmitterTransformDesc& transform_desc : m_emitter_transform_indices)
        {
            int index                                                 = transform_desc.m_id;
            m_emitter_buffer_batches[index].m_emitter_desc.m_position = transform_desc.m_position;
            m_emitter_buffer_batches[index].m_emitter_desc.m_rotation = transform_desc.m_rotation;

            memcpy(m_emitter_buffer_batches[index].m_emitter_desc_mapped,
                   &m_emitter_buffer_batches[index].m_emitter_desc,
                   sizeof(ParticleEmitterDesc));
        }
    }

    void ParticlePass::updateUniformBuffer()
    {
        std::random_device r;
        std::seed_seq      seed {r()};
        m_random_engine.seed(seed);
        float rnd0 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
        float rnd1 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
        float rnd2 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
        m_ubo.pack = Vector4 {rnd0, rnd1, rnd2, static_cast<float>(m_vulkan_rhi->m_current_frame_index)};

        m_ubo.viewport.x = m_vulkan_rhi->m_viewport.x;
        m_ubo.viewport.y = m_vulkan_rhi->m_viewport.y;
        m_ubo.viewport.z = m_vulkan_rhi->m_viewport.width;
        m_ubo.viewport.w = m_vulkan_rhi->m_viewport.height;
        m_ubo.extent.x   = m_vulkan_rhi->m_scissor.extent.width;
        m_ubo.extent.y   = m_vulkan_rhi->m_scissor.extent.height;

        m_ubo.extent.z = g_runtime_global_context.m_render_system->getRenderCamera()->m_znear;
        m_ubo.extent.w = g_runtime_global_context.m_render_system->getRenderCamera()->m_zfar;
        memcpy(m_particle_compute_buffer_mapped, &m_ubo, sizeof(m_ubo));
    }

    void ParticlePass::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        const RenderResource* vulkan_resource = static_cast<const RenderResource*>(render_resource.get());
        if (vulkan_resource)
        {
            m_particle_collision_perframe_storage_buffer_object =
                vulkan_resource->m_particle_collision_perframe_storage_buffer_object;
            memcpy(m_scene_uniform_buffer_mapped,
                   &m_particle_collision_perframe_storage_buffer_object,
                   sizeof(ParticleCollisionPerframeStorageBufferObject));

            m_particlebillboard_perframe_storage_buffer_object =
                vulkan_resource->m_particlebillboard_perframe_storage_buffer_object;
            memcpy(m_particle_billboard_uniform_buffer_mapped,
                   &m_particlebillboard_perframe_storage_buffer_object,
                   sizeof(m_particlebillboard_perframe_storage_buffer_object));

            m_viewport_params = m_vulkan_rhi->m_viewport;
            updateUniformBuffer();
            updateEmitterTransform();
        }
    }

    void ParticlePass::setDepthAndNormalImage(VkImage depth_image, VkImage normal_image)
    {
        m_src_depth_image  = depth_image;
        m_src_normal_image = normal_image;
    }

    void ParticlePass::setRenderCommandBufferHandle(VkCommandBuffer command_buffer)
    {
        m_render_command_buffer = command_buffer;
    }

    void ParticlePass::setRenderPassHandle(VkRenderPass render_pass) { m_render_pass = render_pass; }

    void ParticlePass::setTickIndices(const std::vector<ParticleEmitterID>& tick_indices)
    {
        m_emitter_tick_indices = tick_indices;
    }

    void ParticlePass::setTransformIndices(const std::vector<ParticleEmitterTransformDesc>& transform_indices)
    {
        m_emitter_transform_indices = transform_indices;
    }
} // namespace Piccolo