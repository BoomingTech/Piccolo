#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

// vulkan graphics render pass (m_vulkan_context._swapchain --> m_renderpass)
bool Pilot::PVulkanManager::initializeRenderPass()
{
    PRenderPassHelperInfo helper_info {};
    helper_info.p_context                = &m_vulkan_context;
    helper_info.descriptor_pool          = m_descriptor_pool;
    helper_info.p_global_render_resource = &m_global_render_resource;

    PLightingPass::setContext(helper_info);

    // initialize before lighting pass
    m_point_light_pass.initialize();
    m_directional_light_pass.initialize();

    PLightPassHelperInfo light_pass_helper_info {};
    light_pass_helper_info.point_light_shadow_color_image_view = m_point_light_pass.getFramebufferImageViews()[0];
    light_pass_helper_info.directional_light_shadow_color_image_view =
        m_directional_light_pass._framebuffer.attachments[0].view;
    m_mesh_lighting_pass.setHelperInfo(light_pass_helper_info);
    m_mesh_lighting_pass.initialize();

    auto descriptor_layouts = m_mesh_lighting_pass.getDescriptorSetLayouts();

    m_point_light_pass._per_mesh_layout       = descriptor_layouts[PLightingPass::LayoutType::_per_mesh];
    m_directional_light_pass._per_mesh_layout = descriptor_layouts[PLightingPass::LayoutType::_per_mesh];

    m_point_light_pass.postInitialize();
    m_directional_light_pass.postInitialize();

    m_postprocess_pass._render_pass = m_mesh_lighting_pass.getRenderPass();
    m_postprocess_pass.initialize();

    m_mouse_pick_pass._per_mesh_layout = descriptor_layouts[PLightingPass::LayoutType::_per_mesh];
    m_mouse_pick_pass.initialize();

    return true;
}
