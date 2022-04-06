#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "runtime/function/render/include/render/surface.h"
#include "runtime/resource/config_manager/config_manager.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <stb_image.h>

using namespace Pilot;

int SurfaceUI::initialize(SurfaceRHI* rhi, PilotRenderer* prenderer, std::shared_ptr<SurfaceIO> pio)
{
    m_io  = pio;
    m_rhi = rhi;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO&    io    = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingAlwaysTabBar         = true;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    io.Fonts->AddFontFromFileTTF(
        ConfigManager::getInstance().getEditorFontPath().generic_string().data(), 16, nullptr, nullptr);
    io.Fonts->Build();
    style.WindowPadding   = ImVec2(1.0, 0);
    style.FramePadding    = ImVec2(14.0, 2.0f);
    style.ChildBorderSize = 0.0f;
    style.FrameRounding   = 5.0f;
    style.FrameBorderSize = 1.5f;

    // Setup Pilot ImGui style
    setDefaultStyle();

    // implement init
    ImGui_ImplGlfw_InitForVulkan(rhi->m_vulkan_manager->m_vulkan_context._window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = rhi->m_vulkan_manager->m_vulkan_context._instance;
    init_info.PhysicalDevice            = rhi->m_vulkan_manager->m_vulkan_context._physical_device;
    init_info.Device                    = rhi->m_vulkan_manager->m_vulkan_context._device;
    init_info.QueueFamily               = rhi->m_vulkan_manager->m_vulkan_context._queue_indices.graphicsFamily.value();
    init_info.Queue                     = rhi->m_vulkan_manager->m_vulkan_context._graphics_queue;
    init_info.DescriptorPool            = rhi->m_vulkan_manager->m_descriptor_pool;
    init_info.Subpass                   = 1;
    // may be diffirent from the real swapchain image count // see: ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
    init_info.MinImageCount = rhi->m_vulkan_manager->m_max_frames_in_flight;
    init_info.ImageCount    = rhi->m_vulkan_manager->m_max_frames_in_flight;
    ImGui_ImplVulkan_Init(&init_info, rhi->m_vulkan_manager->getLightingPass());

    // fonts upload
    fontsUpload(rhi);

    // register input
    registerInput();

    // initialize window icon
    GLFWimage   window_icon[2];
    std::string big_icon_path_string   = ConfigManager::getInstance().getEditorBigIconPath().generic_string();
    std::string small_icon_path_string = ConfigManager::getInstance().getEditorSmallIconPath().generic_string();
    window_icon[0].pixels = stbi_load(big_icon_path_string.data(), &window_icon[0].width, &window_icon[0].height, 0, 4);
    window_icon[1].pixels =
        stbi_load(small_icon_path_string.data(), &window_icon[1].width, &window_icon[1].height, 0, 4);
    glfwSetWindowIcon(m_io->m_window, 2, window_icon);
    stbi_image_free(window_icon[0].pixels);
    stbi_image_free(window_icon[1].pixels);

    return 0;
}

void SurfaceUI::fontsUpload(SurfaceRHI* rhi)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool                 = rhi->m_vulkan_manager->m_vulkan_context._command_pool;
    allocInfo.commandBufferCount          = 1;

    VkCommandBuffer commandBuffer = {};
    if (vkAllocateCommandBuffers(rhi->m_vulkan_manager->m_vulkan_context._device, &allocInfo, &commandBuffer) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create one-time command buffer!");
    }

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(rhi->m_vulkan_manager->m_vulkan_context._graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(rhi->m_vulkan_manager->m_vulkan_context._graphics_queue);

    vkFreeCommandBuffers(rhi->m_vulkan_manager->m_vulkan_context._device,
                         rhi->m_vulkan_manager->m_vulkan_context._command_pool,
                         1,
                         &commandBuffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void SurfaceUI::tick_pre(UIState* uistate)
{
    if (m_rhi->m_vulkan_manager->m_frame_swapchain_image_acquired[m_rhi->m_vulkan_manager->m_current_frame_index])
    {
        m_tmp_uistate = uistate;
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        onTick(uistate);
    }
}

void SurfaceUI::tick_post(UIState* uistate) { draw_frame(); }

void SurfaceUI::draw_frame()
{
    if (m_rhi->m_vulkan_manager->m_frame_swapchain_image_acquired[m_rhi->m_vulkan_manager->m_current_frame_index])
    {
        VkCommandBuffer current_command_buffer = m_rhi->m_vulkan_manager->getCurrentCommandBuffer();

        if (m_rhi->m_vulkan_manager->m_enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "ImGUI", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_rhi->m_vulkan_manager->m_vulkan_context._vkCmdBeginDebugUtilsLabelEXT(current_command_buffer,
                                                                                    &label_info);
        }

        ImGui::Render();

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), current_command_buffer);

        if (m_rhi->m_vulkan_manager->m_enable_debug_untils_label)
        {
            m_rhi->m_vulkan_manager->m_vulkan_context._vkCmdEndDebugUtilsLabelEXT(current_command_buffer);
        }
    }
}

int SurfaceUI::clear() { return 0; }

void SurfaceUI::setDefaultStyle()
{
    ImGuiStyle* style  = &ImGui::GetStyle();
    ImVec4*     colors = style->Colors;

    colors[ImGuiCol_Text]                  = ImVec4(0.4745f, 0.4745f, 0.4745f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.0078f, 0.0078f, 0.0078f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.047f, 0.047f, 0.047f, 0.5411f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.196f, 0.196f, 0.196f, 0.40f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.294f, 0.294f, 0.294f, 0.67f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(93.0f / 255.0f, 10.0f / 255.0f, 66.0f / 255.0f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = colors[ImGuiCol_CheckMark];
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.3647f, 0.0392f, 0.2588f, 0.50f);
    colors[ImGuiCol_Button]                = ImVec4(0.0117f, 0.0117f, 0.0117f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.0235f, 0.0235f, 0.0235f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.0353f, 0.0196f, 0.0235f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.1137f, 0.0235f, 0.0745f, 0.588f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                   = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 150.0f / 255.0f);
    colors[ImGuiCol_TabActive]             = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 1.0f);
    colors[ImGuiCol_TabUnfocused]          = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 200.0f / 255.0f);
    colors[ImGuiCol_DockingPreview]        = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 0.7f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(2.0f / 255.0f, 2.0f / 255.0f, 2.0f / 255.0f, 1.0f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
