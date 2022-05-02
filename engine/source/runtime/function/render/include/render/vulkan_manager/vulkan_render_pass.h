#pragma once

#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_global_resource.h"

#include <vector>
#include <vulkan/vulkan.h>

#include <tuple>
using std::tuple;

namespace Pilot
{
    struct PRenderPassHelperInfo
    {
        PVulkanContext*        p_context;
        VkDescriptorPool       descriptor_pool;
        PGlobalRenderResource* p_global_render_resource;
    };

    struct PVisiableNodes
    {
        std::vector<PVulkanMeshNode>*              p_directional_light_visible_mesh_nodes;
        std::vector<PVulkanMeshNode>*              p_point_lights_visible_mesh_nodes;
        std::vector<PVulkanMeshNode>*              p_main_camera_visible_mesh_nodes;
        PVulkanAxisNode*                           p_axis_node;
        std::vector<PVulkanParticleBillboardNode>* p_main_camera_visible_particlebillboard_nodes;
    };

    struct PRenderConfig
    {
        bool _enable_validation_Layers;
        bool _enable_debug_untils_label;
        bool _enable_point_light_shadow;
    };

    struct PRenderCommandInfo
    {
        uint32_t         _current_frame_index;
        uint32_t*        _p_current_frame_index;
        VkCommandBuffer  _current_command_buffer;
        VkCommandBuffer* _p_current_command_buffer;
        VkCommandPool    _current_command_pool;
        VkCommandPool*   _p_command_pools;
        VkFence          _current_fence;
        VkViewport       _viewport;
        VkRect2D         _scissor;
        int              _max_frames_in_flight;
        VkFence*         _is_frame_in_flight_fences;
    };

    class PRenderPassBase
    {
    public:

        struct Framebuffer
        {
            int           width;
            int           height;
            VkFramebuffer framebuffer;
            VkRenderPass  render_pass;
        };

        struct Descriptor
        {
            VkDescriptorSetLayout layout;
            VkDescriptorSet       descriptor_set;
        };

        struct RenderPipeline
        {
            VkPipelineLayout layout;
            VkPipeline       pipeline;
        };

        std::vector<Descriptor>     _descriptor_infos;
        std::vector<RenderPipeline> _render_pipelines;
        Framebuffer                 _framebuffer;

        static void setContext(const PRenderPassHelperInfo& helper_info);

        virtual void initialize();
        virtual void draw();
        virtual void postInitialize();

        virtual VkRenderPass                       getRenderPass();
        virtual std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts();

        static PVisiableNodes     m_visiable_nodes;
        static PRenderConfig      m_render_config;
        static PRenderCommandInfo m_command_info;

    protected:
        void                                         FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo*  fill,
                                                          VkDevice                          _device,
                                                          const std::vector<unsigned char>& vert,
                                                          const std::vector<unsigned char>& frag);
        void                                         FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo*  fill,
                                                          VkDevice                          _device,
                                                          const std::vector<unsigned char>& vert,
                                                          const std::vector<unsigned char>& geom,
                                                          const std::vector<unsigned char>& frag);
        void                                         ModuleGC();
        std::vector<tuple<VkDevice, VkShaderModule>> module_reference;

        static PVulkanContext*        m_p_vulkan_context;
        static VkDescriptorPool       m_descriptor_pool;
        static PGlobalRenderResource* m_p_global_render_resource;
    };
} // namespace Pilot
