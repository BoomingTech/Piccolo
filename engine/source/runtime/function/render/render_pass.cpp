#include "runtime/function/render/render_pass.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"

Pilot::VisiableNodes Pilot::RenderPass::m_visiable_nodes;

namespace Pilot
{
    void RenderPass::initialize(const RenderPassInitInfo* init_info)
    {
        m_vulkan_rhi = std::static_pointer_cast<VulkanRHI>(m_rhi);
        m_global_render_resource =
            &(std::static_pointer_cast<RenderResource>(m_render_resource)->m_global_render_resource);
    }
    void RenderPass::draw() {}

    void RenderPass::postInitialize() {}

    VkRenderPass RenderPass::getRenderPass() const { return m_framebuffer.render_pass; }

    std::vector<VkImageView> RenderPass::getFramebufferImageViews() const
    {
        std::vector<VkImageView> image_views;
        for (auto& attach : m_framebuffer.attachments)
        {
            image_views.push_back(attach.view);
        }
        return image_views;
    }

    std::vector<VkDescriptorSetLayout> RenderPass::getDescriptorSetLayouts() const
    {
        std::vector<VkDescriptorSetLayout> layouts;
        for (auto& desc : m_descriptor_infos)
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
} // namespace Pilot
