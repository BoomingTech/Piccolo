#pragma once

#include "runtime/function/render/render_common.h"
#include "runtime/function/render/render_pass_base.h"
#include "runtime/function/render/render_resource.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace Pilot
{
    class VulkanRHI;

    enum
    {
        _main_camera_pass_gbuffer_a                     = 0,
        _main_camera_pass_gbuffer_b                     = 1,
        _main_camera_pass_gbuffer_c                     = 2,
        _main_camera_pass_backup_buffer_odd             = 3,
        _main_camera_pass_backup_buffer_even            = 4,
        _main_camera_pass_post_process_buffer_odd       = 5,
        _main_camera_pass_post_process_buffer_even      = 6,
        _main_camera_pass_depth                         = 7,
        _main_camera_pass_swap_chain_image              = 8,
        _main_camera_pass_custom_attachment_count       = 5,
        _main_camera_pass_post_process_attachment_count = 2,
        _main_camera_pass_attachment_count              = 9,
    };

    enum
    {
        _main_camera_subpass_basepass = 0,
        _main_camera_subpass_deferred_lighting,
        _main_camera_subpass_forward_lighting,
        _main_camera_subpass_tone_mapping,
        _main_camera_subpass_color_grading,
        _main_camera_subpass_fxaa,
        _main_camera_subpass_ui,
        _main_camera_subpass_combine_ui,
        _main_camera_subpass_count
    };

    struct PVisiableNodes
    {
        std::vector<PVulkanMeshNode>*              p_directional_light_visible_mesh_nodes        = nullptr;
        std::vector<PVulkanMeshNode>*              p_point_lights_visible_mesh_nodes             = nullptr;
        std::vector<PVulkanMeshNode>*              p_main_camera_visible_mesh_nodes              = nullptr;
        PVulkanAxisNode*                           p_axis_node                                   = nullptr;
        std::vector<PVulkanParticleBillboardNode>* p_main_camera_visible_particlebillboard_nodes = nullptr;
    };

    class RenderPass : public RenderPassBase
    {
    public:
        struct FrameBufferAttachment
        {
            VkImage        image;
            VkDeviceMemory mem;
            VkImageView    view;
            VkFormat       format;
        };

        struct Framebuffer
        {
            int           width;
            int           height;
            VkFramebuffer framebuffer;
            VkRenderPass  render_pass;

            std::vector<FrameBufferAttachment> attachments;
        };

        struct Descriptor
        {
            VkDescriptorSetLayout layout;
            VkDescriptorSet       descriptor_set;
        };

        struct RenderPipelineBase
        {
            VkPipelineLayout layout;
            VkPipeline       pipeline;
        };

        std::shared_ptr<VulkanRHI> _rhi {nullptr};
        PGlobalRenderResource*     _p_global_render_resource {nullptr};

        std::vector<Descriptor>         _descriptor_infos;
        std::vector<RenderPipelineBase> _render_pipelines;
        Framebuffer                     _framebuffer;

        virtual void initialize(const RenderPassInitInfo* init_info) override;
        virtual void draw();
        virtual void postInitialize();

        virtual VkRenderPass                       getRenderPass();
        virtual std::vector<VkImageView>           getFramebufferImageViews();
        virtual std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts();

        static PVisiableNodes m_visiable_nodes;

    private:
    };
} // namespace Pilot
