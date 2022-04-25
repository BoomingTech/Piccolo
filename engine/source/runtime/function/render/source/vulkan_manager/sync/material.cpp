#include "runtime/function/render/include/render/render.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

Pilot::VulkanPBRMaterial& Pilot::PVulkanManager::syncMaterial(const struct Material& material,
                                                              class PilotRenderer*   pilot_renderer)
{
    size_t assetid = material.m_guid;

    auto iter = m_vulkan_pbr_materials.find(assetid);
    if (iter != m_vulkan_pbr_materials.end())
    {
        return iter->second;
    }
    else
    {
        VulkanPBRMaterial temp;
        auto              res = m_vulkan_pbr_materials.insert(std::make_pair(assetid, std::move(temp)));
        assert(res.second);

        float empty_image[] = {0.5f, 0.5f, 0.5f, 0.5f};

        TextureHandle      base_color_texture_handle = material.m_baseColorTexture;
        const SceneImage*  base_color_texture_image  = pilot_renderer->f_get_image(base_color_texture_handle);
        void*              base_color_image_pixels   = empty_image;
        uint32_t           base_color_image_width    = 1;
        uint32_t           base_color_image_height   = 1;
        PILOT_PIXEL_FORMAT base_color_image_format   = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_SRGB;
        if (base_color_texture_image != NULL)
        {
            base_color_image_pixels = base_color_texture_image->m_pixels;
            base_color_image_width  = static_cast<uint32_t>(base_color_texture_image->m_width);
            base_color_image_height = static_cast<uint32_t>(base_color_texture_image->m_height);
            base_color_image_format = base_color_texture_image->m_format;
        }

        TextureHandle     metallic_roughness_texture_handle = material.m_metallicRoughnessTexture;
        const SceneImage* metallic_roughness_texture_image =
            pilot_renderer->f_get_image(metallic_roughness_texture_handle);
        void*              metallic_roughness_image_pixels = empty_image;
        uint32_t           metallic_roughness_width        = 1;
        uint32_t           metallic_roughness_height       = 1;
        PILOT_PIXEL_FORMAT metallic_roughness_format       = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM;
        if (metallic_roughness_texture_image != NULL)
        {
            metallic_roughness_image_pixels = metallic_roughness_texture_image->m_pixels;
            metallic_roughness_width        = static_cast<uint32_t>(metallic_roughness_texture_image->m_width);
            metallic_roughness_height       = static_cast<uint32_t>(metallic_roughness_texture_image->m_height);
            metallic_roughness_format       = metallic_roughness_texture_image->m_format;
        }

        TextureHandle      normal_texture_handle         = material.m_normalTexture;
        const SceneImage*  normal_texture_image          = pilot_renderer->f_get_image(normal_texture_handle);
        void*              normal_roughness_image_pixels = empty_image;
        uint32_t           normal_roughness_width        = 1;
        uint32_t           normal_roughness_height       = 1;
        PILOT_PIXEL_FORMAT normal_roughness_format       = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM;
        if (normal_texture_image != NULL)
        {
            normal_roughness_image_pixels = normal_texture_image->m_pixels;
            normal_roughness_width        = static_cast<uint32_t>(normal_texture_image->m_width);
            normal_roughness_height       = static_cast<uint32_t>(normal_texture_image->m_height);
            normal_roughness_format       = normal_texture_image->m_format;
        }

        // some GLTF files combine metallic/roughness and occlusion values into one
        // texture
        TextureHandle      occlusion_texture_handle = material.m_occlusionTexture;
        const SceneImage*  occlusion_texture_image  = pilot_renderer->f_get_image(occlusion_texture_handle);
        void*              occlusion_image_pixels   = empty_image;
        uint32_t           occlusion_image_width    = 1;
        uint32_t           occlusion_image_height   = 1;
        PILOT_PIXEL_FORMAT occlusion_image_format   = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM;
        if (occlusion_texture_image != NULL)
        {
            occlusion_image_pixels = occlusion_texture_image->m_pixels;
            occlusion_image_width  = static_cast<uint32_t>(occlusion_texture_image->m_width);
            occlusion_image_height = static_cast<uint32_t>(occlusion_texture_image->m_height);
            occlusion_image_format = occlusion_texture_image->m_format;
        }

        TextureHandle      emissive_texture_handle = material.m_emissiveTexture;
        const SceneImage*  emissive_texture_image  = pilot_renderer->f_get_image(emissive_texture_handle);
        void*              emissive_image_pixels   = empty_image;
        uint32_t           emissive_image_width    = 1;
        uint32_t           emissive_image_height   = 1;
        PILOT_PIXEL_FORMAT emissive_image_format   = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM;
        if (emissive_texture_image != NULL)
        {
            emissive_image_pixels = emissive_texture_image->m_pixels;
            emissive_image_width  = static_cast<uint32_t>(emissive_texture_image->m_width);
            emissive_image_height = static_cast<uint32_t>(emissive_texture_image->m_height);
            emissive_image_format = emissive_texture_image->m_format;
        }

        VulkanPBRMaterial& now_material = res.first->second;

        // similiarly to the vertex/index buffer, we should allocate the uniform
        // buffer in DEVICE_LOCAL memory and use the temp stage buffer to copy the
        // data
        {
            // temporary staging buffer
            VkDeviceSize buffer_size = sizeof(MeshPerMaterialUniformBufferObject);

            VkBuffer       inefficient_staging_buffer        = VK_NULL_HANDLE;
            VkDeviceMemory inefficient_staging_buffer_memory = VK_NULL_HANDLE;
            PVulkanUtil::createBuffer(m_vulkan_context._physical_device,
                                      m_vulkan_context._device,
                                      buffer_size,
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      inefficient_staging_buffer,
                                      inefficient_staging_buffer_memory);
            // VK_BUFFER_USAGE_TRANSFER_SRC_BIT: buffer can be used as source in a
            // memory transfer operation

            void* staging_buffer_data = nullptr;
            vkMapMemory(
                m_vulkan_context._device, inefficient_staging_buffer_memory, 0, buffer_size, 0, &staging_buffer_data);

            MeshPerMaterialUniformBufferObject& material_uniform_buffer_info =
                (*static_cast<MeshPerMaterialUniformBufferObject*>(staging_buffer_data));
            material_uniform_buffer_info.is_blend          = material.m_blend;
            material_uniform_buffer_info.is_double_sided   = material.m_doubleSided;
            material_uniform_buffer_info.baseColorFactor   = material.m_baseColorFactor;
            material_uniform_buffer_info.metallicFactor    = material.m_metallicFactor;
            material_uniform_buffer_info.roughnessFactor   = material.m_roughnessFactor;
            material_uniform_buffer_info.normalScale       = material.m_normalScale;
            material_uniform_buffer_info.occlusionStrength = material.m_occlusionStrength;
            material_uniform_buffer_info.emissiveFactor    = material.m_emissiveFactor;

            vkUnmapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory);

            // use the vmaAllocator to allocate asset uniform buffer
            VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufferInfo.size               = buffer_size;
            bufferInfo.usage              = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

            vmaCreateBufferWithAlignment(m_vulkan_context._assets_allocator,
                                         &bufferInfo,
                                         &allocInfo,
                                         m_global_render_resource._storage_buffer._min_uniform_buffer_offset_alignment,
                                         &now_material.material_uniform_buffer,
                                         &now_material.material_uniform_buffer_allocation,
                                         NULL);

            // use the data from staging buffer
            PVulkanUtil::copyBuffer(
                &m_vulkan_context, inefficient_staging_buffer, now_material.material_uniform_buffer, 0, 0, buffer_size);

            // release staging buffer
            vkDestroyBuffer(m_vulkan_context._device, inefficient_staging_buffer, nullptr);
            vkFreeMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, nullptr);
        }

        PTextureDataToUpdate update_texture_data;
        update_texture_data.base_color_image_pixels         = base_color_image_pixels;
        update_texture_data.base_color_image_width          = base_color_image_width;
        update_texture_data.base_color_image_height         = base_color_image_height;
        update_texture_data.base_color_image_format         = base_color_image_format;
        update_texture_data.metallic_roughness_image_pixels = metallic_roughness_image_pixels;
        update_texture_data.metallic_roughness_image_width  = metallic_roughness_width;
        update_texture_data.metallic_roughness_image_height = metallic_roughness_height;
        update_texture_data.metallic_roughness_image_format = metallic_roughness_format;
        update_texture_data.normal_roughness_image_pixels   = normal_roughness_image_pixels;
        update_texture_data.normal_roughness_image_width    = normal_roughness_width;
        update_texture_data.normal_roughness_image_height   = normal_roughness_height;
        update_texture_data.normal_roughness_image_format   = normal_roughness_format;
        update_texture_data.occlusion_image_pixels          = occlusion_image_pixels;
        update_texture_data.occlusion_image_width           = occlusion_image_width;
        update_texture_data.occlusion_image_height          = occlusion_image_height;
        update_texture_data.occlusion_image_format          = occlusion_image_format;
        update_texture_data.emissive_image_pixels           = emissive_image_pixels;
        update_texture_data.emissive_image_width            = emissive_image_width;
        update_texture_data.emissive_image_height           = emissive_image_height;
        update_texture_data.emissive_image_format           = emissive_image_format;
        update_texture_data.now_material                    = &now_material;

        updateTextureImageData(update_texture_data);

        VkDescriptorSetAllocateInfo material_descriptor_set_alloc_info;
        material_descriptor_set_alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        material_descriptor_set_alloc_info.pNext              = NULL;
        material_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        material_descriptor_set_alloc_info.descriptorSetCount = 1;
        material_descriptor_set_alloc_info.pSetLayouts =
            &m_main_camera_pass._descriptor_infos[PMainCameraPass::LayoutType::_mesh_per_material].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_context._device,
                                                   &material_descriptor_set_alloc_info,
                                                   &now_material.material_descriptor_set))
        {
            throw std::runtime_error("allocate material descriptor set");
        }

        VkDescriptorBufferInfo material_uniform_buffer_info = {};
        material_uniform_buffer_info.offset                 = 0;
        material_uniform_buffer_info.range                  = sizeof(MeshPerMaterialUniformBufferObject);
        material_uniform_buffer_info.buffer                 = now_material.material_uniform_buffer;

        VkDescriptorImageInfo base_color_image_info = {};
        base_color_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        base_color_image_info.imageView             = now_material.base_color_image_view;
        base_color_image_info.sampler = PVulkanUtil::getOrCreateMipmapSampler(m_vulkan_context._physical_device,
                                                                              m_vulkan_context._device,
                                                                              base_color_image_width,
                                                                              base_color_image_height);

        VkDescriptorImageInfo metallic_roughness_image_info = {};
        metallic_roughness_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metallic_roughness_image_info.imageView             = now_material.metallic_roughness_image_view;
        metallic_roughness_image_info.sampler = PVulkanUtil::getOrCreateMipmapSampler(m_vulkan_context._physical_device,
                                                                                      m_vulkan_context._device,
                                                                                      metallic_roughness_width,
                                                                                      metallic_roughness_height);

        VkDescriptorImageInfo normal_roughness_image_info = {};
        normal_roughness_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normal_roughness_image_info.imageView             = now_material.normal_image_view;
        normal_roughness_image_info.sampler = PVulkanUtil::getOrCreateMipmapSampler(m_vulkan_context._physical_device,
                                                                                    m_vulkan_context._device,
                                                                                    normal_roughness_width,
                                                                                    normal_roughness_height);

        VkDescriptorImageInfo occlusion_image_info = {};
        occlusion_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        occlusion_image_info.imageView             = now_material.occlusion_image_view;
        occlusion_image_info.sampler               = PVulkanUtil::getOrCreateMipmapSampler(
            m_vulkan_context._physical_device, m_vulkan_context._device, occlusion_image_width, occlusion_image_height);

        VkDescriptorImageInfo emissive_image_info = {};
        emissive_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        emissive_image_info.imageView             = now_material.emissive_image_view;
        emissive_image_info.sampler               = PVulkanUtil::getOrCreateMipmapSampler(
            m_vulkan_context._physical_device, m_vulkan_context._device, emissive_image_width, emissive_image_height);

        VkWriteDescriptorSet mesh_descriptor_writes_info[6];

        mesh_descriptor_writes_info[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[0].pNext           = NULL;
        mesh_descriptor_writes_info[0].dstSet          = now_material.material_descriptor_set;
        mesh_descriptor_writes_info[0].dstBinding      = 0;
        mesh_descriptor_writes_info[0].dstArrayElement = 0;
        mesh_descriptor_writes_info[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mesh_descriptor_writes_info[0].descriptorCount = 1;
        mesh_descriptor_writes_info[0].pBufferInfo     = &material_uniform_buffer_info;

        mesh_descriptor_writes_info[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[1].pNext           = NULL;
        mesh_descriptor_writes_info[1].dstSet          = now_material.material_descriptor_set;
        mesh_descriptor_writes_info[1].dstBinding      = 1;
        mesh_descriptor_writes_info[1].dstArrayElement = 0;
        mesh_descriptor_writes_info[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mesh_descriptor_writes_info[1].descriptorCount = 1;
        mesh_descriptor_writes_info[1].pImageInfo      = &base_color_image_info;

        mesh_descriptor_writes_info[2]            = mesh_descriptor_writes_info[1];
        mesh_descriptor_writes_info[2].dstBinding = 2;
        mesh_descriptor_writes_info[2].pImageInfo = &metallic_roughness_image_info;

        mesh_descriptor_writes_info[3]            = mesh_descriptor_writes_info[1];
        mesh_descriptor_writes_info[3].dstBinding = 3;
        mesh_descriptor_writes_info[3].pImageInfo = &normal_roughness_image_info;

        mesh_descriptor_writes_info[4]            = mesh_descriptor_writes_info[1];
        mesh_descriptor_writes_info[4].dstBinding = 4;
        mesh_descriptor_writes_info[4].pImageInfo = &occlusion_image_info;

        mesh_descriptor_writes_info[5]            = mesh_descriptor_writes_info[1];
        mesh_descriptor_writes_info[5].dstBinding = 5;
        mesh_descriptor_writes_info[5].pImageInfo = &emissive_image_info;

        vkUpdateDescriptorSets(m_vulkan_context._device, 6, mesh_descriptor_writes_info, 0, nullptr);

        return now_material;
    }
}
