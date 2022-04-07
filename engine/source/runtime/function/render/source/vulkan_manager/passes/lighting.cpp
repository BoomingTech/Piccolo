#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_mesh.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_misc.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_passes.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_util.h"

#include <map>
#include <stdexcept>

#include <axis_frag.h>
#include <axis_vert.h>
#include <mesh_frag.h>
#include <mesh_vert.h>
#include <particlebillboard_frag.h>
#include <particlebillboard_vert.h>
#include <skybox_frag.h>
#include <skybox_vert.h>

namespace Pilot
{
    void PLightingPass::initialize()
    {
        PRenderPassBase::initialize();
        setupRenderPass();
        setupDescriptorSetLayout();
        setupPipelines();
        setupDescriptorSet();
    }
    void PLightingPass::draw()
    {
        // lighting pass start
        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Lighting", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(m_command_info._current_command_buffer, &label_info);
        }

        drawModel();
        drawAxis();
        drawSkybox();
        drawBillboardParticle();

        // lighting pass end
        if (m_render_config._enable_debug_untils_label)
        {
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(m_command_info._current_command_buffer);
        }
    }
    void PLightingPass::setHelperInfo(const PLightPassHelperInfo& helper_info)
    {
        m_point_light_shadow_color_image_view       = helper_info.point_light_shadow_color_image_view;
        m_directional_light_shadow_color_image_view = helper_info.directional_light_shadow_color_image_view;
    }
    void PLightingPass::setupRenderPass()
    {
        VkAttachmentDescription attachments[3] = {};

        VkAttachmentDescription& backup_color_attachment_description = attachments[0];
        backup_color_attachment_description.format                   = m_p_vulkan_context->_swapchain_image_format;
        backup_color_attachment_description.samples                  = VK_SAMPLE_COUNT_1_BIT;
        backup_color_attachment_description.loadOp                   = VK_ATTACHMENT_LOAD_OP_CLEAR;
        backup_color_attachment_description.storeOp                  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backup_color_attachment_description.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        backup_color_attachment_description.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backup_color_attachment_description.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        backup_color_attachment_description.finalLayout              = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& depth_attachment_description = attachments[1];
        depth_attachment_description.format                   = m_p_vulkan_context->_depth_image_format;
        depth_attachment_description.samples                  = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment_description.loadOp                   = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_description.storeOp                  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment_description.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment_description.finalLayout              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription& swapchain_image_attachment_description = attachments[2];
        swapchain_image_attachment_description.format                   = m_p_vulkan_context->_swapchain_image_format;
        swapchain_image_attachment_description.samples                  = VK_SAMPLE_COUNT_1_BIT;
        swapchain_image_attachment_description.loadOp                   = VK_ATTACHMENT_LOAD_OP_CLEAR;
        swapchain_image_attachment_description.storeOp                  = VK_ATTACHMENT_STORE_OP_STORE;
        swapchain_image_attachment_description.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        swapchain_image_attachment_description.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        swapchain_image_attachment_description.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        swapchain_image_attachment_description.finalLayout              = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkSubpassDescription subpasses[2] = {};

        VkAttachmentReference lighting_pass_color_attachment_reference {};
        lighting_pass_color_attachment_reference.attachment = &backup_color_attachment_description - attachments;
        lighting_pass_color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference lighting_pass_depth_attachment_reference {};
        lighting_pass_depth_attachment_reference.attachment = &depth_attachment_description - attachments;
        lighting_pass_depth_attachment_reference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& lighting_pass   = subpasses[0];
        lighting_pass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        lighting_pass.colorAttachmentCount    = 1;
        lighting_pass.pColorAttachments       = &lighting_pass_color_attachment_reference;
        lighting_pass.pDepthStencilAttachment = &lighting_pass_depth_attachment_reference;
        lighting_pass.preserveAttachmentCount = 0;
        lighting_pass.pPreserveAttachments    = NULL;

        VkAttachmentReference postprocess_pass_input_attachment_reference {};
        postprocess_pass_input_attachment_reference.attachment = &backup_color_attachment_description - attachments;
        postprocess_pass_input_attachment_reference.layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference postprocess_pass_color_attachment_reference {};
        postprocess_pass_color_attachment_reference.attachment = &swapchain_image_attachment_description - attachments;
        postprocess_pass_color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& postprocess_pass   = subpasses[1];
        postprocess_pass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        postprocess_pass.inputAttachmentCount    = 1;
        postprocess_pass.pInputAttachments       = &postprocess_pass_input_attachment_reference;
        postprocess_pass.colorAttachmentCount    = 1;
        postprocess_pass.pColorAttachments       = &postprocess_pass_color_attachment_reference;
        postprocess_pass.pDepthStencilAttachment = NULL;
        postprocess_pass.preserveAttachmentCount = 0;
        postprocess_pass.pPreserveAttachments    = NULL;

        VkSubpassDependency dependencies[2] = {};

        VkSubpassDependency& lighting_pass_dependency = dependencies[0];
        lighting_pass_dependency.srcSubpass           = VK_SUBPASS_EXTERNAL;
        lighting_pass_dependency.dstSubpass           = 0;
        lighting_pass_dependency.srcStageMask         = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        lighting_pass_dependency.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        lighting_pass_dependency.srcAccessMask        = 0;
        lighting_pass_dependency.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT;
        lighting_pass_dependency.dependencyFlags      = 0; // NOT BY REGION

        VkSubpassDependency& postprocess_pass_dependency = dependencies[1];
        postprocess_pass_dependency.srcSubpass           = 0;
        postprocess_pass_dependency.dstSubpass           = 1;
        postprocess_pass_dependency.srcStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        postprocess_pass_dependency.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        postprocess_pass_dependency.srcAccessMask        = 0;
        postprocess_pass_dependency.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT;
        postprocess_pass_dependency.dependencyFlags      = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_create_info {};
        renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        renderpass_create_info.pAttachments    = attachments;
        renderpass_create_info.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
        renderpass_create_info.pSubpasses      = subpasses;
        renderpass_create_info.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
        renderpass_create_info.pDependencies   = dependencies;

        if (vkCreateRenderPass(
                m_p_vulkan_context->_device, &renderpass_create_info, nullptr, &_framebuffer.render_pass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass");
        }
    }
    void PLightingPass::setupDescriptorSetLayout()
    {
        _descriptor_infos.resize(_layout_type_count);

        {
            VkDescriptorSetLayoutBinding mesh_mesh_layout_bindings[1];

            VkDescriptorSetLayoutBinding& mesh_mesh_layout_uniform_buffer_binding = mesh_mesh_layout_bindings[0];
            mesh_mesh_layout_uniform_buffer_binding.binding                       = 0;
            mesh_mesh_layout_uniform_buffer_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_mesh_layout_uniform_buffer_binding.descriptorCount               = 1;
            mesh_mesh_layout_uniform_buffer_binding.stageFlags                    = VK_SHADER_STAGE_VERTEX_BIT;
            mesh_mesh_layout_uniform_buffer_binding.pImmutableSamplers            = NULL;

            VkDescriptorSetLayoutCreateInfo mesh_mesh_layout_create_info {};
            mesh_mesh_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_mesh_layout_create_info.bindingCount = 1;
            mesh_mesh_layout_create_info.pBindings    = mesh_mesh_layout_bindings;

            if (vkCreateDescriptorSetLayout(m_p_vulkan_context->_device,
                                            &mesh_mesh_layout_create_info,
                                            NULL,
                                            &_descriptor_infos[_per_mesh].layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create mesh mesh layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding mesh_global_layout_bindings[8];

            VkDescriptorSetLayoutBinding& mesh_global_layout_perframe_storage_buffer_binding =
                mesh_global_layout_bindings[0];
            mesh_global_layout_perframe_storage_buffer_binding.binding = 0;
            mesh_global_layout_perframe_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            mesh_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            mesh_global_layout_perframe_storage_buffer_binding.stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_global_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_perdrawcall_storage_buffer_binding =
                mesh_global_layout_bindings[1];
            mesh_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
            mesh_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            mesh_global_layout_perdrawcall_storage_buffer_binding.descriptorCount    = 1;
            mesh_global_layout_perdrawcall_storage_buffer_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
            mesh_global_layout_perdrawcall_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding =
                mesh_global_layout_bindings[2];
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.binding = 2;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.descriptorCount = 1;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_brdfLUT_texture_binding = mesh_global_layout_bindings[3];
            mesh_global_layout_brdfLUT_texture_binding.binding                       = 3;
            mesh_global_layout_brdfLUT_texture_binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            mesh_global_layout_brdfLUT_texture_binding.descriptorCount    = 1;
            mesh_global_layout_brdfLUT_texture_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_global_layout_brdfLUT_texture_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_irradiance_texture_binding =
                mesh_global_layout_bindings[4];
            mesh_global_layout_irradiance_texture_binding         = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_irradiance_texture_binding.binding = 4;

            VkDescriptorSetLayoutBinding& mesh_global_layout_specular_texture_binding = mesh_global_layout_bindings[5];
            mesh_global_layout_specular_texture_binding         = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_specular_texture_binding.binding = 5;

            VkDescriptorSetLayoutBinding& mesh_global_layout_point_light_shadow_texture_binding =
                mesh_global_layout_bindings[6];
            mesh_global_layout_point_light_shadow_texture_binding         = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_point_light_shadow_texture_binding.binding = 6;

            VkDescriptorSetLayoutBinding& mesh_global_layout_directional_light_shadow_texture_binding =
                mesh_global_layout_bindings[7];
            mesh_global_layout_directional_light_shadow_texture_binding = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_directional_light_shadow_texture_binding.binding = 7;

            VkDescriptorSetLayoutCreateInfo mesh_global_layout_create_info;
            mesh_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_global_layout_create_info.pNext = NULL;
            mesh_global_layout_create_info.flags = 0;
            mesh_global_layout_create_info.bindingCount =
                (sizeof(mesh_global_layout_bindings) / sizeof(mesh_global_layout_bindings[0]));
            mesh_global_layout_create_info.pBindings = mesh_global_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_p_vulkan_context->_device,
                                                          &mesh_global_layout_create_info,
                                                          NULL,
                                                          &_descriptor_infos[_global].layout))
            {
                throw std::runtime_error("create mesh global layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding mesh_material_layout_bindings[6];

            // (set = 2, binding = 0 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_uniform_buffer_binding =
                mesh_material_layout_bindings[0];
            mesh_material_layout_uniform_buffer_binding.binding            = 0;
            mesh_material_layout_uniform_buffer_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            mesh_material_layout_uniform_buffer_binding.descriptorCount    = 1;
            mesh_material_layout_uniform_buffer_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_material_layout_uniform_buffer_binding.pImmutableSamplers = nullptr;

            // (set = 2, binding = 1 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_base_color_texture_binding =
                mesh_material_layout_bindings[1];
            mesh_material_layout_base_color_texture_binding.binding         = 1;
            mesh_material_layout_base_color_texture_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            mesh_material_layout_base_color_texture_binding.descriptorCount = 1;
            mesh_material_layout_base_color_texture_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_material_layout_base_color_texture_binding.pImmutableSamplers = nullptr;

            // (set = 2, binding = 2 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_metallic_roughness_texture_binding =
                mesh_material_layout_bindings[2];
            mesh_material_layout_metallic_roughness_texture_binding = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_metallic_roughness_texture_binding.binding = 2;

            // (set = 2, binding = 3 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_normal_roughness_texture_binding =
                mesh_material_layout_bindings[3];
            mesh_material_layout_normal_roughness_texture_binding = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_normal_roughness_texture_binding.binding = 3;

            // (set = 2, binding = 4 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_occlusion_texture_binding =
                mesh_material_layout_bindings[4];
            mesh_material_layout_occlusion_texture_binding         = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_occlusion_texture_binding.binding = 4;

            // (set = 2, binding = 5 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_emissive_texture_binding =
                mesh_material_layout_bindings[5];
            mesh_material_layout_emissive_texture_binding         = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_emissive_texture_binding.binding = 5;

            VkDescriptorSetLayoutCreateInfo mesh_material_layout_create_info;
            mesh_material_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_material_layout_create_info.pNext        = NULL;
            mesh_material_layout_create_info.flags        = 0;
            mesh_material_layout_create_info.bindingCount = 6;
            mesh_material_layout_create_info.pBindings    = mesh_material_layout_bindings;

            if (vkCreateDescriptorSetLayout(m_p_vulkan_context->_device,
                                            &mesh_material_layout_create_info,
                                            nullptr,
                                            &_descriptor_infos[_material].layout) != VK_SUCCESS)

            {
                throw std::runtime_error("create mesh material layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding skybox_layout_bindings[2];

            VkDescriptorSetLayoutBinding& skybox_layout_perframe_storage_buffer_binding = skybox_layout_bindings[0];
            skybox_layout_perframe_storage_buffer_binding.binding                       = 0;
            skybox_layout_perframe_storage_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            skybox_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            skybox_layout_perframe_storage_buffer_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
            skybox_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& skybox_layout_specular_texture_binding = skybox_layout_bindings[1];
            skybox_layout_specular_texture_binding.binding                       = 1;
            skybox_layout_specular_texture_binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            skybox_layout_specular_texture_binding.descriptorCount    = 1;
            skybox_layout_specular_texture_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            skybox_layout_specular_texture_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutCreateInfo skybox_layout_create_info {};
            skybox_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            skybox_layout_create_info.bindingCount = 2;
            skybox_layout_create_info.pBindings    = skybox_layout_bindings;

            if (VK_SUCCESS !=
                vkCreateDescriptorSetLayout(
                    m_p_vulkan_context->_device, &skybox_layout_create_info, NULL, &_descriptor_infos[_skybox].layout))
            {
                throw std::runtime_error("create skybox layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding axis_layout_bindings[2];

            VkDescriptorSetLayoutBinding& axis_layout_perframe_storage_buffer_binding = axis_layout_bindings[0];
            axis_layout_perframe_storage_buffer_binding.binding                       = 0;
            axis_layout_perframe_storage_buffer_binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            axis_layout_perframe_storage_buffer_binding.descriptorCount    = 1;
            axis_layout_perframe_storage_buffer_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
            axis_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& axis_layout_storage_buffer_binding = axis_layout_bindings[1];
            axis_layout_storage_buffer_binding.binding                       = 1;
            axis_layout_storage_buffer_binding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            axis_layout_storage_buffer_binding.descriptorCount               = 1;
            axis_layout_storage_buffer_binding.stageFlags                    = VK_SHADER_STAGE_VERTEX_BIT;
            axis_layout_storage_buffer_binding.pImmutableSamplers            = NULL;

            VkDescriptorSetLayoutCreateInfo axis_layout_create_info {};
            axis_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            axis_layout_create_info.bindingCount = 2;
            axis_layout_create_info.pBindings    = axis_layout_bindings;

            if (VK_SUCCESS !=
                vkCreateDescriptorSetLayout(
                    m_p_vulkan_context->_device, &axis_layout_create_info, NULL, &_descriptor_infos[_axis].layout))
            {
                throw std::runtime_error("create axis layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding particlebillboard_global_layout_bindings[2];

            VkDescriptorSetLayoutBinding& particlebillboard_global_layout_perframe_storage_buffer_binding =
                particlebillboard_global_layout_bindings[0];
            particlebillboard_global_layout_perframe_storage_buffer_binding.binding = 0;
            particlebillboard_global_layout_perframe_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            particlebillboard_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            particlebillboard_global_layout_perframe_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            particlebillboard_global_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& particlebillboard_global_layout_perdrawcall_storage_buffer_binding =
                particlebillboard_global_layout_bindings[1];
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            particlebillboard_global_layout_perdrawcall_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutCreateInfo particlebillboard_global_layout_create_info;
            particlebillboard_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            particlebillboard_global_layout_create_info.pNext = NULL;
            particlebillboard_global_layout_create_info.flags = 0;
            particlebillboard_global_layout_create_info.bindingCount = 2;
            particlebillboard_global_layout_create_info.pBindings    = particlebillboard_global_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_p_vulkan_context->_device,
                                                          &particlebillboard_global_layout_create_info,
                                                          NULL,
                                                          &_descriptor_infos[_particle].layout))
            {
                throw std::runtime_error("create particle billboard global layout");
            }
        }
    }
    void PLightingPass::setupPipelines()
    {
        _render_pipelines.resize(_render_pipeline_type_count);
        // mesh
        {
            VkDescriptorSetLayout      descriptorset_layouts[3] = {_descriptor_infos[_global].layout,
                                                              _descriptor_infos[_per_mesh].layout,
                                                              _descriptor_infos[_material].layout};
            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 3;
            pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_p_vulkan_context->_device,
                                       &pipeline_layout_create_info,
                                       nullptr,
                                       &_render_pipelines[_render_pipeline_type_model].layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create mesh pipeline layout");
            }

            VkShaderModule vert_shader_module = PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, MESH_VERT);
            VkShaderModule frag_shader_module = PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, MESH_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
            vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName  = "main";
            // vert_pipeline_shader_stage_create_info.pSpecializationInfo

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
            frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName  = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info};

            auto vertex_binding_descriptions   = PMeshVertex::getBindingDescriptions();
            auto vertex_attribute_descriptions = PMeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount   = vertex_binding_descriptions.size();
            vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_binding_descriptions[0];
            vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
            vertex_input_state_create_info.pVertexAttributeDescriptions    = &vertex_attribute_descriptions[0];

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
            input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info {};
            viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports    = &m_command_info._viewport;
            viewport_state_create_info.scissorCount  = 1;
            viewport_state_create_info.pScissors     = &m_command_info._scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable        = VK_FALSE;
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
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
            depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable  = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS;
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
            pipelineInfo.layout              = _render_pipelines[_render_pipeline_type_model].layout;
            pipelineInfo.renderPass          = _framebuffer.render_pass;
            pipelineInfo.subpass             = 0;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState       = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &_render_pipelines[_render_pipeline_type_model].pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create mesh graphics pipeline");
            }

            vkDestroyShaderModule(m_p_vulkan_context->_device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_p_vulkan_context->_device, frag_shader_module, nullptr);
        }

        // skybox
        {
            VkDescriptorSetLayout      descriptorset_layouts[1] = {_descriptor_infos[_skybox].layout};
            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_p_vulkan_context->_device,
                                       &pipeline_layout_create_info,
                                       nullptr,
                                       &_render_pipelines[_render_pipeline_type_skybox].layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create skybox pipeline layout");
            }

            VkShaderModule vert_shader_module =
                PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, SKYBOX_VERT);
            VkShaderModule frag_shader_module =
                PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, SKYBOX_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
            vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName  = "main";
            // vert_pipeline_shader_stage_create_info.pSpecializationInfo

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
            frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName  = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info};

            auto vertex_binding_descriptions   = PMeshVertex::getBindingDescriptions();
            auto vertex_attribute_descriptions = PMeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
            vertex_input_state_create_info.pVertexBindingDescriptions      = NULL;
            vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
            vertex_input_state_create_info.pVertexAttributeDescriptions    = NULL;

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
            input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info {};
            viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports    = &m_command_info._viewport;
            viewport_state_create_info.scissorCount  = 1;
            viewport_state_create_info.pScissors     = &m_command_info._scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable        = VK_FALSE;
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
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
            depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable  = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS;
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
            pipelineInfo.layout              = _render_pipelines[_render_pipeline_type_skybox].layout;
            pipelineInfo.renderPass          = _framebuffer.render_pass;
            pipelineInfo.subpass             = 0;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState       = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &_render_pipelines[_render_pipeline_type_skybox].pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create skybox graphics pipeline");
            }

            vkDestroyShaderModule(m_p_vulkan_context->_device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_p_vulkan_context->_device, frag_shader_module, nullptr);
        }

        // draw axis
        {
            VkDescriptorSetLayout      descriptorset_layouts[1] = {_descriptor_infos[_axis].layout};
            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_p_vulkan_context->_device,
                                       &pipeline_layout_create_info,
                                       nullptr,
                                       &_render_pipelines[_render_pipeline_type_axis].layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create axis pipeline layout");
            }

            VkShaderModule vert_shader_module = PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, AXIS_VERT);
            VkShaderModule frag_shader_module = PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, AXIS_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
            vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName  = "main";
            // vert_pipeline_shader_stage_create_info.pSpecializationInfo

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
            frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName  = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info};

            auto vertex_binding_descriptions   = PMeshVertex::getBindingDescriptions();
            auto vertex_attribute_descriptions = PMeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount   = vertex_binding_descriptions.size();
            vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_binding_descriptions[0];
            vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
            vertex_input_state_create_info.pVertexAttributeDescriptions    = &vertex_attribute_descriptions[0];

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
            input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info {};
            viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports    = &m_command_info._viewport;
            viewport_state_create_info.scissorCount  = 1;
            viewport_state_create_info.pScissors     = &m_command_info._scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable        = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth               = 1.0f;
            rasterization_state_create_info.cullMode                = VK_CULL_MODE_NONE;
            rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp          = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
            depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable  = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS;
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
            pipelineInfo.layout              = _render_pipelines[_render_pipeline_type_axis].layout;
            pipelineInfo.renderPass          = _framebuffer.render_pass;
            pipelineInfo.subpass             = 0;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState       = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &_render_pipelines[_render_pipeline_type_axis].pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create axis graphics pipeline");
            }

            vkDestroyShaderModule(m_p_vulkan_context->_device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_p_vulkan_context->_device, frag_shader_module, nullptr);
        }

        // particle billboard
        {
            VkDescriptorSetLayout      descriptorset_layouts[1] = {_descriptor_infos[_particle].layout};
            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_p_vulkan_context->_device,
                                       &pipeline_layout_create_info,
                                       nullptr,
                                       &_render_pipelines[_render_pipeline_type_particle].layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create particle billboard pipeline layout");
            }

            VkShaderModule vert_shader_module =
                PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, PARTICLEBILLBOARD_VERT);
            VkShaderModule frag_shader_module =
                PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, PARTICLEBILLBOARD_FRAG);

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
            input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info {};
            viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports    = &m_command_info._viewport;
            viewport_state_create_info.scissorCount  = 1;
            viewport_state_create_info.pScissors     = &m_command_info._scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable        = VK_FALSE;
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
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
            depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable  = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS;
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
            pipelineInfo.layout              = _render_pipelines[_render_pipeline_type_particle].layout;
            pipelineInfo.renderPass          = _framebuffer.render_pass;
            pipelineInfo.subpass             = 0;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState       = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &_render_pipelines[_render_pipeline_type_particle].pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create particle billboard graphics pipeline");
            }

            vkDestroyShaderModule(m_p_vulkan_context->_device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_p_vulkan_context->_device, frag_shader_module, nullptr);
        }
    }
    void PLightingPass::setupDescriptorSet()
    {
        setupModelGlobalDescriptorSet();
        setupSkyboxDescriptorSet();
        setupAxisDescriptorSet();
        setupParticleDescriptorSet();
    }
    void PLightingPass::setupModelGlobalDescriptorSet()
    {
        // update common model's global descriptor set
        VkDescriptorSetAllocateInfo mesh_global_descriptor_set_alloc_info;
        mesh_global_descriptor_set_alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_global_descriptor_set_alloc_info.pNext              = NULL;
        mesh_global_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        mesh_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_global_descriptor_set_alloc_info.pSetLayouts        = &_descriptor_infos[_global].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_p_vulkan_context->_device,
                                                   &mesh_global_descriptor_set_alloc_info,
                                                   &_descriptor_infos[_global].descriptor_set))
        {
            throw std::runtime_error("allocate mesh global descriptor set");
        }

        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of the buffer
        mesh_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_perdrawcall_storage_buffer_info = {};
        mesh_perdrawcall_storage_buffer_info.offset                 = 0;
        mesh_perdrawcall_storage_buffer_info.range                  = sizeof(MeshPerdrawcallStorageBufferObject);
        mesh_perdrawcall_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perdrawcall_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_per_drawcall_vertex_blending_storage_buffer_info = {};
        mesh_per_drawcall_vertex_blending_storage_buffer_info.offset                 = 0;
        mesh_per_drawcall_vertex_blending_storage_buffer_info.range =
            sizeof(MeshPerdrawcallVertexBlendingStorageBufferObject);
        mesh_per_drawcall_vertex_blending_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_per_drawcall_vertex_blending_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorImageInfo brdf_texture_image_info = {};
        brdf_texture_image_info.sampler     = m_p_global_render_resource->_ibl_resource._brdfLUT_texture_sampler;
        brdf_texture_image_info.imageView   = m_p_global_render_resource->_ibl_resource._brdfLUT_texture_image_view;
        brdf_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo irradiance_texture_image_info = {};
        irradiance_texture_image_info.sampler = m_p_global_render_resource->_ibl_resource._irradiance_texture_sampler;
        irradiance_texture_image_info.imageView =
            m_p_global_render_resource->_ibl_resource._irradiance_texture_image_view;
        irradiance_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo specular_texture_image_info {};
        specular_texture_image_info.sampler   = m_p_global_render_resource->_ibl_resource._specular_texture_sampler;
        specular_texture_image_info.imageView = m_p_global_render_resource->_ibl_resource._specular_texture_image_view;
        specular_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo point_light_shadow_texture_image_info {};
        point_light_shadow_texture_image_info.sampler =
            PVulkanUtil::getOrCreateNearestSampler(m_p_vulkan_context->_physical_device, m_p_vulkan_context->_device);
        point_light_shadow_texture_image_info.imageView   = m_point_light_shadow_color_image_view;
        point_light_shadow_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo directional_light_shadow_texture_image_info {};
        directional_light_shadow_texture_image_info.sampler =
            PVulkanUtil::getOrCreateNearestSampler(m_p_vulkan_context->_physical_device, m_p_vulkan_context->_device);
        directional_light_shadow_texture_image_info.imageView   = m_directional_light_shadow_color_image_view;
        directional_light_shadow_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet mesh_descriptor_writes_info[8];

        mesh_descriptor_writes_info[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[0].pNext           = NULL;
        mesh_descriptor_writes_info[0].dstSet          = _descriptor_infos[_global].descriptor_set;
        mesh_descriptor_writes_info[0].dstBinding      = 0;
        mesh_descriptor_writes_info[0].dstArrayElement = 0;
        mesh_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[0].descriptorCount = 1;
        mesh_descriptor_writes_info[0].pBufferInfo     = &mesh_perframe_storage_buffer_info;

        mesh_descriptor_writes_info[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[1].pNext           = NULL;
        mesh_descriptor_writes_info[1].dstSet          = _descriptor_infos[_global].descriptor_set;
        mesh_descriptor_writes_info[1].dstBinding      = 1;
        mesh_descriptor_writes_info[1].dstArrayElement = 0;
        mesh_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[1].descriptorCount = 1;
        mesh_descriptor_writes_info[1].pBufferInfo     = &mesh_perdrawcall_storage_buffer_info;

        mesh_descriptor_writes_info[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[2].pNext           = NULL;
        mesh_descriptor_writes_info[2].dstSet          = _descriptor_infos[_global].descriptor_set;
        mesh_descriptor_writes_info[2].dstBinding      = 2;
        mesh_descriptor_writes_info[2].dstArrayElement = 0;
        mesh_descriptor_writes_info[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[2].descriptorCount = 1;
        mesh_descriptor_writes_info[2].pBufferInfo     = &mesh_per_drawcall_vertex_blending_storage_buffer_info;

        mesh_descriptor_writes_info[3].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[3].pNext           = NULL;
        mesh_descriptor_writes_info[3].dstSet          = _descriptor_infos[_global].descriptor_set;
        mesh_descriptor_writes_info[3].dstBinding      = 3;
        mesh_descriptor_writes_info[3].dstArrayElement = 0;
        mesh_descriptor_writes_info[3].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mesh_descriptor_writes_info[3].descriptorCount = 1;
        mesh_descriptor_writes_info[3].pImageInfo      = &brdf_texture_image_info;

        mesh_descriptor_writes_info[4]            = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[4].dstBinding = 4;
        mesh_descriptor_writes_info[4].pImageInfo = &irradiance_texture_image_info;

        mesh_descriptor_writes_info[5]            = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[5].dstBinding = 5;
        mesh_descriptor_writes_info[5].pImageInfo = &specular_texture_image_info;

        mesh_descriptor_writes_info[6]            = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[6].dstBinding = 6;
        mesh_descriptor_writes_info[6].pImageInfo = &point_light_shadow_texture_image_info;

        mesh_descriptor_writes_info[7]            = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[7].dstBinding = 7;
        mesh_descriptor_writes_info[7].pImageInfo = &directional_light_shadow_texture_image_info;

        vkUpdateDescriptorSets(m_p_vulkan_context->_device,
                               sizeof(mesh_descriptor_writes_info) / sizeof(mesh_descriptor_writes_info[0]),
                               mesh_descriptor_writes_info,
                               0,
                               NULL);
    }
    void PLightingPass::setupSkyboxDescriptorSet()
    {
        VkDescriptorSetAllocateInfo skybox_descriptor_set_alloc_info;
        skybox_descriptor_set_alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        skybox_descriptor_set_alloc_info.pNext              = NULL;
        skybox_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        skybox_descriptor_set_alloc_info.descriptorSetCount = 1;
        skybox_descriptor_set_alloc_info.pSetLayouts        = &_descriptor_infos[_skybox].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_p_vulkan_context->_device,
                                                   &skybox_descriptor_set_alloc_info,
                                                   &_descriptor_infos[_skybox].descriptor_set))
        {
            throw std::runtime_error("allocate skybox descriptor set");
        }

        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset                 = 0;
        mesh_perframe_storage_buffer_info.range                  = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorImageInfo specular_texture_image_info = {};
        specular_texture_image_info.sampler   = m_p_global_render_resource->_ibl_resource._specular_texture_sampler;
        specular_texture_image_info.imageView = m_p_global_render_resource->_ibl_resource._specular_texture_image_view;
        specular_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet skybox_descriptor_writes_info[2];

        skybox_descriptor_writes_info[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        skybox_descriptor_writes_info[0].pNext           = NULL;
        skybox_descriptor_writes_info[0].dstSet          = _descriptor_infos[_skybox].descriptor_set;
        skybox_descriptor_writes_info[0].dstBinding      = 0;
        skybox_descriptor_writes_info[0].dstArrayElement = 0;
        skybox_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        skybox_descriptor_writes_info[0].descriptorCount = 1;
        skybox_descriptor_writes_info[0].pBufferInfo     = &mesh_perframe_storage_buffer_info;

        skybox_descriptor_writes_info[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        skybox_descriptor_writes_info[1].pNext           = NULL;
        skybox_descriptor_writes_info[1].dstSet          = _descriptor_infos[_skybox].descriptor_set;
        skybox_descriptor_writes_info[1].dstBinding      = 1;
        skybox_descriptor_writes_info[1].dstArrayElement = 0;
        skybox_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skybox_descriptor_writes_info[1].descriptorCount = 1;
        skybox_descriptor_writes_info[1].pImageInfo      = &specular_texture_image_info;

        vkUpdateDescriptorSets(m_p_vulkan_context->_device, 2, skybox_descriptor_writes_info, 0, NULL);
    }
    void PLightingPass::setupAxisDescriptorSet()
    {
        VkDescriptorSetAllocateInfo axis_descriptor_set_alloc_info;
        axis_descriptor_set_alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        axis_descriptor_set_alloc_info.pNext              = NULL;
        axis_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        axis_descriptor_set_alloc_info.descriptorSetCount = 1;
        axis_descriptor_set_alloc_info.pSetLayouts        = &_descriptor_infos[_axis].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_p_vulkan_context->_device,
                                                   &axis_descriptor_set_alloc_info,
                                                   &_descriptor_infos[_axis].descriptor_set))
        {
            throw std::runtime_error("allocate axis descriptor set");
        }

        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset                 = 0;
        mesh_perframe_storage_buffer_info.range                  = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo axis_storage_buffer_info = {};
        axis_storage_buffer_info.offset                 = 0;
        axis_storage_buffer_info.range                  = sizeof(AxisStorageBufferObject);
        axis_storage_buffer_info.buffer = m_p_global_render_resource->_storage_buffer._axis_inefficient_storage_buffer;

        VkWriteDescriptorSet axis_descriptor_writes_info[2];

        axis_descriptor_writes_info[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[0].pNext           = NULL;
        axis_descriptor_writes_info[0].dstSet          = _descriptor_infos[_axis].descriptor_set;
        axis_descriptor_writes_info[0].dstBinding      = 0;
        axis_descriptor_writes_info[0].dstArrayElement = 0;
        axis_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        axis_descriptor_writes_info[0].descriptorCount = 1;
        axis_descriptor_writes_info[0].pBufferInfo     = &mesh_perframe_storage_buffer_info;

        axis_descriptor_writes_info[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[1].pNext           = NULL;
        axis_descriptor_writes_info[1].dstSet          = _descriptor_infos[_axis].descriptor_set;
        axis_descriptor_writes_info[1].dstBinding      = 1;
        axis_descriptor_writes_info[1].dstArrayElement = 0;
        axis_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        axis_descriptor_writes_info[1].descriptorCount = 1;
        axis_descriptor_writes_info[1].pBufferInfo     = &axis_storage_buffer_info;

        vkUpdateDescriptorSets(m_p_vulkan_context->_device,
                               (uint32_t)(sizeof(axis_descriptor_writes_info) / sizeof(axis_descriptor_writes_info[0])),
                               axis_descriptor_writes_info,
                               0,
                               NULL);
    }
    void PLightingPass::setupParticleDescriptorSet()
    {
        VkDescriptorSetAllocateInfo particlebillboard_global_descriptor_set_alloc_info;
        particlebillboard_global_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        particlebillboard_global_descriptor_set_alloc_info.pNext = NULL;
        particlebillboard_global_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        particlebillboard_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        particlebillboard_global_descriptor_set_alloc_info.pSetLayouts        = &_descriptor_infos[_particle].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_p_vulkan_context->_device,
                                                   &particlebillboard_global_descriptor_set_alloc_info,
                                                   &_descriptor_infos[_particle].descriptor_set))
        {
            throw std::runtime_error("allocate particle billboard global descriptor set");
        }

        VkDescriptorBufferInfo particlebillboard_perframe_storage_buffer_info = {};
        particlebillboard_perframe_storage_buffer_info.offset                 = 0;
        particlebillboard_perframe_storage_buffer_info.range = sizeof(ParticleBillboardPerframeStorageBufferObject);
        particlebillboard_perframe_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;

        VkDescriptorBufferInfo particlebillboard_perdrawcall_storage_buffer_info = {};
        particlebillboard_perdrawcall_storage_buffer_info.offset                 = 0;
        particlebillboard_perdrawcall_storage_buffer_info.range =
            sizeof(ParticleBillboardPerdrawcallStorageBufferObject);
        particlebillboard_perdrawcall_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(particlebillboard_perdrawcall_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkWriteDescriptorSet particlebillboard_descriptor_writes_info[2];

        particlebillboard_descriptor_writes_info[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        particlebillboard_descriptor_writes_info[0].pNext           = NULL;
        particlebillboard_descriptor_writes_info[0].dstSet          = _descriptor_infos[_particle].descriptor_set;
        particlebillboard_descriptor_writes_info[0].dstBinding      = 0;
        particlebillboard_descriptor_writes_info[0].dstArrayElement = 0;
        particlebillboard_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        particlebillboard_descriptor_writes_info[0].descriptorCount = 1;
        particlebillboard_descriptor_writes_info[0].pBufferInfo     = &particlebillboard_perframe_storage_buffer_info;

        particlebillboard_descriptor_writes_info[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        particlebillboard_descriptor_writes_info[1].pNext           = NULL;
        particlebillboard_descriptor_writes_info[1].dstSet          = _descriptor_infos[_particle].descriptor_set;
        particlebillboard_descriptor_writes_info[1].dstBinding      = 1;
        particlebillboard_descriptor_writes_info[1].dstArrayElement = 0;
        particlebillboard_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        particlebillboard_descriptor_writes_info[1].descriptorCount = 1;
        particlebillboard_descriptor_writes_info[1].pBufferInfo = &particlebillboard_perdrawcall_storage_buffer_info;

        vkUpdateDescriptorSets(m_p_vulkan_context->_device, 2, particlebillboard_descriptor_writes_info, 0, NULL);
    }
    void PLightingPass::drawModel()
    {
        struct PMeshNode
        {
            glm::mat4 model_matrix;
            glm::mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count];
            bool      enable_vertex_blending;
        };

        std::map<VulkanPBRMaterial*, std::map<VulkanMesh*, std::vector<PMeshNode>>> main_camera_mesh_drawcall_batch;

        // reorganize mesh
        for (PVulkanMeshNode& node : *(m_visiable_nodes.p_main_camera_visible_mesh_nodes))
        {
            auto& mesh_instanced = main_camera_mesh_drawcall_batch[node.ref_material];
            auto& mesh_nodes     = mesh_instanced[node.ref_mesh];

            PMeshNode temp;
            temp.model_matrix           = node.model_matrix;
            temp.enable_vertex_blending = node.enable_vertex_blending;
            if (node.enable_vertex_blending)
            {
                for (uint32_t i = 0; i < m_mesh_vertex_blending_max_joint_count; ++i)
                {
                    temp.joint_matrices[i] = node.joint_matrices[i];
                }
            }

            mesh_nodes.push_back(temp);
        }

        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Model", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(m_command_info._current_command_buffer, &label_info);
        }

        m_p_vulkan_context->_vkCmdBindPipeline(m_command_info._current_command_buffer,
                                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               _render_pipelines[_render_pipeline_type_model].pipeline);
        m_p_vulkan_context->_vkCmdSetViewport(m_command_info._current_command_buffer, 0, 1, &m_command_info._viewport);
        m_p_vulkan_context->_vkCmdSetScissor(m_command_info._current_command_buffer, 0, 1, &m_command_info._scissor);

        // perframe storage buffer
        uint32_t perframe_dynamic_offset =
            roundUp(m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);

        m_p_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[m_command_info._current_frame_index] =
            perframe_dynamic_offset + sizeof(MeshPerframeStorageBufferObject);
        assert(m_p_global_render_resource->_storage_buffer
                   ._global_upload_ringbuffers_end[m_command_info._current_frame_index] <=
               (m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_begin[m_command_info._current_frame_index] +
                m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_size[m_command_info._current_frame_index]));

        (*reinterpret_cast<MeshPerframeStorageBufferObject*>(
            reinterpret_cast<uintptr_t>(
                m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
            perframe_dynamic_offset)) = m_mesh_perframe_storage_buffer_object;

        for (auto& pair1 : main_camera_mesh_drawcall_batch)
        {
            VulkanPBRMaterial& material       = (*pair1.first);
            auto&              mesh_instanced = pair1.second;

            // bind per material
            m_p_vulkan_context->_vkCmdBindDescriptorSets(m_command_info._current_command_buffer,
                                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                         _render_pipelines[_render_pipeline_type_model].layout,
                                                         2,
                                                         1,
                                                         &material.material_descriptor_set,
                                                         0,
                                                         NULL);

            // TODO: render from near to far

            for (auto& pair2 : mesh_instanced)
            {
                VulkanMesh& mesh       = (*pair2.first);
                auto&       mesh_nodes = pair2.second;

                uint32_t total_instance_count = static_cast<uint32_t>(mesh_nodes.size());
                if (total_instance_count > 0)
                {
                    // bind per mesh
                    m_p_vulkan_context->_vkCmdBindDescriptorSets(m_command_info._current_command_buffer,
                                                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                                 _render_pipelines[_render_pipeline_type_model].layout,
                                                                 1,
                                                                 1,
                                                                 &mesh.mesh_vertex_blending_descriptor_set,
                                                                 0,
                                                                 NULL);

                    VkBuffer     vertex_buffers[] = {mesh.mesh_vertex_position_buffer,
                                                 mesh.mesh_vertex_varying_enable_blending_buffer,
                                                 mesh.mesh_vertex_varying_buffer};
                    VkDeviceSize offsets[]        = {0, 0, 0};
                    m_p_vulkan_context->_vkCmdBindVertexBuffers(m_command_info._current_command_buffer,
                                                                0,
                                                                (sizeof(vertex_buffers) / sizeof(vertex_buffers[0])),
                                                                vertex_buffers,
                                                                offsets);
                    m_p_vulkan_context->_vkCmdBindIndexBuffer(
                        m_command_info._current_command_buffer, mesh.mesh_index_buffer, 0, VK_INDEX_TYPE_UINT16);

                    uint32_t drawcall_max_instance_count =
                        (sizeof(MeshPerdrawcallStorageBufferObject::mesh_instances) /
                         sizeof(MeshPerdrawcallStorageBufferObject::mesh_instances[0]));
                    uint32_t drawcall_count =
                        roundUp(total_instance_count, drawcall_max_instance_count) / drawcall_max_instance_count;

                    for (uint32_t drawcall_index = 0; drawcall_index < drawcall_count; ++drawcall_index)
                    {
                        uint32_t current_instance_count =
                            ((total_instance_count - drawcall_max_instance_count * drawcall_index) <
                             drawcall_max_instance_count) ?
                                (total_instance_count - drawcall_max_instance_count * drawcall_index) :
                                drawcall_max_instance_count;

                        // per drawcall storage buffer
                        uint32_t perdrawcall_dynamic_offset =
                            roundUp(m_p_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                        m_p_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[m_command_info._current_frame_index] =
                            perdrawcall_dynamic_offset + sizeof(MeshPerdrawcallStorageBufferObject);
                        assert(m_p_global_render_resource->_storage_buffer
                                   ._global_upload_ringbuffers_end[m_command_info._current_frame_index] <=
                               (m_p_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_begin[m_command_info._current_frame_index] +
                                m_p_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_size[m_command_info._current_frame_index]));

                        MeshPerdrawcallStorageBufferObject& perdrawcall_storage_buffer_object =
                            (*reinterpret_cast<MeshPerdrawcallStorageBufferObject*>(
                                reinterpret_cast<uintptr_t>(m_p_global_render_resource->_storage_buffer
                                                                ._global_upload_ringbuffer_memory_pointer) +
                                perdrawcall_dynamic_offset));
                        for (uint32_t i = 0; i < current_instance_count; ++i)
                        {
                            perdrawcall_storage_buffer_object.mesh_instances[i].model_matrix =
                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i].model_matrix;
                            perdrawcall_storage_buffer_object.mesh_instances[i].enable_vertex_blending =
                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i].enable_vertex_blending ?
                                    1.0 :
                                    -1.0;
                        }

                        // per drawcall vertex blending storage buffer
                        uint32_t per_drawcall_vertex_blending_dynamic_offset;
                        bool     least_one_enable_vertex_blending = true;
                        for (uint32_t i = 0; i < current_instance_count; ++i)
                        {
                            if (!mesh_nodes[drawcall_max_instance_count * drawcall_index + i].enable_vertex_blending)
                            {
                                least_one_enable_vertex_blending = false;
                                break;
                            }
                        }
                        if (least_one_enable_vertex_blending)
                        {
                            per_drawcall_vertex_blending_dynamic_offset = roundUp(
                                m_p_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                                m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                            m_p_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_command_info._current_frame_index] =
                                per_drawcall_vertex_blending_dynamic_offset +
                                sizeof(MeshPerdrawcallVertexBlendingStorageBufferObject);
                            assert(m_p_global_render_resource->_storage_buffer
                                       ._global_upload_ringbuffers_end[m_command_info._current_frame_index] <=
                                   (m_p_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_begin[m_command_info._current_frame_index] +
                                    m_p_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_size[m_command_info._current_frame_index]));

                            MeshPerdrawcallVertexBlendingStorageBufferObject&
                                per_drawcall_vertex_blending_storage_buffer_object =
                                    (*reinterpret_cast<MeshPerdrawcallVertexBlendingStorageBufferObject*>(
                                        reinterpret_cast<uintptr_t>(m_p_global_render_resource->_storage_buffer
                                                                        ._global_upload_ringbuffer_memory_pointer) +
                                        per_drawcall_vertex_blending_dynamic_offset));
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                if (mesh_nodes[drawcall_max_instance_count * drawcall_index + i].enable_vertex_blending)
                                {
                                    for (uint32_t j = 0; j < m_mesh_vertex_blending_max_joint_count; ++j)
                                    {
                                        per_drawcall_vertex_blending_storage_buffer_object
                                            .joint_matrices[m_mesh_vertex_blending_max_joint_count * i + j] =
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
                        m_p_vulkan_context->_vkCmdBindDescriptorSets(
                            m_command_info._current_command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _render_pipelines[_render_pipeline_type_model].layout,
                            0,
                            1,
                            &_descriptor_infos[_global].descriptor_set,
                            3,
                            dynamic_offsets);

                        m_p_vulkan_context->_vkCmdDrawIndexed(m_command_info._current_command_buffer,
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
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(m_command_info._current_command_buffer);
        }
    }
    void PLightingPass::drawSkybox()
    {
        uint32_t perframe_dynamic_offset =
            roundUp(m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);

        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Skybox", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(m_command_info._current_command_buffer, &label_info);
        }

        m_p_vulkan_context->_vkCmdBindPipeline(m_command_info._current_command_buffer,
                                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               _render_pipelines[_render_pipeline_type_skybox].pipeline);
        m_p_vulkan_context->_vkCmdBindDescriptorSets(m_command_info._current_command_buffer,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     _render_pipelines[_render_pipeline_type_skybox].layout,
                                                     0,
                                                     1,
                                                     &_descriptor_infos[_skybox].descriptor_set,
                                                     1,
                                                     &perframe_dynamic_offset);
        vkCmdDraw(m_command_info._current_command_buffer, 36, 1, 0,
                  0); // 2 triangles(6 vertex) each face, 6 faces

        if (m_render_config._enable_debug_untils_label)
        {
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(m_command_info._current_command_buffer);
        }
    }
    void PLightingPass::drawAxis()
    {
        if (!m_is_show_axis)
            return;

        uint32_t perframe_dynamic_offset =
            roundUp(m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);

        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Axis", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(m_command_info._current_command_buffer, &label_info);
        }

        m_p_vulkan_context->_vkCmdBindPipeline(m_command_info._current_command_buffer,
                                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               _render_pipelines[_render_pipeline_type_axis].pipeline);
        m_p_vulkan_context->_vkCmdBindDescriptorSets(m_command_info._current_command_buffer,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     _render_pipelines[_render_pipeline_type_axis].layout,
                                                     0,
                                                     1,
                                                     &_descriptor_infos[_axis].descriptor_set,
                                                     1,
                                                     &perframe_dynamic_offset);

        m_axis_storage_buffer_object.selected_axis = m_selected_axis;
        m_axis_storage_buffer_object.model_matrix  = m_visiable_nodes.p_axis_node->model_matrix;

        VkBuffer     vertex_buffers[] = {m_visiable_nodes.p_axis_node->ref_mesh->mesh_vertex_position_buffer,
                                     m_visiable_nodes.p_axis_node->ref_mesh->mesh_vertex_varying_enable_blending_buffer,
                                     m_visiable_nodes.p_axis_node->ref_mesh->mesh_vertex_varying_buffer};
        VkDeviceSize offsets[]        = {0, 0, 0};
        m_p_vulkan_context->_vkCmdBindVertexBuffers(m_command_info._current_command_buffer,
                                                    0,
                                                    (sizeof(vertex_buffers) / sizeof(vertex_buffers[0])),
                                                    vertex_buffers,
                                                    offsets);
        m_p_vulkan_context->_vkCmdBindIndexBuffer(m_command_info._current_command_buffer,
                                                  m_visiable_nodes.p_axis_node->ref_mesh->mesh_index_buffer,
                                                  0,
                                                  VK_INDEX_TYPE_UINT16);
        (*reinterpret_cast<AxisStorageBufferObject*>(reinterpret_cast<uintptr_t>(
            m_p_global_render_resource->_storage_buffer._axis_inefficient_storage_buffer_memory_pointer))) =
            m_axis_storage_buffer_object;

        m_p_vulkan_context->_vkCmdDrawIndexed(m_command_info._current_command_buffer,
                                              m_visiable_nodes.p_axis_node->ref_mesh->mesh_index_count,
                                              1,
                                              0,
                                              0,
                                              0);

        if (m_render_config._enable_debug_untils_label)
        {
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(m_command_info._current_command_buffer);
        }
    }
    void PLightingPass::drawBillboardParticle()
    {
        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "ParticleBillboard", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(m_command_info._current_command_buffer, &label_info);
        }

        m_p_vulkan_context->_vkCmdBindPipeline(m_command_info._current_command_buffer,
                                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               _render_pipelines[_render_pipeline_type_particle].pipeline);
        m_p_vulkan_context->_vkCmdSetViewport(m_command_info._current_command_buffer, 0, 1, &m_command_info._viewport);
        m_p_vulkan_context->_vkCmdSetScissor(m_command_info._current_command_buffer, 0, 1, &m_command_info._scissor);

        // perframe storage buffer
        uint32_t perframe_dynamic_offset =
            roundUp(m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
        m_p_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[m_command_info._current_frame_index] =
            perframe_dynamic_offset + sizeof(ParticleBillboardPerframeStorageBufferObject);
        assert(m_p_global_render_resource->_storage_buffer
                   ._global_upload_ringbuffers_end[m_command_info._current_frame_index] <=
               (m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_begin[m_command_info._current_frame_index] +
                m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_size[m_command_info._current_frame_index]));

        (*reinterpret_cast<ParticleBillboardPerframeStorageBufferObject*>(
            reinterpret_cast<uintptr_t>(
                m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
            perframe_dynamic_offset)) = m_particlebillboard_perframe_storage_buffer_object;

        for (PVulkanParticleBillboardNode& node : *(m_visiable_nodes.p_main_camera_visible_particlebillboard_nodes))
        {
            uint32_t total_instance_count = static_cast<uint32_t>(node.positions.size());

            if (total_instance_count > 0)
            {
                uint32_t drawcall_max_instance_count =
                    (sizeof(ParticleBillboardPerdrawcallStorageBufferObject::positions) /
                     sizeof(ParticleBillboardPerdrawcallStorageBufferObject::positions[0]));
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
                                    ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                                m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                    m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index] =
                        perdrawcall_dynamic_offset + sizeof(ParticleBillboardPerdrawcallStorageBufferObject);
                    assert(m_p_global_render_resource->_storage_buffer
                               ._global_upload_ringbuffers_end[m_command_info._current_frame_index] <=
                           (m_p_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_begin[m_command_info._current_frame_index] +
                            m_p_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_size[m_command_info._current_frame_index]));

                    ParticleBillboardPerdrawcallStorageBufferObject& perdrawcall_storage_buffer_object =
                        (*reinterpret_cast<ParticleBillboardPerdrawcallStorageBufferObject*>(
                            reinterpret_cast<uintptr_t>(
                                m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
                            perdrawcall_dynamic_offset));
                    for (uint32_t i = 0; i < current_instance_count; ++i)
                    {
                        perdrawcall_storage_buffer_object.positions[i] =
                            node.positions[drawcall_max_instance_count * drawcall_index + i];
                    }

                    // bind perdrawcall
                    uint32_t dynamic_offsets[2] = {perframe_dynamic_offset, perdrawcall_dynamic_offset};
                    m_p_vulkan_context->_vkCmdBindDescriptorSets(
                        m_command_info._current_command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        _render_pipelines[_render_pipeline_type_particle].layout,
                        0,
                        1,
                        &_descriptor_infos[_particle].descriptor_set,
                        2,
                        dynamic_offsets);

                    vkCmdDraw(m_command_info._current_command_buffer, 4, current_instance_count, 0, 0);
                }
            }
        }

        if (m_render_config._enable_debug_untils_label)
        {
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(m_command_info._current_command_buffer);
        }
    }
} // namespace Pilot
