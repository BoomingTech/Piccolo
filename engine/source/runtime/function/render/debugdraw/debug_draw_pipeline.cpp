#include "debug_draw_pipeline.h"
#include <fstream>
#include <debugdraw_vert.h>
#include <debugdraw_frag.h>
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/render_system.h"
namespace Piccolo
{
    void DebugDrawPipeline::initialize()
    {
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        setupAttachments();
        setupRenderPass();
        setupFramebuffer();
        setupDescriptorLayout();
        setupPipelines();
    }
    
    void DebugDrawPipeline::recreateAfterSwapchain()
    {
        for (auto framebuffer : m_framebuffer.framebuffers)
        {
            m_rhi->destroyFramebuffer(framebuffer);
        }

        setupFramebuffer();
    }

    void DebugDrawPipeline::setupAttachments()
    {

    }
    void DebugDrawPipeline::setupRenderPass()
    {
        RHIAttachmentDescription color_attachment_description{};
        color_attachment_description.format = m_rhi->getSwapchainInfo().image_format;
        color_attachment_description.samples = RHI_SAMPLE_COUNT_1_BIT;
        color_attachment_description.loadOp = RHI_ATTACHMENT_LOAD_OP_LOAD;
        color_attachment_description.storeOp = RHI_ATTACHMENT_STORE_OP_STORE;
        color_attachment_description.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_description.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment_description.initialLayout = RHI_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        color_attachment_description.finalLayout = RHI_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        RHIAttachmentReference color_attachment_reference{};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RHIAttachmentDescription depth_attachment_description{};
        depth_attachment_description.format = m_rhi->getDepthImageInfo().depth_image_format;
        depth_attachment_description.samples = RHI_SAMPLE_COUNT_1_BIT;
        depth_attachment_description.loadOp = RHI_ATTACHMENT_LOAD_OP_LOAD;
        depth_attachment_description.storeOp = RHI_ATTACHMENT_STORE_OP_STORE;
        depth_attachment_description.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment_description.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.initialLayout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_attachment_description.finalLayout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        RHIAttachmentReference depth_attachment_reference{};
        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        RHISubpassDescription subpass{};
        subpass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;
        subpass.pDepthStencilAttachment = &depth_attachment_reference;

        std::array<RHIAttachmentDescription, 2> attachments = { color_attachment_description, depth_attachment_description };

        RHISubpassDependency dependencies[1] = {};
        RHISubpassDependency& debug_draw_dependency = dependencies[0];
        debug_draw_dependency.srcSubpass = 0;
        debug_draw_dependency.dstSubpass = RHI_SUBPASS_EXTERNAL;
        debug_draw_dependency.srcStageMask = RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RHI_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        debug_draw_dependency.dstStageMask = RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RHI_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        debug_draw_dependency.srcAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RHI_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // STORE_OP_STORE
        debug_draw_dependency.dstAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RHI_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        debug_draw_dependency.dependencyFlags = 0; // NOT BY REGION

        RHIRenderPassCreateInfo renderpass_create_info{};
        renderpass_create_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = attachments.size();
        renderpass_create_info.pAttachments = attachments.data();
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpass;
        renderpass_create_info.dependencyCount = 1;
        renderpass_create_info.pDependencies = dependencies;

        if (m_rhi->createRenderPass(&renderpass_create_info, m_framebuffer.render_pass) != RHI_SUCCESS)
        {
            throw std::runtime_error("create inefficient pick render pass");
        }
    }
    void DebugDrawPipeline::setupFramebuffer()
    {
        const std::vector<RHIImageView*>&& imageViews = m_rhi->getSwapchainInfo().imageViews;
        m_framebuffer.framebuffers.resize(imageViews.size());
        for (size_t i = 0; i < m_framebuffer.framebuffers.size(); i++) {

            RHIImageView* attachments[2] = { imageViews[i], m_rhi->getDepthImageInfo().depth_image_view};

            RHIFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = RHI_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = m_framebuffer.render_pass;
            framebuffer_create_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
            framebuffer_create_info.pAttachments = attachments;
            framebuffer_create_info.width = m_rhi->getSwapchainInfo().extent.width;
            framebuffer_create_info.height = m_rhi->getSwapchainInfo().extent.height;
            framebuffer_create_info.layers = 1;
            
            if (m_rhi->createFramebuffer(&framebuffer_create_info, m_framebuffer.framebuffers[i]) !=RHI_SUCCESS)
            {
                throw std::runtime_error("create inefficient pick framebuffer");
            }
        }
    }

    void DebugDrawPipeline::setupDescriptorLayout()
    {
        RHIDescriptorSetLayoutBinding uboLayoutBinding[3];
        uboLayoutBinding[0].binding = 0;
        uboLayoutBinding[0].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding[0].descriptorCount = 1;
        uboLayoutBinding[0].stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[0].pImmutableSamplers = nullptr;

        uboLayoutBinding[1].binding = 1;
        uboLayoutBinding[1].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        uboLayoutBinding[1].descriptorCount = 1;
        uboLayoutBinding[1].stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[1].pImmutableSamplers = nullptr;

        uboLayoutBinding[2].binding = 2;
        uboLayoutBinding[2].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uboLayoutBinding[2].descriptorCount = 1;
        uboLayoutBinding[2].stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding[2].pImmutableSamplers = nullptr;

        RHIDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 3;
        layoutInfo.pBindings = uboLayoutBinding;

        if (m_rhi->createDescriptorSetLayout(&layoutInfo, m_descriptor_layout) != RHI_SUCCESS)
        {
            throw std::runtime_error("create debug draw layout");
        }
    }

