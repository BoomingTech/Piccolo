#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"

#include "runtime/function/render/passes/ui_pass.h"

#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/ui/window_ui.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <cassert>

namespace Pilot
{
    void UIPass::initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::initialize(nullptr);

        _framebuffer.render_pass = static_cast<const UIPassInitInfo*>(init_info)->render_pass;
    }

    void UIPass::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_window_ui = window_ui;

        ImGui_ImplGlfw_InitForVulkan(_rhi->_window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = _rhi->_instance;
        init_info.PhysicalDevice            = _rhi->_physical_device;
        init_info.Device                    = _rhi->_device;
        init_info.QueueFamily               = _rhi->_queue_indices.graphics_family.value();
        init_info.Queue                     = _rhi->_graphics_queue;
        init_info.DescriptorPool            = _rhi->_descriptor_pool;
        init_info.Subpass                   = _main_camera_subpass_ui;

        // may be different from the real swapchain image count
        // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
        init_info.MinImageCount = 3;
        init_info.ImageCount    = 3;
        ImGui_ImplVulkan_Init(&init_info, _framebuffer.render_pass);

        uploadFonts();
    }

    void UIPass::uploadFonts()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool                 = _rhi->_command_pool;
        allocInfo.commandBufferCount          = 1;

        VkCommandBuffer commandBuffer = {};
        if (VK_SUCCESS != vkAllocateCommandBuffers(_rhi->_device, &allocInfo, &commandBuffer))
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo))
        {
            throw std::runtime_error("Could not create one-time command buffer!");
        }

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer))
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        VkSubmitInfo submitInfo {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(_rhi->_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(_rhi->_graphics_queue);

        vkFreeCommandBuffers(_rhi->_device, _rhi->_command_pool, 1, &commandBuffer);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void UIPass::draw()
    {
        if (m_window_ui)
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            m_window_ui->preRender();

            if (_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr, "ImGUI", {1.0f, 1.0f, 1.0f, 1.0f}};
                _rhi->_vkCmdBeginDebugUtilsLabelEXT(_rhi->_current_command_buffer, &label_info);
            }

            ImGui::Render();

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _rhi->_current_command_buffer);

            if (_rhi->isDebugLabelEnabled())
            {
                _rhi->_vkCmdEndDebugUtilsLabelEXT(_rhi->_current_command_buffer);
            }
        }
    }
} // namespace Pilot
