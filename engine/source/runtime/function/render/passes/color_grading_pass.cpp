#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"
#include "runtime/function/render/rhi/vulkan/vulkan_util.h"

#include "runtime/function/render/passes/color_grading_pass.h"

#include <color_grading_frag.h>
#include <post_process_vert.h>

#include <stdexcept>

namespace Piccolo
{
    void ColorGradingPass::initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::initialize(nullptr);

        const ColorGradingPassInitInfo* _init_info = static_cast<const ColorGradingPassInitInfo*>(init_info);
        m_framebuffer.render_pass                  = _init_info->render_pass;

        setupDescriptorSetLayout();
        setupPipelines();
        setupDescriptorSet();
        updateAfterFramebufferRecreate(_init_info->input_attachment);
    }

    void ColorGradingPass::setupDescriptorSetLayout()
    {
        m_descriptor_infos.resize(1);

        VkDescriptorSetLayoutBinding post_process_global_layout_bindings[2] = {};

        VkDescriptorSetLayoutBinding& post_process_global_layout_input_attachment_binding =
            post_process_global_layout_bindings[0];
        post_process_global_layout_input_attachment_binding.binding         = 0;
        post_process_global_layout_input_attachment_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        post_process_global_layout_input_attachment_binding.descriptorCount = 1;
        post_process_global_layout_input_attachment_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding& post_process_global_layout_LUT_binding = post_process_global_layout_bindings[1];
        post_process_global_layout_LUT_binding.binding                       = 1;
        post_process_global_layout_LUT_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        post_process_global_layout_LUT_binding.descriptorCount = 1;
        post_process_global_layout_LUT_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo post_process_global_layout_create_info;
        post_process_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        post_process_global_layout_create_info.pNext = NULL;
        post_process_global_layout_create_info.flags = 0;
        post_process_global_layout_create_info.bindingCount =
            sizeof(post_process_global_layout_bindings) / sizeof(post_process_global_layout_bindings[0]);
        post_process_global_layout_create_info.pBindings = post_process_global_layout_bindings;

        if (VK_SUCCESS !=
            vkCreateDescriptorSetLayout(
                m_vulkan_rhi->m_device, &post_process_global_layout_create_info, NULL, &m_descriptor_infos[0].layout))
        {
            throw std::runtime_error("create post process global layout");
        }
    }

    void ColorGradingPass::setupPipelines()
    {
        m_render_pipelines.resize(1);

        VkDescriptorSetLayout      descriptorset_layouts[1] = {m_descriptor_infos[0].layout};
        VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
        pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

        if (vkCreatePipelineLayout(
                m_vulkan_rhi->m_device, &pipeline_layout_create_info, nullptr, &m_render_pipelines[0].layout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create post process pipeline layout");
        }

        VkShaderModule vert_shader_module = VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, POST_PROCESS_VERT);
        VkShaderModule frag_shader_module = VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, COLOR_GRADING_FRAG);

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

        VkPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                           frag_pipeline_shader_stage_create_info};

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
        vertex_input_state_create_info.pVertexBindingDescriptions      = NULL;
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
        vertex_input_state_create_info.pVertexAttributeDescriptions    = NULL;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
        input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state_create_info {};
        viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports    = &m_vulkan_rhi->m_viewport;
        viewport_state_create_info.scissorCount  = 1;
        viewport_state_create_info.pScissors     = &m_vulkan_rhi->m_scissor;

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
        rasterization_state_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth               = 1.0f;
        rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp          = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
        multisample_state_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
        color_blend_attachment_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable         = VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp        = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
        color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable     = VK_FALSE;
        color_blend_state_create_info.logicOp           = VK_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount   = 1;
        color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info {};
        depth_stencil_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable       = VK_TRUE;
        depth_stencil_create_info.depthWriteEnable      = VK_TRUE;
        depth_stencil_create_info.depthCompareOp        = VK_COMPARE_OP_LESS;
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable     = VK_FALSE;

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
        dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates    = dynamic_states;

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shader_stages;
        pipelineInfo.pVertexInputState   = &vertex_input_state_create_info;
        pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
        pipelineInfo.pViewportState      = &viewport_state_create_info;
        pipelineInfo.pRasterizationState = &rasterization_state_create_info;
        pipelineInfo.pMultisampleState   = &multisample_state_create_info;
        pipelineInfo.pColorBlendState    = &color_blend_state_create_info;
        pipelineInfo.pDepthStencilState  = &depth_stencil_create_info;
        pipelineInfo.layout              = m_render_pipelines[0].layout;
        pipelineInfo.renderPass          = m_framebuffer.render_pass;
        pipelineInfo.subpass             = _main_camera_subpass_color_grading;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState       = &dynamic_state_create_info;

        if (vkCreateGraphicsPipelines(
                m_vulkan_rhi->m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_render_pipelines[0].pipeline) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create post process graphics pipeline");
        }

        vkDestroyShaderModule(m_vulkan_rhi->m_device, vert_shader_module, nullptr);
        vkDestroyShaderModule(m_vulkan_rhi->m_device, frag_shader_module, nullptr);
    }

    void ColorGradingPass::setupDescriptorSet()
    {
        VkDescriptorSetAllocateInfo post_process_global_descriptor_set_alloc_info;
        post_process_global_descriptor_set_alloc_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        post_process_global_descriptor_set_alloc_info.pNext          = NULL;
        post_process_global_descriptor_set_alloc_info.descriptorPool = m_vulkan_rhi->m_descriptor_pool;
        post_process_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        post_process_global_descriptor_set_alloc_info.pSetLayouts        = &m_descriptor_infos[0].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_rhi->m_device,
                                                   &post_process_global_descriptor_set_alloc_info,
                                                   &m_descriptor_infos[0].descriptor_set))
        {
            throw std::runtime_error("allocate post process global descriptor set");
        }
    }

    void ColorGradingPass::updateAfterFramebufferRecreate(VkImageView input_attachment)
    {
        VkDescriptorImageInfo post_process_per_frame_input_attachment_info = {};
        post_process_per_frame_input_attachment_info.sampler =
            VulkanUtil::getOrCreateNearestSampler(m_vulkan_rhi->m_physical_device, m_vulkan_rhi->m_device);
        post_process_per_frame_input_attachment_info.imageView   = input_attachment;
        post_process_per_frame_input_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo color_grading_LUT_image_info = {};
        color_grading_LUT_image_info.sampler =
            VulkanUtil::getOrCreateLinearSampler(m_vulkan_rhi->m_physical_device, m_vulkan_rhi->m_device);
        color_grading_LUT_image_info.imageView =
            m_global_render_resource->_color_grading_resource._color_grading_LUT_texture_image_view;
        color_grading_LUT_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet post_process_descriptor_writes_info[2];

        VkWriteDescriptorSet& post_process_descriptor_input_attachment_write_info =
            post_process_descriptor_writes_info[0];
        post_process_descriptor_input_attachment_write_info.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        post_process_descriptor_input_attachment_write_info.pNext           = NULL;
        post_process_descriptor_input_attachment_write_info.dstSet          = m_descriptor_infos[0].descriptor_set;
        post_process_descriptor_input_attachment_write_info.dstBinding      = 0;
        post_process_descriptor_input_attachment_write_info.dstArrayElement = 0;
        post_process_descriptor_input_attachment_write_info.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        post_process_descriptor_input_attachment_write_info.descriptorCount = 1;
        post_process_descriptor_input_attachment_write_info.pImageInfo = &post_process_per_frame_input_attachment_info;

        VkWriteDescriptorSet& post_process_descriptor_LUT_write_info = post_process_descriptor_writes_info[1];
        post_process_descriptor_LUT_write_info.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        post_process_descriptor_LUT_write_info.pNext                 = NULL;
        post_process_descriptor_LUT_write_info.dstSet                = m_descriptor_infos[0].descriptor_set;
        post_process_descriptor_LUT_write_info.dstBinding            = 1;
        post_process_descriptor_LUT_write_info.dstArrayElement       = 0;
        post_process_descriptor_LUT_write_info.descriptorType        = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        post_process_descriptor_LUT_write_info.descriptorCount       = 1;
        post_process_descriptor_LUT_write_info.pImageInfo            = &color_grading_LUT_image_info;

        vkUpdateDescriptorSets(m_vulkan_rhi->m_device,
                               sizeof(post_process_descriptor_writes_info) /
                                   sizeof(post_process_descriptor_writes_info[0]),
                               post_process_descriptor_writes_info,
                               0,
                               NULL);
    }

    void ColorGradingPass::draw()
    {
        if (m_vulkan_rhi->isDebugLabelEnabled())
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Color Grading", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer, &label_info);
        }

        m_vulkan_rhi->m_vk_cmd_bind_pipeline(
            m_vulkan_rhi->m_current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_render_pipelines[0].pipeline);
        m_vulkan_rhi->m_vk_cmd_set_viewport(m_vulkan_rhi->m_current_command_buffer, 0, 1, &m_vulkan_rhi->m_viewport);
        m_vulkan_rhi->m_vk_cmd_set_scissor(m_vulkan_rhi->m_current_command_buffer, 0, 1, &m_vulkan_rhi->m_scissor);
        m_vulkan_rhi->m_vk_cmd_bind_descriptor_sets(m_vulkan_rhi->m_current_command_buffer,
                                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    m_render_pipelines[0].layout,
                                                    0,
                                                    1,
                                                    &m_descriptor_infos[0].descriptor_set,
                                                    0,
                                                    NULL);

        vkCmdDraw(m_vulkan_rhi->m_current_command_buffer, 3, 1, 0, 0);

        if (m_vulkan_rhi->isDebugLabelEnabled())
        {
            m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer);
        }
    }
} // namespace Piccolo