    void DebugDrawPipeline::setupPipelines()
    {
        m_render_pipelines.resize(1);

        RHIPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts = &m_descriptor_layout;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        if (m_rhi->createPipelineLayout(&pipeline_layout_create_info, m_render_pipelines[0].layout) != RHI_SUCCESS)
        {
            throw std::runtime_error("create mesh inefficient pick pipeline layout");
        }

        RHIShader* vert_shader_module = m_rhi->createShaderModule(DEBUGDRAW_VERT);
        RHIShader* frag_shader_module = m_rhi->createShaderModule(DEBUGDRAW_FRAG);

        RHIPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
        vert_pipeline_shader_stage_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage = RHI_SHADER_STAGE_VERTEX_BIT;
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;
        vert_pipeline_shader_stage_create_info.pName = "main";

        RHIPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
        frag_pipeline_shader_stage_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName = "main";

        RHIPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                           frag_pipeline_shader_stage_create_info };

        auto vertex_binding_descriptions = DebugDrawVertex::getBindingDescriptions();
        auto vertex_attribute_descriptions = DebugDrawVertex::getAttributeDescriptions();
        RHIPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_binding_descriptions.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

        RHIPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
        input_assembly_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_LINE_LIST;
        input_assembly_create_info.primitiveRestartEnable = RHI_FALSE;

        RHIPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports = m_rhi->getSwapchainInfo().viewport;
        viewport_state_create_info.scissorCount = 1;
        viewport_state_create_info.pScissors = m_rhi->getSwapchainInfo().scissor;

        RHIPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
        rasterization_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = RHI_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = RHI_FALSE;
        rasterization_state_create_info.polygonMode = RHI_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth = 1.0f;
        rasterization_state_create_info.cullMode = RHI_CULL_MODE_NONE;
        rasterization_state_create_info.frontFace = RHI_FRONT_FACE_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable = RHI_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

        RHIPipelineMultisampleStateCreateInfo multisample_state_create_info{};
        multisample_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable = RHI_FALSE;
        multisample_state_create_info.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

        RHIPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = RHI_TRUE;
        color_blend_attachment_state.srcColorBlendFactor = RHI_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = RHI_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp = RHI_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp = RHI_BLEND_OP_ADD;

        RHIPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
        color_blend_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable = RHI_FALSE;
        color_blend_state_create_info.logicOp = RHI_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount = 1;
        color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

        RHIPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
        depth_stencil_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable = RHI_TRUE;
        depth_stencil_create_info.depthWriteEnable = RHI_TRUE;
        depth_stencil_create_info.depthCompareOp = RHI_COMPARE_OP_LESS;
        depth_stencil_create_info.depthBoundsTestEnable = RHI_FALSE;
        depth_stencil_create_info.stencilTestEnable = RHI_FALSE;

        RHIDynamicState                   dynamic_states[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
        RHIPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates = dynamic_states;

        RHIGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shader_stages;
        pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
        pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
        pipelineInfo.pViewportState = &viewport_state_create_info;
        pipelineInfo.pRasterizationState = &rasterization_state_create_info;
        pipelineInfo.pMultisampleState = &multisample_state_create_info;
        pipelineInfo.pColorBlendState = &color_blend_state_create_info;
        pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
        pipelineInfo.layout = m_render_pipelines[0].layout;
        pipelineInfo.renderPass = m_framebuffer.render_pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamic_state_create_info;

        if (m_pipeline_type == _debug_draw_pipeline_type_point)
        {
            input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }
        else if (m_pipeline_type == _debug_draw_pipeline_type_line)
        {
            input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
        else if (m_pipeline_type == _debug_draw_pipeline_type_triangle)
        {
            input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        else if (m_pipeline_type == _debug_draw_pipeline_type_point_no_depth_test)
        {
            input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_POINT_LIST;
            depth_stencil_create_info.depthTestEnable = RHI_FALSE;
        }
        else if (m_pipeline_type == _debug_draw_pipeline_type_line_no_depth_test)
        {
            input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_LINE_LIST;
            depth_stencil_create_info.depthTestEnable = RHI_FALSE;
        }
        else if (m_pipeline_type == _debug_draw_pipeline_type_triangle_no_depth_test)
        {
            input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            depth_stencil_create_info.depthTestEnable = RHI_FALSE;
        }

        if (m_rhi->createGraphicsPipelines(
            RHI_NULL_HANDLE, 1, &pipelineInfo, m_render_pipelines[0].pipeline) !=
            RHI_SUCCESS)
        {
            throw std::runtime_error("create debug draw graphics pipeline");
        }

        m_rhi->destroyShaderModule(vert_shader_module);
        m_rhi->destroyShaderModule(frag_shader_module);
    }

    void DebugDrawPipeline::destory()
    {
        
    }
    const DebugDrawPipelineBase &DebugDrawPipeline::getPipeline() const
    {
        return m_render_pipelines[0];
    }
    const DebugDrawFramebuffer &DebugDrawPipeline::getFramebuffer() const
    {
        return m_framebuffer;
    }
}
