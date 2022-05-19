#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

int Pilot::PVulkanManager::initialize(GLFWwindow* window, class Scene& scene, PilotRenderer* pilot_renderer)
{
    m_vulkan_context.initialize(window);

    m_global_render_resource.initialize(m_vulkan_context, m_max_frames_in_flight);

    PRenderPassBase::m_render_config._enable_debug_untils_label = m_enable_debug_untils_label;
    PRenderPassBase::m_render_config._enable_point_light_shadow = m_enable_point_light_shadow;
    PRenderPassBase::m_render_config._enable_validation_Layers  = m_enable_validation_Layers;

    PRenderPassBase::m_command_info._viewport = m_viewport;
    PRenderPassBase::m_command_info._scissor  = m_scissor;

    // global textures for IBL
    PIBLResourceData ibl_resource_data = m_global_render_resource.getIBLTextureData(&scene, pilot_renderer);
    updateGlobalTexturesForIBL(ibl_resource_data);

    // global textures for color grading
    PColorGradingResourceData color_grading_resource_data = m_global_render_resource.getColorGradingTextureData(&scene, pilot_renderer);
    updateGlobalTexturesForColorGrading(color_grading_resource_data);

    if (initializeCommandPool() && initializeDescriptorPool() && createSyncPrimitives() && initializeCommandBuffers() &&
        initializeRenderPass())
        return 1;
    else
        return 0;
}
