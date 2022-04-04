#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"

Pilot::PVulkanContext*        Pilot::PRenderPassBase::m_p_vulkan_context         = nullptr;
VkDescriptorPool              Pilot::PRenderPassBase::m_descriptor_pool          = VK_NULL_HANDLE;
Pilot::PGlobalRenderResource* Pilot::PRenderPassBase::m_p_global_render_resource = nullptr;

Pilot::PRenderConfig      Pilot::PRenderPassBase::m_render_config;
Pilot::PVisiableNodes     Pilot::PRenderPassBase::m_visiable_nodes;
Pilot::PRenderCommandInfo Pilot::PRenderPassBase::m_command_info;

namespace Pilot
{
    void PRenderPassBase::setContext(const PRenderPassHelperInfo& helper_info)
    {
        m_p_vulkan_context         = helper_info.p_context;
        m_descriptor_pool          = helper_info.descriptor_pool;
        m_p_global_render_resource = helper_info.p_global_render_resource;
    }
    void PRenderPassBase::initialize()
    {
        assert(m_p_vulkan_context);
        assert(m_descriptor_pool != VK_NULL_HANDLE);
        assert(m_p_global_render_resource);
    }
    void                     PRenderPassBase::draw() {}
    void                     PRenderPassBase::postInitialize() {}
    VkRenderPass             PRenderPassBase::getRenderPass() { return _framebuffer.render_pass; }
    std::vector<VkImageView> PRenderPassBase::getFramebufferImageViews()
    {
        std::vector<VkImageView> image_views;
        for (auto& attach : _framebuffer.attachments)
        {
            image_views.push_back(attach.view);
        }
        return image_views;
    }
    std::vector<VkDescriptorSetLayout> PRenderPassBase::getDescriptorSetLayouts()
    {
        std::vector<VkDescriptorSetLayout> layouts;
        for (auto& desc : _descriptor_infos)
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
} // namespace Pilot
