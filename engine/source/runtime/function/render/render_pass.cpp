#include "runtime/function/render/render_pass.h"
#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"

#include "runtime/core/base/macro.h"

Pilot::PVisiableNodes Pilot::RenderPass::m_visiable_nodes;

namespace Pilot
{
    void RenderPass::initialize(const RenderPassInitInfo* init_info)
    {
        _rhi = std::static_pointer_cast<VulkanRHI>(m_rhi);
        _p_global_render_resource =
            &(std::static_pointer_cast<RenderResource>(m_render_resource)->m_global_render_resource);
    }
    void                     RenderPass::draw() {}
    void                     RenderPass::postInitialize() {}
    VkRenderPass             RenderPass::getRenderPass() { return _framebuffer.render_pass; }
    std::vector<VkImageView> RenderPass::getFramebufferImageViews()
    {
        std::vector<VkImageView> image_views;
        for (auto& attach : _framebuffer.attachments)
        {
            image_views.push_back(attach.view);
        }
        return image_views;
    }
    std::vector<VkDescriptorSetLayout> RenderPass::getDescriptorSetLayouts()
    {
        std::vector<VkDescriptorSetLayout> layouts;
        for (auto& desc : _descriptor_infos)
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
} // namespace Pilot
