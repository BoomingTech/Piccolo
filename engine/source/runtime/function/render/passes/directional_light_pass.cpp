#include "runtime/function/render/render_helper.h"
#include "runtime/function/render/render_mesh.h"
#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"
#include "runtime/function/render/rhi/vulkan/vulkan_util.h"

#include "runtime/function/render/passes/directional_light_pass.h"

#include <mesh_directional_light_shadow_frag.h>
#include <mesh_directional_light_shadow_vert.h>

#include <stdexcept>

namespace Piccolo
{
    void DirectionalLightShadowPass::initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::initialize(nullptr);

        setupAttachments();
        setupRenderPass();
        setupFramebuffer();
        setupDescriptorSetLayout();
    }
    void DirectionalLightShadowPass::postInitialize()
    {
        setupPipelines();
        setupDescriptorSet();
    }
    void DirectionalLightShadowPass::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        const RenderResource* vulkan_resource = static_cast<const RenderResource*>(render_resource.get());
        if (vulkan_resource)
        {
            m_mesh_directional_light_shadow_perframe_storage_buffer_object =
                vulkan_resource->m_mesh_directional_light_shadow_perframe_storage_buffer_object;
        }
    }
    void DirectionalLightShadowPass::draw() { drawModel(); }
    void DirectionalLightShadowPass::setupAttachments()
    {
        // color and depth
        m_framebuffer.attachments.resize(2);

        // color
        m_framebuffer.attachments[0].format = VK_FORMAT_R32_SFLOAT;
        VulkanUtil::createImage(m_vulkan_rhi->m_physical_device,
                                m_vulkan_rhi->m_device,
                                s_directional_light_shadow_map_dimension,
                                s_directional_light_shadow_map_dimension,
                                m_framebuffer.attachments[0].format,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_framebuffer.attachments[0].image,
                                m_framebuffer.attachments[0].mem,
                                0,
                                1,
                                1);
        m_framebuffer.attachments[0].view = VulkanUtil::createImageView(m_vulkan_rhi->m_device,
                                                                        m_framebuffer.attachments[0].image,
                                                                        m_framebuffer.attachments[0].format,
                                                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                                                        VK_IMAGE_VIEW_TYPE_2D,
                                                                        1,
                                                                        1);

        // depth
        m_framebuffer.attachments[1].format = m_vulkan_rhi->m_depth_image_format;
        VulkanUtil::createImage(m_vulkan_rhi->m_physical_device,
                                m_vulkan_rhi->m_device,
                                s_directional_light_shadow_map_dimension,
                                s_directional_light_shadow_map_dimension,
                                m_framebuffer.attachments[1].format,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_framebuffer.attachments[1].image,
                                m_framebuffer.attachments[1].mem,
                                0,
                                1,
                                1);
        m_framebuffer.attachments[1].view = VulkanUtil::createImageView(m_vulkan_rhi->m_device,
                                                                        m_framebuffer.attachments[1].image,
                                                                        m_framebuffer.attachments[1].format,
                                                                        VK_IMAGE_ASPECT_DEPTH_BIT,
                                                                        VK_IMAGE_VIEW_TYPE_2D,
                                                                        1,
                                                                        1);
    }
    void DirectionalLightShadowPass::setupRenderPass()
    {
        VkAttachmentDescription attachments[2] = {};

        VkAttachmentDescription& directional_light_shadow_color_attachment_description = attachments[0];
        directional_light_shadow_color_attachment_description.format         = m_framebuffer.attachments[0].format;
        directional_light_shadow_color_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
        directional_light_shadow_color_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        directional_light_shadow_color_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        directional_light_shadow_color_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        directional_light_shadow_color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        directional_light_shadow_color_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        directional_light_shadow_color_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& directional_light_shadow_depth_attachment_description = attachments[1];
        directional_light_shadow_depth_attachment_description.format         = m_framebuffer.attachments[1].format;
        directional_light_shadow_depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
        directional_light_shadow_depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        directional_light_shadow_depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        directional_light_shadow_depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        directional_light_shadow_depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        directional_light_shadow_depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        directional_light_shadow_depth_attachment_description.finalLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpasses[1] = {};

        VkAttachmentReference shadow_pass_color_attachment_reference {};
        shadow_pass_color_attachment_reference.attachment =
            &directional_light_shadow_color_attachment_description - attachments;
        shadow_pass_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference shadow_pass_depth_attachment_reference {};
        shadow_pass_depth_attachment_reference.attachment =
            &directional_light_shadow_depth_attachment_description - attachments;
        shadow_pass_depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& shadow_pass   = subpasses[0];
        shadow_pass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        shadow_pass.colorAttachmentCount    = 1;
        shadow_pass.pColorAttachments       = &shadow_pass_color_attachment_reference;
        shadow_pass.pDepthStencilAttachment = &shadow_pass_depth_attachment_reference;

        VkSubpassDependency dependencies[1] = {};

        VkSubpassDependency& lighting_pass_dependency = dependencies[0];
        lighting_pass_dependency.srcSubpass           = 0;
        lighting_pass_dependency.dstSubpass           = VK_SUBPASS_EXTERNAL;
        lighting_pass_dependency.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        lighting_pass_dependency.dstStageMask         = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        lighting_pass_dependency.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // STORE_OP_STORE
        lighting_pass_dependency.dstAccessMask        = 0;
        lighting_pass_dependency.dependencyFlags      = 0; // NOT BY REGION

        VkRenderPassCreateInfo renderpass_create_info {};
        renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        renderpass_create_info.pAttachments    = attachments;
        renderpass_create_info.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
        renderpass_create_info.pSubpasses      = subpasses;
        renderpass_create_info.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
        renderpass_create_info.pDependencies   = dependencies;

        if (vkCreateRenderPass(m_vulkan_rhi->m_device, &renderpass_create_info, nullptr, &m_framebuffer.render_pass) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create directional light shadow render pass");
        }
    }
    void DirectionalLightShadowPass::setupFramebuffer()
    {
        VkImageView attachments[2] = {m_framebuffer.attachments[0].view, m_framebuffer.attachments[1].view};

        VkFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.flags           = 0U;
        framebuffer_create_info.renderPass      = m_framebuffer.render_pass;
        framebuffer_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        framebuffer_create_info.pAttachments    = attachments;
        framebuffer_create_info.width           = s_directional_light_shadow_map_dimension;
        framebuffer_create_info.height          = s_directional_light_shadow_map_dimension;
        framebuffer_create_info.layers          = 1;

        if (vkCreateFramebuffer(
                m_vulkan_rhi->m_device, &framebuffer_create_info, nullptr, &m_framebuffer.framebuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("create directional light shadow framebuffer");
        }
    }
    void DirectionalLightShadowPass::setupDescriptorSetLayout()
    {
        m_descriptor_infos.resize(1);

        VkDescriptorSetLayoutBinding mesh_directional_light_shadow_global_layout_bindings[3];

        VkDescriptorSetLayoutBinding& mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding =
            mesh_directional_light_shadow_global_layout_bindings[0];
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.binding = 0;
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding& mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding =
            mesh_directional_light_shadow_global_layout_bindings[1];
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding&
            mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding =
                mesh_directional_light_shadow_global_layout_bindings[2];
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.binding = 2;
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding
            .descriptorCount = 1;
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo mesh_point_light_shadow_global_layout_create_info;
        mesh_point_light_shadow_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        mesh_point_light_shadow_global_layout_create_info.pNext = NULL;
        mesh_point_light_shadow_global_layout_create_info.flags = 0;
        mesh_point_light_shadow_global_layout_create_info.bindingCount =
            (sizeof(mesh_directional_light_shadow_global_layout_bindings) /
             sizeof(mesh_directional_light_shadow_global_layout_bindings[0]));
        mesh_point_light_shadow_global_layout_create_info.pBindings =
            mesh_directional_light_shadow_global_layout_bindings;

        if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_vulkan_rhi->m_device,
                                                      &mesh_point_light_shadow_global_layout_create_info,
                                                      NULL,
                                                      &m_descriptor_infos[0].layout))
        {
            throw std::runtime_error("create mesh directional light shadow global layout");
        }
    }
    void DirectionalLightShadowPass::setupPipelines()
    {
        m_render_pipelines.resize(1);

        VkDescriptorSetLayout      descriptorset_layouts[] = {m_descriptor_infos[0].layout, m_per_mesh_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
        pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = (sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]));
        pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

        if (vkCreatePipelineLayout(
                m_vulkan_rhi->m_device, &pipeline_layout_create_info, nullptr, &m_render_pipelines[0].layout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create mesh directional light shadow pipeline layout");
        }

        VkShaderModule vert_shader_module =
            VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, MESH_DIRECTIONAL_LIGHT_SHADOW_VERT);
        VkShaderModule frag_shader_module =
            VulkanUtil::createShaderModule(m_vulkan_rhi->m_device, MESH_DIRECTIONAL_LIGHT_SHADOW_FRAG);

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

        auto                                 vertex_binding_descriptions   = MeshVertex::getBindingDescriptions();
        auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_binding_descriptions[0];
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
        vertex_input_state_create_info.pVertexAttributeDescriptions    = &vertex_attribute_descriptions[0];

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
        input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {
            0, 0, s_directional_light_shadow_map_dimension, s_directional_light_shadow_map_dimension, 0.0, 1.0};
        VkRect2D scissor = {{0, 0},
                            {s_directional_light_shadow_map_dimension, s_directional_light_shadow_map_dimension}};

        VkPipelineViewportStateCreateInfo viewport_state_create_info {};
        viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports    = &viewport;
        viewport_state_create_info.scissorCount  = 1;
        viewport_state_create_info.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
        rasterization_state_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth               = 1.0f;
        rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
        dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 0;
        dynamic_state_create_info.pDynamicStates    = NULL;

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = (sizeof(shader_stages) / sizeof(shader_stages[0]));
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
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState       = &dynamic_state_create_info;

        if (vkCreateGraphicsPipelines(
                m_vulkan_rhi->m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_render_pipelines[0].pipeline) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create mesh directional light shadow graphics pipeline");
        }

        vkDestroyShaderModule(m_vulkan_rhi->m_device, vert_shader_module, nullptr);
    }
    void DirectionalLightShadowPass::setupDescriptorSet()
    {
        VkDescriptorSetAllocateInfo mesh_directional_light_shadow_global_descriptor_set_alloc_info;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.pNext          = NULL;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.descriptorPool = m_vulkan_rhi->m_descriptor_pool;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.pSetLayouts = &m_descriptor_infos[0].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_rhi->m_device,
                                                   &mesh_directional_light_shadow_global_descriptor_set_alloc_info,
                                                   &m_descriptor_infos[0].descriptor_set))
        {
            throw std::runtime_error("allocate mesh directional light shadow global descriptor set");
        }

        VkDescriptorBufferInfo mesh_directional_light_shadow_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of the buffer
        mesh_directional_light_shadow_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_directional_light_shadow_perframe_storage_buffer_info.range =
            sizeof(MeshDirectionalLightShadowPerframeStorageBufferObject);
        mesh_directional_light_shadow_perframe_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_directional_light_shadow_perframe_storage_buffer_info.range <
               m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_directional_light_shadow_perdrawcall_storage_buffer_info = {};
        mesh_directional_light_shadow_perdrawcall_storage_buffer_info.offset                 = 0;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_info.range =
            sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject);
        mesh_directional_light_shadow_perdrawcall_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_directional_light_shadow_perdrawcall_storage_buffer_info.range <
               m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info = {};
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.offset                 = 0;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.range =
            sizeof(MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject);
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.range <
               m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorSet descriptor_set_to_write = m_descriptor_infos[0].descriptor_set;

        VkWriteDescriptorSet descriptor_writes[3];

        VkWriteDescriptorSet& mesh_directional_light_shadow_perframe_storage_buffer_write_info = descriptor_writes[0];
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.pNext = NULL;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.dstSet          = descriptor_set_to_write;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.dstBinding      = 0;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.dstArrayElement = 0;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.descriptorCount = 1;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.pBufferInfo =
            &mesh_directional_light_shadow_perframe_storage_buffer_info;

        VkWriteDescriptorSet& mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info =
            descriptor_writes[1];
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.pNext           = NULL;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.dstSet          = descriptor_set_to_write;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.dstBinding      = 1;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.dstArrayElement = 0;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.descriptorCount = 1;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.pBufferInfo =
            &mesh_directional_light_shadow_perdrawcall_storage_buffer_info;

        VkWriteDescriptorSet& mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info =
            descriptor_writes[2];
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.pNext = NULL;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.dstSet =
            descriptor_set_to_write;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.dstBinding      = 2;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.dstArrayElement = 0;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.descriptorCount = 1;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.pBufferInfo =
            &mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info;

        vkUpdateDescriptorSets(m_vulkan_rhi->m_device,
                               (sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                               descriptor_writes,
                               0,
                               NULL);
    }
    void DirectionalLightShadowPass::drawModel()
    {
        struct MeshNode
        {
            const Matrix4x4* model_matrix {nullptr};
            const Matrix4x4* joint_matrices {nullptr};
            uint32_t         joint_count {0};
        };

        std::map<VulkanPBRMaterial*, std::map<VulkanMesh*, std::vector<MeshNode>>>
            directional_light_mesh_drawcall_batch;

        // reorganize mesh
        for (RenderMeshNode& node : *(m_visiable_nodes.p_directional_light_visible_mesh_nodes))
        {
            auto& mesh_instanced = directional_light_mesh_drawcall_batch[node.ref_material];
            auto& mesh_nodes     = mesh_instanced[node.ref_mesh];

            MeshNode temp;
            temp.model_matrix = node.model_matrix;
            if (node.enable_vertex_blending)
            {
                temp.joint_matrices = node.joint_matrices;
                temp.joint_count    = node.joint_count;
            }

            mesh_nodes.push_back(temp);
        }

        // Directional Light Shadow begin pass
        {
            VkRenderPassBeginInfo renderpass_begin_info {};
            renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_begin_info.renderPass        = m_framebuffer.render_pass;
            renderpass_begin_info.framebuffer       = m_framebuffer.framebuffer;
            renderpass_begin_info.renderArea.offset = {0, 0};
            renderpass_begin_info.renderArea.extent = {s_directional_light_shadow_map_dimension,
                                                       s_directional_light_shadow_map_dimension};

            VkClearValue clear_values[2];
            clear_values[0].color                 = {1.0f};
            clear_values[1].depthStencil          = {1.0f, 0};
            renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
            renderpass_begin_info.pClearValues    = clear_values;

            m_vulkan_rhi->m_vk_cmd_begin_render_pass(
                m_vulkan_rhi->m_current_command_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                                   NULL,
                                                   "Directional Light Shadow",
                                                   {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer, &label_info);
            }
        }

        // Mesh
        if (m_vulkan_rhi->isPointLightShadowEnabled())
        {
            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Mesh", {1.0f, 1.0f, 1.0f, 1.0f}};
                m_vulkan_rhi->m_vk_cmd_begin_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer, &label_info);
            }

            m_vulkan_rhi->m_vk_cmd_bind_pipeline(m_vulkan_rhi->m_current_command_buffer,
                                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 m_render_pipelines[0].pipeline);

            // perframe storage buffer
            uint32_t perframe_dynamic_offset =
                roundUp(m_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index],
                        m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
            m_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index] =
                perframe_dynamic_offset + sizeof(MeshPerframeStorageBufferObject);
            assert(m_global_render_resource->_storage_buffer
                       ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index] <=
                   (m_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_begin[m_vulkan_rhi->m_current_frame_index] +
                    m_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_size[m_vulkan_rhi->m_current_frame_index]));

            MeshDirectionalLightShadowPerframeStorageBufferObject& perframe_storage_buffer_object =
                (*reinterpret_cast<MeshDirectionalLightShadowPerframeStorageBufferObject*>(
                    reinterpret_cast<uintptr_t>(
                        m_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
                    perframe_dynamic_offset));
            perframe_storage_buffer_object = m_mesh_directional_light_shadow_perframe_storage_buffer_object;

            for (auto& [material, mesh_instanced] : directional_light_mesh_drawcall_batch)
            {
                // TODO: render from near to far

                for (auto& [mesh, mesh_nodes] : mesh_instanced)
                {
                    uint32_t total_instance_count = static_cast<uint32_t>(mesh_nodes.size());
                    if (total_instance_count > 0)
                    {
                        // bind per mesh
                        m_vulkan_rhi->m_vk_cmd_bind_descriptor_sets(m_vulkan_rhi->m_current_command_buffer,
                                                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                                    m_render_pipelines[0].layout,
                                                                    1,
                                                                    1,
                                                                    &mesh->mesh_vertex_blending_descriptor_set,
                                                                    0,
                                                                    NULL);

                        VkBuffer     vertex_buffers[] = {mesh->mesh_vertex_position_buffer};
                        VkDeviceSize offsets[]        = {0};
                        m_vulkan_rhi->m_vk_cmd_bind_vertex_buffers(
                            m_vulkan_rhi->m_current_command_buffer, 0, 1, vertex_buffers, offsets);
                        m_vulkan_rhi->m_vk_cmd_bind_index_buffer(
                            m_vulkan_rhi->m_current_command_buffer, mesh->mesh_index_buffer, 0, VK_INDEX_TYPE_UINT16);

                        uint32_t drawcall_max_instance_count =
                            (sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject::mesh_instances) /
                             sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject::mesh_instances[0]));
                        uint32_t drawcall_count =
                            roundUp(total_instance_count, drawcall_max_instance_count) / drawcall_max_instance_count;

                        for (uint32_t drawcall_index = 0; drawcall_index < drawcall_count; ++drawcall_index)
                        {
                            uint32_t current_instance_count =
                                ((total_instance_count - drawcall_max_instance_count * drawcall_index) <
                                 drawcall_max_instance_count) ?
                                    (total_instance_count - drawcall_max_instance_count * drawcall_index) :
                                    drawcall_max_instance_count;

                            // perdrawcall storage buffer
                            uint32_t perdrawcall_dynamic_offset =
                                roundUp(m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index],
                                        m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                            m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index] =
                                perdrawcall_dynamic_offset +
                                sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject);
                            assert(m_global_render_resource->_storage_buffer
                                       ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index] <=
                                   (m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_begin[m_vulkan_rhi->m_current_frame_index] +
                                    m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_size[m_vulkan_rhi->m_current_frame_index]));

                            MeshDirectionalLightShadowPerdrawcallStorageBufferObject&
                                perdrawcall_storage_buffer_object =
                                    (*reinterpret_cast<MeshDirectionalLightShadowPerdrawcallStorageBufferObject*>(
                                        reinterpret_cast<uintptr_t>(m_global_render_resource->_storage_buffer
                                                                        ._global_upload_ringbuffer_memory_pointer) +
                                        perdrawcall_dynamic_offset));
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                perdrawcall_storage_buffer_object.mesh_instances[i].model_matrix =
                                    *mesh_nodes[drawcall_max_instance_count * drawcall_index + i].model_matrix;
                                perdrawcall_storage_buffer_object.mesh_instances[i].enable_vertex_blending =
                                    mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices ? 1.0 :
                                                                                                                  -1.0;
                            }

                            // per drawcall vertex blending storage buffer
                            uint32_t per_drawcall_vertex_blending_dynamic_offset;
                            bool     least_one_enable_vertex_blending = true;
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                if (!mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices)
                                {
                                    least_one_enable_vertex_blending = false;
                                    break;
                                }
                            }
                            if (least_one_enable_vertex_blending)
                            {
                                per_drawcall_vertex_blending_dynamic_offset = roundUp(
                                    m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index],
                                    m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                                m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index] =
                                    per_drawcall_vertex_blending_dynamic_offset +
                                    sizeof(MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject);
                                assert(m_global_render_resource->_storage_buffer
                                           ._global_upload_ringbuffers_end[m_vulkan_rhi->m_current_frame_index] <=
                                       (m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_begin[m_vulkan_rhi->m_current_frame_index] +
                                        m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_size[m_vulkan_rhi->m_current_frame_index]));

                                MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject&
                                    per_drawcall_vertex_blending_storage_buffer_object =
                                        (*reinterpret_cast<
                                            MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject*>(
                                            reinterpret_cast<uintptr_t>(m_global_render_resource->_storage_buffer
                                                                            ._global_upload_ringbuffer_memory_pointer) +
                                            per_drawcall_vertex_blending_dynamic_offset));
                                for (uint32_t i = 0; i < current_instance_count; ++i)
                                {
                                    if (mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices)
                                    {
                                        for (uint32_t j = 0;
                                             j <
                                             mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_count;
                                             ++j)
                                        {
                                            per_drawcall_vertex_blending_storage_buffer_object
                                                .joint_matrices[s_mesh_vertex_blending_max_joint_count * i + j] =
                                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i]
                                                    .joint_matrices[j];
                                        }
                                    }
                                }
                            }
                            else
                            {
                                per_drawcall_vertex_blending_dynamic_offset = 0;
                            }

                            // bind perdrawcall
                            uint32_t dynamic_offsets[3] = {perframe_dynamic_offset,
                                                           perdrawcall_dynamic_offset,
                                                           per_drawcall_vertex_blending_dynamic_offset};
                            m_vulkan_rhi->m_vk_cmd_bind_descriptor_sets(
                                m_vulkan_rhi->m_current_command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_render_pipelines[0].layout,
                                0,
                                1,
                                &m_descriptor_infos[0].descriptor_set,
                                (sizeof(dynamic_offsets) / sizeof(dynamic_offsets[0])),
                                dynamic_offsets);
                            m_vulkan_rhi->m_vk_cmd_draw_indexed(m_vulkan_rhi->m_current_command_buffer,
                                                                mesh->mesh_index_count,
                                                                current_instance_count,
                                                                0,
                                                                0,
                                                                0);
                        }
                    }
                }
            }

            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer);
            }
        }

        // Directional Light Shadow end pass
        {
            if (m_vulkan_rhi->isDebugLabelEnabled())
            {
                m_vulkan_rhi->m_vk_cmd_end_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer);
            }

            m_vulkan_rhi->m_vk_cmd_end_render_pass(m_vulkan_rhi->m_current_command_buffer);
        }
    }
} // namespace Piccolo
