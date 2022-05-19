#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

// vulkan graphics render pass (m_vulkan_context._swapchain --> m_renderpass)
bool Pilot::PVulkanManager::initializeRenderPass()
{
    PRenderPassHelperInfo helper_info {};
    helper_info.p_context                = &m_vulkan_context;
    helper_info.descriptor_pool          = m_descriptor_pool;
    helper_info.p_global_render_resource = &m_global_render_resource;

    PMainCameraPass::setContext(helper_info);

    // initialize before lighting pass
    m_point_light_shadow_pass.initialize();
    m_directional_light_shadow_pass.initialize();

    PLightPassHelperInfo light_pass_helper_info {};
    light_pass_helper_info.point_light_shadow_color_image_view = m_point_light_shadow_pass.getFramebufferImageViews()[0];
    light_pass_helper_info.directional_light_shadow_color_image_view =
        m_directional_light_shadow_pass._framebuffer.attachments[0].view;
    m_main_camera_pass.setHelperInfo(light_pass_helper_info);
    m_main_camera_pass.initialize();

    auto descriptor_layouts = m_main_camera_pass.getDescriptorSetLayouts();

    m_point_light_shadow_pass._per_mesh_layout       = descriptor_layouts[PMainCameraPass::LayoutType::_per_mesh];
    m_directional_light_shadow_pass._per_mesh_layout = descriptor_layouts[PMainCameraPass::LayoutType::_per_mesh];

    m_point_light_shadow_pass.postInitialize();
    m_directional_light_shadow_pass.postInitialize();

    m_tone_mapping_pass.initialize(m_main_camera_pass.getRenderPass(), m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);

    m_color_grading_pass.initialize(m_main_camera_pass.getRenderPass(), m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);

	m_pixel_pass.initialize(m_main_camera_pass.getRenderPass(), m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);

    m_ui_pass.initialize(m_main_camera_pass.getRenderPass());

	//m_combine_ui_pass.initialize(m_main_camera_pass.getRenderPass(), m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd], m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
    m_combine_ui_pass.initialize(m_main_camera_pass.getRenderPass(), m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even], m_main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);

    m_mouse_pick_pass._per_mesh_layout = descriptor_layouts[PMainCameraPass::LayoutType::_per_mesh];
    m_mouse_pick_pass.initialize();

    return true;
}


void Pilot::PVulkanManager::initializeUI(void *surface_ui)
{
    m_ui_pass.setSurfaceUI(surface_ui);
}