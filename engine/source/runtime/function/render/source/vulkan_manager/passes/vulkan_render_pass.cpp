#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_util.h"

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
    std::vector<VkDescriptorSetLayout> PRenderPassBase::getDescriptorSetLayouts()
    {
        std::vector<VkDescriptorSetLayout> layouts;
        for (auto& desc : _descriptor_infos)
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
    void PRenderPassBase::FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo*  fill,
                                                    VkDevice                          _device,
                                                    const std::vector<unsigned char>& vert,
                                                    const std::vector<unsigned char>& frag)
    {

        VkShaderModule vert_shader_module = PVulkanUtil::createShaderModule(_device, vert);
        VkShaderModule frag_shader_module = PVulkanUtil::createShaderModule(_device, frag);
        module_reference.push_back(tuple<VkDevice, VkShaderModule>(_device, vert_shader_module));
        module_reference.push_back(tuple<VkDevice, VkShaderModule>(_device, frag_shader_module));

        VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
        vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;
        vert_pipeline_shader_stage_create_info.pName  = "main";

        VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
        frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName  = "main";

        fill[0] = vert_pipeline_shader_stage_create_info;
        fill[1] = frag_pipeline_shader_stage_create_info;
    }
    void PRenderPassBase::FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo*  fill,
                                                    VkDevice                          _device,
                                                    const std::vector<unsigned char>& vert,
                                                    const std::vector<unsigned char>& geom,
                                                    const std::vector<unsigned char>& frag)
    {

        VkShaderModule vert_shader_module = PVulkanUtil::createShaderModule(_device, vert);
        VkShaderModule geom_shader_module = PVulkanUtil::createShaderModule(_device, geom);
        VkShaderModule frag_shader_module = PVulkanUtil::createShaderModule(_device, frag);
        module_reference.push_back(tuple<VkDevice, VkShaderModule>(_device, vert_shader_module));
        module_reference.push_back(tuple<VkDevice, VkShaderModule>(_device, geom_shader_module));
        module_reference.push_back(tuple<VkDevice, VkShaderModule>(_device, frag_shader_module));

        VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
        vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;
        vert_pipeline_shader_stage_create_info.pName  = "main";

        VkPipelineShaderStageCreateInfo geom_pipeline_shader_stage_create_info {};
        geom_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geom_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
        geom_pipeline_shader_stage_create_info.module = geom_shader_module;
        geom_pipeline_shader_stage_create_info.pName  = "main";

        VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
        frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName  = "main";

        fill[0] = vert_pipeline_shader_stage_create_info;
        fill[1] = geom_pipeline_shader_stage_create_info;
        fill[2] = frag_pipeline_shader_stage_create_info;
    }
    void PRenderPassBase::ModuleGC()
    {
        for (auto& it : module_reference)
        {
            vkDestroyShaderModule(std::get<0>(it), std::get<1>(it), nullptr);
        }
        module_reference.clear();
    }
} // namespace Pilot
