#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_mesh.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_misc.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_pick_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_util.h"

#include <mesh_inefficient_pick_frag.h>
#include <mesh_inefficient_pick_vert.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <map>

namespace Pilot
{
    void PPickPass::initialize()
    {
        setupAttachments();
        setupRenderPass();
        setupFramebuffer();
        setupDescriptorSetLayout();
        setupPipelines();
        setupDescriptorSet();
    }
    void PPickPass::postInitialize() {}
    void PPickPass::draw() {}
    void PPickPass::setupAttachments()
    {
        _framebuffer.attachments.resize(1);
        _framebuffer.attachments[0].format = VK_FORMAT_R32_UINT;

        PVulkanUtil::createImage(m_p_vulkan_context->_physical_device,
                                 m_p_vulkan_context->_device,
                                 m_p_vulkan_context->_swapchain_extent.width,
                                 m_p_vulkan_context->_swapchain_extent.height,
                                 _framebuffer.attachments[0].format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 _framebuffer.attachments[0].image,
                                 _framebuffer.attachments[0].mem,
                                 0,
                                 1,
                                 1);
        _framebuffer.attachments[0].view = PVulkanUtil::createImageView(m_p_vulkan_context->_device,
                                                                        _framebuffer.attachments[0].image,
                                                                        _framebuffer.attachments[0].format,
                                                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                                                        VK_IMAGE_VIEW_TYPE_2D,
                                                                        1,
                                                                        1);
    }
    void PPickPass::setupRenderPass()
    {
        VkAttachmentDescription color_attachment_description {};
        color_attachment_description.format         = _framebuffer.attachments[0].format;
        color_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
        color_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        VkAttachmentDescription depth_attachment_description {};
        depth_attachment_description.format         = m_p_vulkan_context->_depth_image_format;
        depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription attachments[2] = {color_attachment_description, depth_attachment_description};

        VkAttachmentReference color_attachment_reference {};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_reference {};
        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &color_attachment_reference;
        subpass.pDepthStencilAttachment = &depth_attachment_reference;

        VkRenderPassCreateInfo renderpass_create_info {};
        renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        renderpass_create_info.pAttachments    = attachments;
        renderpass_create_info.subpassCount    = 1;
        renderpass_create_info.pSubpasses      = &subpass;
        renderpass_create_info.dependencyCount = 0;
        renderpass_create_info.pDependencies   = NULL;

        if (vkCreateRenderPass(
                m_p_vulkan_context->_device, &renderpass_create_info, nullptr, &_framebuffer.render_pass) != VK_SUCCESS)
        {
            throw std::runtime_error("create inefficient pick render pass");
        }
    }
    void PPickPass::setupFramebuffer()
    {
        VkImageView attachments[2] = {_framebuffer.attachments[0].view, m_p_vulkan_context->_depth_image_view};

        VkFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = _framebuffer.render_pass;
        framebuffer_create_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        framebuffer_create_info.pAttachments    = attachments;
        framebuffer_create_info.width           = m_p_vulkan_context->_swapchain_extent.width;
        framebuffer_create_info.height          = m_p_vulkan_context->_swapchain_extent.height;
        framebuffer_create_info.layers          = 1;

        if (vkCreateFramebuffer(
                m_p_vulkan_context->_device, &framebuffer_create_info, nullptr, &_framebuffer.framebuffer) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create inefficient pick framebuffer");
        }
    }
    void PPickPass::setupDescriptorSetLayout()
    {
        _descriptor_infos.resize(1);

        VkDescriptorSetLayoutBinding mesh_inefficient_pick_global_layout_bindings[3];

        VkDescriptorSetLayoutBinding& mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding =
            mesh_inefficient_pick_global_layout_bindings[0];
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.binding = 0;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

        VkDescriptorSetLayoutBinding& mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding =
            mesh_inefficient_pick_global_layout_bindings[1];
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.pImmutableSamplers = NULL;

        VkDescriptorSetLayoutBinding&
            mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding =
                mesh_inefficient_pick_global_layout_bindings[2];
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.binding = 2;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.descriptorCount = 1;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.pImmutableSamplers =
            NULL;

        VkDescriptorSetLayoutCreateInfo mesh_inefficient_pick_global_layout_create_info;
        mesh_inefficient_pick_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        mesh_inefficient_pick_global_layout_create_info.pNext = NULL;
        mesh_inefficient_pick_global_layout_create_info.flags = 0;
        mesh_inefficient_pick_global_layout_create_info.bindingCount =
            (sizeof(mesh_inefficient_pick_global_layout_bindings) /
             sizeof(mesh_inefficient_pick_global_layout_bindings[0]));
        mesh_inefficient_pick_global_layout_create_info.pBindings = mesh_inefficient_pick_global_layout_bindings;

        if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_p_vulkan_context->_device,
                                                      &mesh_inefficient_pick_global_layout_create_info,
                                                      NULL,
                                                      &_descriptor_infos[0].layout))
        {
            throw std::runtime_error("create mesh inefficient pick global layout");
        }
    }
    void PPickPass::setupPipelines()
    {
        _render_pipelines.resize(1);

        VkDescriptorSetLayout descriptorset_layouts[] = {_descriptor_infos[0].layout, _per_mesh_layout};

        VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
        pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]);
        pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

        if (vkCreatePipelineLayout(
                m_p_vulkan_context->_device, &pipeline_layout_create_info, nullptr, &_render_pipelines[0].layout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create mesh inefficient pick pipeline layout");
        }

        VkShaderModule vert_shader_module =
            PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, MESH_INEFFICIENT_PICK_VERT);
        VkShaderModule frag_shader_module =
            PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, MESH_INEFFICIENT_PICK_FRAG);

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

        auto                                 vertex_binding_descriptions   = PMeshVertex::getBindingDescriptions();
        auto                                 vertex_attribute_descriptions = PMeshVertex::getAttributeDescriptions();
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

        VkPipelineViewportStateCreateInfo viewport_state_create_info {};
        viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports    = &m_command_info._viewport;
        viewport_state_create_info.scissorCount  = 1;
        viewport_state_create_info.pScissors     = &m_command_info._scissor;

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
        color_blend_attachment_state.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT;
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

        VkDynamicState                   dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
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
        pipelineInfo.layout              = _render_pipelines[0].layout;
        pipelineInfo.renderPass          = _framebuffer.render_pass;
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState       = &dynamic_state_create_info;

        if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                      VK_NULL_HANDLE,
                                      1,
                                      &pipelineInfo,
                                      nullptr,
                                      &_render_pipelines[0].pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("create mesh inefficient pick graphics pipeline");
        }

        vkDestroyShaderModule(m_p_vulkan_context->_device, vert_shader_module, nullptr);
        vkDestroyShaderModule(m_p_vulkan_context->_device, frag_shader_module, nullptr);
    }
    void PPickPass::setupDescriptorSet()
    {
        VkDescriptorSetAllocateInfo mesh_inefficient_pick_global_descriptor_set_alloc_info;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.pNext = NULL;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.pSetLayouts        = &_descriptor_infos[0].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_p_vulkan_context->_device,
                                                   &mesh_inefficient_pick_global_descriptor_set_alloc_info,
                                                   &_descriptor_infos[0].descriptor_set))
        {
            throw std::runtime_error("allocate mesh inefficient pick global descriptor set");
        }

        VkDescriptorBufferInfo mesh_inefficient_pick_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of
        // the buffer
        mesh_inefficient_pick_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_inefficient_pick_perframe_storage_buffer_info.range =
            sizeof(MeshInefficientPickPerframeStorageBufferObject);
        mesh_inefficient_pick_perframe_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;

        VkDescriptorBufferInfo mesh_inefficient_pick_perdrawcall_storage_buffer_info = {};
        mesh_inefficient_pick_perdrawcall_storage_buffer_info.offset                 = 0;
        mesh_inefficient_pick_perdrawcall_storage_buffer_info.range =
            sizeof(MeshInefficientPickPerdrawcallStorageBufferObject);
        mesh_inefficient_pick_perdrawcall_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_inefficient_pick_perdrawcall_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info = {};
        mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.offset                 = 0;
        mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.range =
            sizeof(MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject);
        mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkWriteDescriptorSet mesh_descriptor_writes_info[3];

        mesh_descriptor_writes_info[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[0].pNext           = NULL;
        mesh_descriptor_writes_info[0].dstSet          = _descriptor_infos[0].descriptor_set;
        mesh_descriptor_writes_info[0].dstBinding      = 0;
        mesh_descriptor_writes_info[0].dstArrayElement = 0;
        mesh_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[0].descriptorCount = 1;
        mesh_descriptor_writes_info[0].pBufferInfo     = &mesh_inefficient_pick_perframe_storage_buffer_info;

        mesh_descriptor_writes_info[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[1].pNext           = NULL;
        mesh_descriptor_writes_info[1].dstSet          = _descriptor_infos[0].descriptor_set;
        mesh_descriptor_writes_info[1].dstBinding      = 1;
        mesh_descriptor_writes_info[1].dstArrayElement = 0;
        mesh_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[1].descriptorCount = 1;
        mesh_descriptor_writes_info[1].pBufferInfo     = &mesh_inefficient_pick_perdrawcall_storage_buffer_info;

        mesh_descriptor_writes_info[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[2].pNext           = NULL;
        mesh_descriptor_writes_info[2].dstSet          = _descriptor_infos[0].descriptor_set;
        mesh_descriptor_writes_info[2].dstBinding      = 2;
        mesh_descriptor_writes_info[2].dstArrayElement = 0;
        mesh_descriptor_writes_info[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[2].descriptorCount = 1;
        mesh_descriptor_writes_info[2].pBufferInfo =
            &mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info;

        vkUpdateDescriptorSets(m_p_vulkan_context->_device,
                               sizeof(mesh_descriptor_writes_info) / sizeof(mesh_descriptor_writes_info[0]),
                               mesh_descriptor_writes_info,
                               0,
                               NULL);
    }
    void PPickPass::recreateFramebuffer()
    {
        for (size_t i = 0; i < _framebuffer.attachments.size(); i++)
        {
            vkDestroyImage(m_p_vulkan_context->_device, _framebuffer.attachments[i].image, nullptr);
            vkDestroyImageView(m_p_vulkan_context->_device, _framebuffer.attachments[i].view, nullptr);
            vkFreeMemory(m_p_vulkan_context->_device, _framebuffer.attachments[i].mem, nullptr);
        }
        vkDestroyFramebuffer(m_p_vulkan_context->_device, _framebuffer.framebuffer, nullptr);

        setupAttachments();
        setupFramebuffer();
    }
    uint32_t PPickPass::pick(glm::vec2 click_uv)
    {
        uint32_t pixel_x =
            static_cast<uint32_t>(click_uv.x * m_command_info._viewport.width + m_command_info._viewport.x);
        uint32_t pixel_y =
            static_cast<uint32_t>(click_uv.y * m_command_info._viewport.height + m_command_info._viewport.y);
        uint32_t picked_pixel_index = m_p_vulkan_context->_swapchain_extent.width * pixel_y + pixel_x;
        if (pixel_x >= m_p_vulkan_context->_swapchain_extent.width ||
            pixel_y >= m_p_vulkan_context->_swapchain_extent.height)
            return 0;

        struct PMeshNode
        {
            glm::mat4 model_matrix;
            uint32_t  node_id;
            glm::mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count];
        };

        std::map<VulkanPBRMaterial*, std::map<VulkanMesh*, std::vector<PMeshNode>>> main_camera_mesh_drawcall_batch;

        // reorganize mesh
        for (PVulkanMeshNode& node : *(m_visiable_nodes.p_main_camera_visible_mesh_nodes))
        {
            auto& mesh_instanced = main_camera_mesh_drawcall_batch[node.ref_material];
            auto& model_nodes    = mesh_instanced[node.ref_mesh];

            PMeshNode temp;
            temp.model_matrix = node.model_matrix;
            temp.node_id      = node.node_id;
            if (node.ref_mesh->enable_vertex_blending)
            {
                for (uint32_t i = 0; i < m_mesh_vertex_blending_max_joint_count; ++i)
                {
                    temp.joint_matrices[i] = node.joint_matrices[i];
                }
            }

            model_nodes.push_back(temp);
        }

        // reset storage buffer offset
        m_p_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] =
            m_p_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_begin[*m_command_info._p_current_frame_index];

        VkResult res_wait_for_fences =
            vkWaitForFences(m_p_vulkan_context->_device,
                            1,
                            &m_command_info._is_frame_in_flight_fences[*m_command_info._p_current_frame_index],
                            VK_TRUE,
                            UINT64_MAX);
        assert(VK_SUCCESS == res_wait_for_fences);

        VkResult res_reset_command_pool = m_p_vulkan_context->_vkResetCommandPool(
            m_p_vulkan_context->_device, m_command_info._p_command_pools[*m_command_info._p_current_frame_index], 0);
        assert(VK_SUCCESS == res_reset_command_pool);

        VkCommandBufferBeginInfo command_buffer_begin_info {};
        command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags            = 0;
        command_buffer_begin_info.pInheritanceInfo = nullptr;

        VkResult res_begin_command_buffer = m_p_vulkan_context->_vkBeginCommandBuffer(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            &command_buffer_begin_info);
        assert(VK_SUCCESS == res_begin_command_buffer);

        {
            VkImageMemoryBarrier transfer_to_render_barrier {};
            transfer_to_render_barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            transfer_to_render_barrier.pNext               = nullptr;
            transfer_to_render_barrier.srcAccessMask       = 0;
            transfer_to_render_barrier.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            transfer_to_render_barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            transfer_to_render_barrier.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            transfer_to_render_barrier.srcQueueFamilyIndex = m_p_vulkan_context->_queue_indices.graphicsFamily.value();
            transfer_to_render_barrier.dstQueueFamilyIndex = m_p_vulkan_context->_queue_indices.graphicsFamily.value();
            transfer_to_render_barrier.image               = _framebuffer.attachments[0].image;
            transfer_to_render_barrier.subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            vkCmdPipelineBarrier(m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &transfer_to_render_barrier);
        }

        m_p_vulkan_context->_vkCmdSetViewport(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            0,
            1,
            &m_command_info._viewport);
        m_p_vulkan_context->_vkCmdSetScissor(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            0,
            1,
            &m_command_info._scissor);

        VkRenderPassBeginInfo renderpass_begin_info {};
        renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderPass        = _framebuffer.render_pass;
        renderpass_begin_info.framebuffer       = _framebuffer.framebuffer;
        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = m_p_vulkan_context->_swapchain_extent;

        VkClearColorValue color_value         = {0, 0, 0, 0};
        VkClearValue      clearValues[2]      = {color_value, {1.0f, 0}};
        renderpass_begin_info.clearValueCount = 2;
        renderpass_begin_info.pClearValues    = clearValues;

        m_p_vulkan_context->_vkCmdBeginRenderPass(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            &renderpass_begin_info,
            VK_SUBPASS_CONTENTS_INLINE); // no second buffer

        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Mesh Inefficient Pick", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(
                m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index], &label_info);
        }

        m_p_vulkan_context->_vkCmdBindPipeline(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            _render_pipelines[0].pipeline);
        m_p_vulkan_context->_vkCmdSetViewport(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            0,
            1,
            &m_command_info._viewport);
        m_p_vulkan_context->_vkCmdSetScissor(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
            0,
            1,
            &m_command_info._scissor);

        // perframe storage buffer
        uint32_t perframe_dynamic_offset =
            roundUp(m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index],
                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
        m_p_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] =
            perframe_dynamic_offset + sizeof(MeshInefficientPickPerframeStorageBufferObject);
        assert(m_p_global_render_resource->_storage_buffer
                   ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] <=
               (m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_begin[*m_command_info._p_current_frame_index] +
                m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_size[*m_command_info._p_current_frame_index]));

        (*reinterpret_cast<MeshInefficientPickPerframeStorageBufferObject*>(
            reinterpret_cast<uintptr_t>(
                m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
            perframe_dynamic_offset)) = _mesh_inefficient_pick_perframe_storage_buffer_object;

        for (auto& pair1 : main_camera_mesh_drawcall_batch)
        {
            VulkanPBRMaterial& material       = (*pair1.first);
            auto&              mesh_instanced = pair1.second;

            // TODO: render from near to far

            for (auto& pair2 : mesh_instanced)
            {
                VulkanMesh& mesh       = (*pair2.first);
                auto&       mesh_nodes = pair2.second;

                uint32_t total_instance_count = static_cast<uint32_t>(mesh_nodes.size());
                if (total_instance_count > 0)
                {
                    // bind per mesh
                    m_p_vulkan_context->_vkCmdBindDescriptorSets(
                        m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        _render_pipelines[0].layout,
                        1,
                        1,
                        &mesh.mesh_vertex_blending_descriptor_set,
                        0,
                        NULL);

                    VkBuffer     vertex_buffers[] = {mesh.mesh_vertex_position_buffer};
                    VkDeviceSize offsets[]        = {0};
                    m_p_vulkan_context->_vkCmdBindVertexBuffers(
                        m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
                        0,
                        1,
                        vertex_buffers,
                        offsets);
                    m_p_vulkan_context->_vkCmdBindIndexBuffer(
                        m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
                        mesh.mesh_index_buffer,
                        0,
                        VK_INDEX_TYPE_UINT16);

                    uint32_t drawcall_max_instance_count =
                        (sizeof(MeshInefficientPickPerdrawcallStorageBufferObject::model_matrices) /
                         sizeof(MeshInefficientPickPerdrawcallStorageBufferObject::model_matrices[0]));
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
                            roundUp(m_p_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index],
                                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                        m_p_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] =
                            perdrawcall_dynamic_offset + sizeof(MeshInefficientPickPerdrawcallStorageBufferObject);
                        assert(m_p_global_render_resource->_storage_buffer
                                   ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] <=
                               (m_p_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_begin[*m_command_info._p_current_frame_index] +
                                m_p_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_size[*m_command_info._p_current_frame_index]));

                        MeshInefficientPickPerdrawcallStorageBufferObject& perdrawcall_storage_buffer_object =
                            (*reinterpret_cast<MeshInefficientPickPerdrawcallStorageBufferObject*>(
                                reinterpret_cast<uintptr_t>(m_p_global_render_resource->_storage_buffer
                                                                ._global_upload_ringbuffer_memory_pointer) +
                                perdrawcall_dynamic_offset));
                        for (uint32_t i = 0; i < current_instance_count; ++i)
                        {
                            perdrawcall_storage_buffer_object.model_matrices[i] =
                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i].model_matrix;
                            perdrawcall_storage_buffer_object.node_ids[i] =
                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i].node_id;
                        }

                        // per drawcall vertex blending storage buffer
                        uint32_t per_drawcall_vertex_blending_dynamic_offset;
                        if (mesh.enable_vertex_blending)
                        {
                            per_drawcall_vertex_blending_dynamic_offset = roundUp(
                                m_p_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index],
                                m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                            m_p_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] =
                                per_drawcall_vertex_blending_dynamic_offset +
                                sizeof(MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject);
                            assert(m_p_global_render_resource->_storage_buffer
                                       ._global_upload_ringbuffers_end[*m_command_info._p_current_frame_index] <=
                                   (m_p_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_begin[*m_command_info._p_current_frame_index] +
                                    m_p_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_size[*m_command_info._p_current_frame_index]));

                            MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject&
                                per_drawcall_vertex_blending_storage_buffer_object =
                                    (*reinterpret_cast<
                                        MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject*>(
                                        reinterpret_cast<uintptr_t>(m_p_global_render_resource->_storage_buffer
                                                                        ._global_upload_ringbuffer_memory_pointer) +
                                        per_drawcall_vertex_blending_dynamic_offset));
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                for (uint32_t j = 0; j < m_mesh_vertex_blending_max_joint_count; ++j)
                                {
                                    per_drawcall_vertex_blending_storage_buffer_object
                                        .joint_matrices[m_mesh_vertex_blending_max_joint_count * i + j] =
                                        mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices[j];
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
                        m_p_vulkan_context->_vkCmdBindDescriptorSets(
                            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _render_pipelines[0].layout,
                            0,
                            1,
                            &_descriptor_infos[0].descriptor_set,
                            sizeof(dynamic_offsets) / sizeof(dynamic_offsets[0]),
                            dynamic_offsets);

                        m_p_vulkan_context->_vkCmdDrawIndexed(
                            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index],
                            mesh.mesh_index_count,
                            current_instance_count,
                            0,
                            0,
                            0);
                    }
                }
            }
        }

        if (m_render_config._enable_debug_untils_label)
        {
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(
                m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index]);
        }

        // end render pass
        m_p_vulkan_context->_vkCmdEndRenderPass(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index]);

        // end command buffer
        VkResult res_end_command_buffer = m_p_vulkan_context->_vkEndCommandBuffer(
            m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index]);
        assert(VK_SUCCESS == res_end_command_buffer);

        VkResult res_reset_fences = m_p_vulkan_context->_vkResetFences(
            m_p_vulkan_context->_device,
            1,
            &m_command_info._is_frame_in_flight_fences[*m_command_info._p_current_frame_index]);
        assert(VK_SUCCESS == res_reset_fences);

        VkSubmitInfo submit_info       = {};
        submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores    = NULL;
        submit_info.pWaitDstStageMask  = 0;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_info._p_current_command_buffer[*m_command_info._p_current_frame_index];
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores    = NULL;

        VkResult res_queue_submit =
            vkQueueSubmit(m_p_vulkan_context->_graphics_queue,
                          1,
                          &submit_info,
                          m_command_info._is_frame_in_flight_fences[*m_command_info._p_current_frame_index]);
        assert(VK_SUCCESS == res_queue_submit);

        auto new_index = (*m_command_info._p_current_frame_index + 1) % m_command_info._max_frames_in_flight;
        *m_command_info._p_current_frame_index = new_index;

        // implicit host read barrier
        res_wait_for_fences = m_p_vulkan_context->_vkWaitForFences(m_p_vulkan_context->_device,
                                                                   m_command_info._max_frames_in_flight,
                                                                   m_command_info._is_frame_in_flight_fences,
                                                                   VK_TRUE,
                                                                   UINT64_MAX);
        assert(VK_SUCCESS == res_wait_for_fences);

        auto command_buffer = m_p_vulkan_context->beginSingleTimeCommands();

        VkBufferImageCopy region {};
        region.bufferOffset                    = 0;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {
            m_p_vulkan_context->_swapchain_extent.width, m_p_vulkan_context->_swapchain_extent.height, 1};

        uint32_t buffer_size =
            m_p_vulkan_context->_swapchain_extent.width * m_p_vulkan_context->_swapchain_extent.height * 4;
        VkBuffer       inefficient_staging_buffer;
        VkDeviceMemory inefficient_staging_buffer_memory;
        PVulkanUtil::createBuffer(m_p_vulkan_context->_physical_device,
                                  m_p_vulkan_context->_device,
                                  buffer_size,
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  inefficient_staging_buffer,
                                  inefficient_staging_buffer_memory);

        VkImageMemoryBarrier copy_to_buffer_barrier {};
        copy_to_buffer_barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_to_buffer_barrier.pNext               = nullptr;
        copy_to_buffer_barrier.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        copy_to_buffer_barrier.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
        copy_to_buffer_barrier.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        copy_to_buffer_barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        copy_to_buffer_barrier.srcQueueFamilyIndex = m_p_vulkan_context->_queue_indices.graphicsFamily.value();
        copy_to_buffer_barrier.dstQueueFamilyIndex = m_p_vulkan_context->_queue_indices.graphicsFamily.value();
        copy_to_buffer_barrier.image               = _framebuffer.attachments[0].image;
        copy_to_buffer_barrier.subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &copy_to_buffer_barrier);

        vkCmdCopyImageToBuffer(command_buffer,
                               _framebuffer.attachments[0].image,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               inefficient_staging_buffer,
                               1,
                               &region);

        m_p_vulkan_context->endSingleTimeCommands(command_buffer);

        uint32_t* data = nullptr;
        vkMapMemory(m_p_vulkan_context->_device, inefficient_staging_buffer_memory, 0, buffer_size, 0, (void**)&data);

#if 0
        auto                 w = m_p_vulkan_context->_swapchain_extent.width;
        auto                 h = m_p_vulkan_context->_swapchain_extent.height;
        std::vector<uint8_t> image_data(w * h * 4);
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                image_data[4 * (i * w + j) + 0] = data[i * w + j];
                image_data[4 * (i * w + j) + 1] = 0;
                image_data[4 * (i * w + j) + 2] = 0;
                image_data[4 * (i * w + j) + 3] = 0xFF;
            }
        }
        stbi_write_png("pick.png", w, h, 4, image_data.data(), w * 4);
#endif

        uint32_t node_id = data[picked_pixel_index];
        vkUnmapMemory(m_p_vulkan_context->_device, inefficient_staging_buffer_memory);

        vkDestroyBuffer(m_p_vulkan_context->_device, inefficient_staging_buffer, nullptr);
        vkFreeMemory(m_p_vulkan_context->_device, inefficient_staging_buffer_memory, nullptr);

        return node_id;
    }
} // namespace Pilot
