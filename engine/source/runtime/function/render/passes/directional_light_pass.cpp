#include "runtime/function/render/passes/directional_light_pass.h"

#include "runtime/function/render/render_helper.h"
#include "runtime/function/render/render_mesh.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/interface/vulkan/vulkan_util.h"

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
        m_framebuffer.attachments[0].format = RHI_FORMAT_R32_SFLOAT;
        m_rhi->createImage(s_directional_light_shadow_map_dimension,
                           s_directional_light_shadow_map_dimension,
                           m_framebuffer.attachments[0].format,
                           RHI_IMAGE_TILING_OPTIMAL,
                           RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT,
                           RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           m_framebuffer.attachments[0].image,
                           m_framebuffer.attachments[0].mem,
                           0,
                           1,
                           1);
        m_rhi->createImageView(m_framebuffer.attachments[0].image,
                               m_framebuffer.attachments[0].format,
                               RHI_IMAGE_ASPECT_COLOR_BIT,
                               RHI_IMAGE_VIEW_TYPE_2D,
                               1,
                               1,
                               m_framebuffer.attachments[0].view);

        // depth
        m_framebuffer.attachments[1].format = m_rhi->getDepthImageInfo().depth_image_format;
        m_rhi->createImage(s_directional_light_shadow_map_dimension,
                           s_directional_light_shadow_map_dimension,
                           m_framebuffer.attachments[1].format,
                           RHI_IMAGE_TILING_OPTIMAL,
                           RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                           RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           m_framebuffer.attachments[1].image,
                           m_framebuffer.attachments[1].mem,
                           0,
                           1,
                           1);
         m_rhi->createImageView(m_framebuffer.attachments[1].image,
                                m_framebuffer.attachments[1].format,
                                RHI_IMAGE_ASPECT_DEPTH_BIT,
                                RHI_IMAGE_VIEW_TYPE_2D,
                                1,
                                1,
                                m_framebuffer.attachments[1].view);
    }
    void DirectionalLightShadowPass::setupRenderPass()
    {
        RHIAttachmentDescription attachments[2] = {};

        RHIAttachmentDescription& directional_light_shadow_color_attachment_description = attachments[0];
        directional_light_shadow_color_attachment_description.format         = m_framebuffer.attachments[0].format;
        directional_light_shadow_color_attachment_description.samples        = RHI_SAMPLE_COUNT_1_BIT;
        directional_light_shadow_color_attachment_description.loadOp         = RHI_ATTACHMENT_LOAD_OP_CLEAR;
        directional_light_shadow_color_attachment_description.storeOp        = RHI_ATTACHMENT_STORE_OP_STORE;
        directional_light_shadow_color_attachment_description.stencilLoadOp  = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
        directional_light_shadow_color_attachment_description.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        directional_light_shadow_color_attachment_description.initialLayout  = RHI_IMAGE_LAYOUT_UNDEFINED;
        directional_light_shadow_color_attachment_description.finalLayout    = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIAttachmentDescription& directional_light_shadow_depth_attachment_description = attachments[1];
        directional_light_shadow_depth_attachment_description.format         = m_framebuffer.attachments[1].format;
        directional_light_shadow_depth_attachment_description.samples        = RHI_SAMPLE_COUNT_1_BIT;
        directional_light_shadow_depth_attachment_description.loadOp         = RHI_ATTACHMENT_LOAD_OP_CLEAR;
        directional_light_shadow_depth_attachment_description.storeOp        = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        directional_light_shadow_depth_attachment_description.stencilLoadOp  = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
        directional_light_shadow_depth_attachment_description.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        directional_light_shadow_depth_attachment_description.initialLayout  = RHI_IMAGE_LAYOUT_UNDEFINED;
        directional_light_shadow_depth_attachment_description.finalLayout =
            RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        RHISubpassDescription subpasses[1] = {};

        RHIAttachmentReference shadow_pass_color_attachment_reference {};
        shadow_pass_color_attachment_reference.attachment =
            &directional_light_shadow_color_attachment_description - attachments;
        shadow_pass_color_attachment_reference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RHIAttachmentReference shadow_pass_depth_attachment_reference {};
        shadow_pass_depth_attachment_reference.attachment =
            &directional_light_shadow_depth_attachment_description - attachments;
        shadow_pass_depth_attachment_reference.layout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        RHISubpassDescription& shadow_pass   = subpasses[0];
        shadow_pass.pipelineBindPoint       = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        shadow_pass.colorAttachmentCount    = 1;
        shadow_pass.pColorAttachments       = &shadow_pass_color_attachment_reference;
        shadow_pass.pDepthStencilAttachment = &shadow_pass_depth_attachment_reference;

        RHISubpassDependency dependencies[1] = {};

        RHISubpassDependency& lighting_pass_dependency = dependencies[0];
        lighting_pass_dependency.srcSubpass           = 0;
        lighting_pass_dependency.dstSubpass           = RHI_SUBPASS_EXTERNAL;
        lighting_pass_dependency.srcStageMask         = RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        lighting_pass_dependency.dstStageMask         = RHI_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        lighting_pass_dependency.srcAccessMask        = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // STORE_OP_STORE
        lighting_pass_dependency.dstAccessMask        = 0;
        lighting_pass_dependency.dependencyFlags      = 0; // NOT BY REGION

        RHIRenderPassCreateInfo renderpass_create_info {};
        renderpass_create_info.sType           = RHI_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        renderpass_create_info.pAttachments    = attachments;
        renderpass_create_info.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
        renderpass_create_info.pSubpasses      = subpasses;
        renderpass_create_info.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
        renderpass_create_info.pDependencies   = dependencies;

        if (RHI_SUCCESS != m_rhi->createRenderPass(&renderpass_create_info, m_framebuffer.render_pass))
        {
            throw std::runtime_error("create directional light shadow render pass");
        }
    }
    void DirectionalLightShadowPass::setupFramebuffer()
    {
        RHIImageView* attachments[2] = {m_framebuffer.attachments[0].view, m_framebuffer.attachments[1].view};

        RHIFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType           = RHI_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.flags           = 0U;
        framebuffer_create_info.renderPass      = m_framebuffer.render_pass;
        framebuffer_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        framebuffer_create_info.pAttachments    = attachments;
        framebuffer_create_info.width           = s_directional_light_shadow_map_dimension;
        framebuffer_create_info.height          = s_directional_light_shadow_map_dimension;
        framebuffer_create_info.layers          = 1;

        if (RHI_SUCCESS != m_rhi->createFramebuffer(&framebuffer_create_info, m_framebuffer.framebuffer))
        {
            throw std::runtime_error("create directional light shadow framebuffer");
        }
    }
    void DirectionalLightShadowPass::setupDescriptorSetLayout()
    {
        m_descriptor_infos.resize(1);

        RHIDescriptorSetLayoutBinding mesh_directional_light_shadow_global_layout_bindings[3];

        RHIDescriptorSetLayoutBinding& mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding =
            mesh_directional_light_shadow_global_layout_bindings[0];
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.binding = 0;
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
        mesh_directional_light_shadow_global_layout_perframe_storage_buffer_binding.stageFlags =
            RHI_SHADER_STAGE_VERTEX_BIT;

        RHIDescriptorSetLayoutBinding& mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding =
            mesh_directional_light_shadow_global_layout_bindings[1];
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
        mesh_directional_light_shadow_global_layout_perdrawcall_storage_buffer_binding.stageFlags =
            RHI_SHADER_STAGE_VERTEX_BIT;

        RHIDescriptorSetLayoutBinding&
            mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding =
                mesh_directional_light_shadow_global_layout_bindings[2];
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.binding = 2;
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding
            .descriptorCount = 1;
        mesh_directional_light_shadow_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.stageFlags =
            RHI_SHADER_STAGE_VERTEX_BIT;

        RHIDescriptorSetLayoutCreateInfo mesh_point_light_shadow_global_layout_create_info;
        mesh_point_light_shadow_global_layout_create_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        mesh_point_light_shadow_global_layout_create_info.pNext = NULL;
        mesh_point_light_shadow_global_layout_create_info.flags = 0;
        mesh_point_light_shadow_global_layout_create_info.bindingCount =
            (sizeof(mesh_directional_light_shadow_global_layout_bindings) /
             sizeof(mesh_directional_light_shadow_global_layout_bindings[0]));
        mesh_point_light_shadow_global_layout_create_info.pBindings =
            mesh_directional_light_shadow_global_layout_bindings;

        if (RHI_SUCCESS != m_rhi->createDescriptorSetLayout(&mesh_point_light_shadow_global_layout_create_info, m_descriptor_infos[0].layout))
        {
            throw std::runtime_error("create mesh directional light shadow global layout");
        }
    }
    void DirectionalLightShadowPass::setupPipelines()
    {
        m_render_pipelines.resize(1);

        RHIDescriptorSetLayout*      descriptorset_layouts[] = {m_descriptor_infos[0].layout, m_per_mesh_layout};
        RHIPipelineLayoutCreateInfo pipeline_layout_create_info {};
        pipeline_layout_create_info.sType          = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = (sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]));
        pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

        if (RHI_SUCCESS != m_rhi->createPipelineLayout(&pipeline_layout_create_info, m_render_pipelines[0].layout))
        {
            throw std::runtime_error("create mesh directional light shadow pipeline layout");
        }

        RHIShader* vert_shader_module =
            m_rhi->createShaderModule(MESH_DIRECTIONAL_LIGHT_SHADOW_VERT);
        RHIShader* frag_shader_module =
            m_rhi->createShaderModule(MESH_DIRECTIONAL_LIGHT_SHADOW_FRAG);

        RHIPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
        vert_pipeline_shader_stage_create_info.sType  = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage  = RHI_SHADER_STAGE_VERTEX_BIT;
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;
        vert_pipeline_shader_stage_create_info.pName  = "main";

        RHIPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
        frag_pipeline_shader_stage_create_info.sType  = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage  = RHI_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName  = "main";

        RHIPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                           frag_pipeline_shader_stage_create_info};

        auto                                 vertex_binding_descriptions   = MeshVertex::getBindingDescriptions();
        auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
        RHIPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
        vertex_input_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_binding_descriptions[0];
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
        vertex_input_state_create_info.pVertexAttributeDescriptions    = &vertex_attribute_descriptions[0];

        RHIPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
        input_assembly_create_info.sType                  = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology               = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = RHI_FALSE;

        RHIViewport viewport = {
            0, 0, s_directional_light_shadow_map_dimension, s_directional_light_shadow_map_dimension, 0.0, 1.0};
        RHIRect2D scissor = {{0, 0},
                            {s_directional_light_shadow_map_dimension, s_directional_light_shadow_map_dimension}};

        RHIPipelineViewportStateCreateInfo viewport_state_create_info {};
        viewport_state_create_info.sType         = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports    = &viewport;
        viewport_state_create_info.scissorCount  = 1;
        viewport_state_create_info.pScissors     = &scissor;

        RHIPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
        rasterization_state_create_info.sType            = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = RHI_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = RHI_FALSE;
        rasterization_state_create_info.polygonMode             = RHI_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth               = 1.0f;
        rasterization_state_create_info.cullMode                = RHI_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace               = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable         = RHI_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp          = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

        RHIPipelineMultisampleStateCreateInfo multisample_state_create_info {};
        multisample_state_create_info.sType                = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable  = RHI_FALSE;
        multisample_state_create_info.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

        RHIPipelineColorBlendAttachmentState color_blend_attachment_state {};
        color_blend_attachment_state.colorWriteMask =
            RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable         = RHI_FALSE;
        color_blend_attachment_state.srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp        = RHI_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp        = RHI_BLEND_OP_ADD;

        RHIPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
        color_blend_state_create_info.sType             = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable     = RHI_FALSE;
        color_blend_state_create_info.logicOp           = RHI_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount   = 1;
        color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

        RHIPipelineDepthStencilStateCreateInfo depth_stencil_create_info {};
        depth_stencil_create_info.sType                 = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable       = RHI_TRUE;
        depth_stencil_create_info.depthWriteEnable      = RHI_TRUE;
        depth_stencil_create_info.depthCompareOp        = RHI_COMPARE_OP_LESS;
        depth_stencil_create_info.depthBoundsTestEnable = RHI_FALSE;
        depth_stencil_create_info.stencilTestEnable     = RHI_FALSE;

        RHIPipelineDynamicStateCreateInfo dynamic_state_create_info {};
        dynamic_state_create_info.sType             = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 0;
        dynamic_state_create_info.pDynamicStates    = NULL;

        RHIGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType               = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
        pipelineInfo.basePipelineHandle  = RHI_NULL_HANDLE;
        pipelineInfo.pDynamicState       = &dynamic_state_create_info;

        if (RHI_SUCCESS != m_rhi->createGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_render_pipelines[0].pipeline))
        {
            throw std::runtime_error("create mesh directional light shadow graphics pipeline");
        }

        m_rhi->destroyShaderModule(vert_shader_module);
    }
    void DirectionalLightShadowPass::setupDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo mesh_directional_light_shadow_global_descriptor_set_alloc_info;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.sType =
            RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.pNext          = NULL;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.descriptorPool = m_rhi->getDescriptorPoor();
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_directional_light_shadow_global_descriptor_set_alloc_info.pSetLayouts = &m_descriptor_infos[0].layout;

        if (RHI_SUCCESS != m_rhi->allocateDescriptorSets(&mesh_directional_light_shadow_global_descriptor_set_alloc_info, m_descriptor_infos[0].descriptor_set))
        {
            throw std::runtime_error("allocate mesh directional light shadow global descriptor set");
        }

        RHIDescriptorBufferInfo mesh_directional_light_shadow_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of the buffer
        mesh_directional_light_shadow_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_directional_light_shadow_perframe_storage_buffer_info.range =
            sizeof(MeshDirectionalLightShadowPerframeStorageBufferObject);
        mesh_directional_light_shadow_perframe_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_directional_light_shadow_perframe_storage_buffer_info.range <
               m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo mesh_directional_light_shadow_perdrawcall_storage_buffer_info = {};
        mesh_directional_light_shadow_perdrawcall_storage_buffer_info.offset                 = 0;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_info.range =
            sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject);
        mesh_directional_light_shadow_perdrawcall_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_directional_light_shadow_perdrawcall_storage_buffer_info.range <
               m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info = {};
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.offset                 = 0;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.range =
            sizeof(MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject);
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info.range <
               m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorSet* descriptor_set_to_write = m_descriptor_infos[0].descriptor_set;

        RHIWriteDescriptorSet descriptor_writes[3];

        RHIWriteDescriptorSet& mesh_directional_light_shadow_perframe_storage_buffer_write_info = descriptor_writes[0];
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.pNext = NULL;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.dstSet          = descriptor_set_to_write;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.dstBinding      = 0;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.dstArrayElement = 0;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.descriptorCount = 1;
        mesh_directional_light_shadow_perframe_storage_buffer_write_info.pBufferInfo =
            &mesh_directional_light_shadow_perframe_storage_buffer_info;

        RHIWriteDescriptorSet& mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info =
            descriptor_writes[1];
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.sType =
            RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.pNext           = NULL;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.dstSet          = descriptor_set_to_write;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.dstBinding      = 1;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.dstArrayElement = 0;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.descriptorCount = 1;
        mesh_directional_light_shadow_perdrawcall_storage_buffer_write_info.pBufferInfo =
            &mesh_directional_light_shadow_perdrawcall_storage_buffer_info;

        RHIWriteDescriptorSet& mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info =
            descriptor_writes[2];
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.sType =
            RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.pNext = NULL;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.dstSet =
            descriptor_set_to_write;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.dstBinding      = 2;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.dstArrayElement = 0;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.descriptorCount = 1;
        mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_write_info.pBufferInfo =
            &mesh_directional_light_shadow_per_drawcall_vertex_blending_storage_buffer_info;

        m_rhi->updateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
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
            RHIRenderPassBeginInfo renderpass_begin_info {};
            renderpass_begin_info.sType             = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_begin_info.renderPass        = m_framebuffer.render_pass;
            renderpass_begin_info.framebuffer       = m_framebuffer.framebuffer;
            renderpass_begin_info.renderArea.offset = {0, 0};
            renderpass_begin_info.renderArea.extent = {s_directional_light_shadow_map_dimension,
                                                       s_directional_light_shadow_map_dimension};

            RHIClearValue clear_values[2];
            clear_values[0].color                 = {1.0f};
            clear_values[1].depthStencil          = {1.0f, 0};
            renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
            renderpass_begin_info.pClearValues    = clear_values;

            m_rhi->cmdBeginRenderPassPFN(m_rhi->getCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);

            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "Directional Light Shadow", color);
        }

        // Mesh
        if (m_rhi->isPointLightShadowEnabled())
        {
            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "Mesh", color);

            m_rhi->cmdBindPipelinePFN(m_rhi->getCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_render_pipelines[0].pipeline);

            // perframe storage buffer
            uint32_t perframe_dynamic_offset =
                roundUp(m_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()],
                        m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
            m_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] =
                perframe_dynamic_offset + sizeof(MeshPerframeStorageBufferObject);
            assert(m_global_render_resource->_storage_buffer
                       ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] <=
                   (m_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_begin[m_rhi->getCurrentFrameIndex()] +
                    m_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_size[m_rhi->getCurrentFrameIndex()]));

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
                        m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                                                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                                                        m_render_pipelines[0].layout,
                                                        1,
                                                        1,
                                                        &mesh->mesh_vertex_blending_descriptor_set,
                                                        0,
                                                        NULL);

                        RHIBuffer*     vertex_buffers[] = {mesh->mesh_vertex_position_buffer};
                        RHIDeviceSize offsets[]        = {0};
                        m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, vertex_buffers, offsets);
                        m_rhi->cmdBindIndexBufferPFN(m_rhi->getCurrentCommandBuffer(), mesh->mesh_index_buffer, 0, RHI_INDEX_TYPE_UINT16);

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
                                            ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()],
                                        m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                            m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] =
                                perdrawcall_dynamic_offset +
                                sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject);
                            assert(m_global_render_resource->_storage_buffer
                                       ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] <=
                                   (m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_begin[m_rhi->getCurrentFrameIndex()] +
                                    m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_size[m_rhi->getCurrentFrameIndex()]));

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
                                        ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()],
                                    m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                                m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] =
                                    per_drawcall_vertex_blending_dynamic_offset +
                                    sizeof(MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject);
                                assert(m_global_render_resource->_storage_buffer
                                           ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] <=
                                       (m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_begin[m_rhi->getCurrentFrameIndex()] +
                                        m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_size[m_rhi->getCurrentFrameIndex()]));

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
                            m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                                                            RHI_PIPELINE_BIND_POINT_GRAPHICS,
                                                            m_render_pipelines[0].layout,
                                                            0,
                                                            1,
                                                            &m_descriptor_infos[0].descriptor_set,
                                                            (sizeof(dynamic_offsets) / sizeof(dynamic_offsets[0])),
                                                            dynamic_offsets);
                            m_rhi->cmdDrawIndexedPFN(m_rhi->getCurrentCommandBuffer(),
                                                     mesh->mesh_index_count,
                                                     current_instance_count,
                                                     0,
                                                     0,
                                                     0);
                        }
                    }
                }
            }

            m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());
        }

        // Directional Light Shadow end pass
        {
            m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());

            m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }
} // namespace Piccolo
