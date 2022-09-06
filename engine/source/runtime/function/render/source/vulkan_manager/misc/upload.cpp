#include "runtime/core/base/macro.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"
#include <cstring>

void Pilot::PVulkanManager::updateMeshData(bool                                          enable_vertex_blending,
                                           uint32_t                                      index_buffer_size,
                                           void*                                         index_buffer_data,
                                           uint32_t                                      vertex_buffer_size,
                                           struct Mesh_PosNormalTangentTex0Vertex const* vertex_buffer_data,
                                           uint32_t                                      joint_binding_buffer_size,
                                           struct Mesh_VertexBinding const*              joint_binding_buffer_data,
                                           VulkanMesh&                                   now_mesh)
{
    now_mesh.enable_vertex_blending = enable_vertex_blending;
    assert(0 == (vertex_buffer_size % sizeof(Mesh_PosNormalTangentTex0Vertex)));
    now_mesh.mesh_vertex_count = vertex_buffer_size / sizeof(Mesh_PosNormalTangentTex0Vertex);
    updateVertexBuffer(enable_vertex_blending,
                       vertex_buffer_size,
                       vertex_buffer_data,
                       joint_binding_buffer_size,
                       joint_binding_buffer_data,
                       index_buffer_size,
                       reinterpret_cast<uint16_t*>(index_buffer_data),
                       now_mesh);
    assert(0 == (index_buffer_size % sizeof(uint16_t)));
    now_mesh.mesh_index_count = index_buffer_size / sizeof(uint16_t);
    updateIndexBuffer(index_buffer_size, index_buffer_data, now_mesh);
}

bool Pilot::PVulkanManager::updateVertexBuffer(bool                                          enable_vertex_blending,
                                               uint32_t                                      vertex_buffer_size,
                                               struct Mesh_PosNormalTangentTex0Vertex const* vertex_buffer_data,
                                               uint32_t                                      joint_binding_buffer_size,
                                               struct Mesh_VertexBinding const*              joint_binding_buffer_data,
                                               uint32_t                                      index_buffer_size,
                                               uint16_t*                                     index_buffer_data,
                                               VulkanMesh&                                   now_mesh)
{
    if (enable_vertex_blending)
    {
        assert(0 == (vertex_buffer_size % sizeof(Mesh_PosNormalTangentTex0Vertex)));
        uint32_t vertex_count = vertex_buffer_size / sizeof(Mesh_PosNormalTangentTex0Vertex);
        assert(0 == (index_buffer_size % sizeof(uint16_t)));
        uint32_t index_count = index_buffer_size / sizeof(uint16_t);

        VkDeviceSize vertex_position_buffer_size = sizeof(PMeshVertex::VulkanMeshVertexPostition) * vertex_count;
        VkDeviceSize vertex_varying_enable_blending_buffer_size =
            sizeof(PMeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
        VkDeviceSize vertex_varying_buffer_size       = sizeof(PMeshVertex::VulkanMeshVertexVarying) * vertex_count;
        VkDeviceSize vertex_joint_binding_buffer_size = sizeof(PMeshVertex::VulkanMeshVertexJointBinding) * index_count;

        VkDeviceSize vertex_position_buffer_offset = 0;
        VkDeviceSize vertex_varying_enable_blending_buffer_offset =
            vertex_position_buffer_offset + vertex_position_buffer_size;
        VkDeviceSize vertex_varying_buffer_offset =
            vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;
        VkDeviceSize vertex_joint_binding_buffer_offset = vertex_varying_buffer_offset + vertex_varying_buffer_size;

        // temporary staging buffer
        VkDeviceSize inefficient_staging_buffer_size = vertex_position_buffer_size +
                                                       vertex_varying_enable_blending_buffer_size +
                                                       vertex_varying_buffer_size + vertex_joint_binding_buffer_size;
        VkBuffer       inefficient_staging_buffer        = VK_NULL_HANDLE;
        VkDeviceMemory inefficient_staging_buffer_memory = VK_NULL_HANDLE;
        PVulkanUtil::createBuffer(m_vulkan_context._physical_device,
                                  m_vulkan_context._device,
                                  inefficient_staging_buffer_size,
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  inefficient_staging_buffer,
                                  inefficient_staging_buffer_memory);

        void* inefficient_staging_buffer_data;
        vkMapMemory(m_vulkan_context._device,
                    inefficient_staging_buffer_memory,
                    0,
                    VK_WHOLE_SIZE,
                    0,
                    &inefficient_staging_buffer_data);

        PMeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexPostition*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
        PMeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
                vertex_varying_enable_blending_buffer_offset);
        PMeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexVarying*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);
        PMeshVertex::VulkanMeshVertexJointBinding* mesh_vertex_joint_binding =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexJointBinding*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_joint_binding_buffer_offset);

        for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
        {
            glm::vec3 normal  = glm::vec3(vertex_buffer_data[vertex_index].nx,
                                         vertex_buffer_data[vertex_index].ny,
                                         vertex_buffer_data[vertex_index].nz);
            glm::vec3 tangent = glm::vec3(vertex_buffer_data[vertex_index].tx,
                                          vertex_buffer_data[vertex_index].ty,
                                          vertex_buffer_data[vertex_index].tz);

            mesh_vertex_positions[vertex_index].position = glm::vec3(vertex_buffer_data[vertex_index].x,
                                                                     vertex_buffer_data[vertex_index].y,
                                                                     vertex_buffer_data[vertex_index].z);

            mesh_vertex_blending_varyings[vertex_index].normal  = normal;
            mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

            mesh_vertex_varyings[vertex_index].texcoord =
                glm::vec2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
        }

        for (uint32_t index_index = 0; index_index < index_count; ++index_index)
        {
            uint32_t vertex_buffer_index = index_buffer_data[index_index];

            // TODO: move to assets loading process

            mesh_vertex_joint_binding[index_index].indices =
                glm::ivec4(joint_binding_buffer_data[vertex_buffer_index].index0,
                           joint_binding_buffer_data[vertex_buffer_index].index1,
                           joint_binding_buffer_data[vertex_buffer_index].index2,
                           joint_binding_buffer_data[vertex_buffer_index].index3);

            float inv_total_weight = joint_binding_buffer_data[vertex_buffer_index].weight0 +
                                     joint_binding_buffer_data[vertex_buffer_index].weight1 +
                                     joint_binding_buffer_data[vertex_buffer_index].weight2 +
                                     joint_binding_buffer_data[vertex_buffer_index].weight3;

            inv_total_weight = (inv_total_weight != 0.0) ? 1 / inv_total_weight : 1.0;

            mesh_vertex_joint_binding[index_index].weights =
                glm::vec4(joint_binding_buffer_data[vertex_buffer_index].weight0 * inv_total_weight,
                          joint_binding_buffer_data[vertex_buffer_index].weight1 * inv_total_weight,
                          joint_binding_buffer_data[vertex_buffer_index].weight2 * inv_total_weight,
                          joint_binding_buffer_data[vertex_buffer_index].weight3 * inv_total_weight);
        }

        vkUnmapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory);

        // use the vmaAllocator to allocate asset vertex buffer
        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.size  = vertex_position_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_position_buffer,
                        &now_mesh.mesh_vertex_position_buffer_allocation,
                        NULL);
        bufferInfo.size = vertex_varying_enable_blending_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_varying_enable_blending_buffer,
                        &now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
                        NULL);
        bufferInfo.size = vertex_varying_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_varying_buffer,
                        &now_mesh.mesh_vertex_varying_buffer_allocation,
                        NULL);

        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.size  = vertex_joint_binding_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_joint_binding_buffer,
                        &now_mesh.mesh_vertex_joint_binding_buffer_allocation,
                        NULL);

        // use the data from staging buffer
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_position_buffer,
                                vertex_position_buffer_offset,
                                0,
                                vertex_position_buffer_size);
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_varying_enable_blending_buffer,
                                vertex_varying_enable_blending_buffer_offset,
                                0,
                                vertex_varying_enable_blending_buffer_size);
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_varying_buffer,
                                vertex_varying_buffer_offset,
                                0,
                                vertex_varying_buffer_size);
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_joint_binding_buffer,
                                vertex_joint_binding_buffer_offset,
                                0,
                                vertex_joint_binding_buffer_size);

        // release staging buffer
        vkDestroyBuffer(m_vulkan_context._device, inefficient_staging_buffer, nullptr);
        vkFreeMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, nullptr);

        // update descriptor set
        VkDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts =
            &m_main_camera_pass._descriptor_infos[PMainCameraPass::LayoutType::_per_mesh].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_context._device,
                                                   &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                                                   &now_mesh.mesh_vertex_blending_descriptor_set))
        {
            throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
        }

        VkDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
        mesh_vertex_Joint_binding_storage_buffer_info.offset                 = 0;
        mesh_vertex_Joint_binding_storage_buffer_info.range                  = vertex_joint_binding_buffer_size;
        mesh_vertex_Joint_binding_storage_buffer_info.buffer = now_mesh.mesh_vertex_joint_binding_buffer;
        assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
               m_global_render_resource._storage_buffer._max_storage_buffer_range);

        VkDescriptorSet descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

        VkWriteDescriptorSet descriptor_writes[1];

        VkWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
            descriptor_writes[0];
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext           = NULL;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet          = descriptor_set_to_write;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding      = 0;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
            &mesh_vertex_Joint_binding_storage_buffer_info;

        vkUpdateDescriptorSets(m_vulkan_context._device,
                               (sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                               descriptor_writes,
                               0,
                               NULL);
    }
    else
    {
        assert(0 == (vertex_buffer_size % sizeof(Mesh_PosNormalTangentTex0Vertex)));
        uint32_t vertex_count = vertex_buffer_size / sizeof(Mesh_PosNormalTangentTex0Vertex);

        VkDeviceSize vertex_position_buffer_size = sizeof(PMeshVertex::VulkanMeshVertexPostition) * vertex_count;
        VkDeviceSize vertex_varying_enable_blending_buffer_size =
            sizeof(PMeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
        VkDeviceSize vertex_varying_buffer_size = sizeof(PMeshVertex::VulkanMeshVertexVarying) * vertex_count;

        VkDeviceSize vertex_position_buffer_offset = 0;
        VkDeviceSize vertex_varying_enable_blending_buffer_offset =
            vertex_position_buffer_offset + vertex_position_buffer_size;
        VkDeviceSize vertex_varying_buffer_offset =
            vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;

        // temporary staging buffer
        VkDeviceSize inefficient_staging_buffer_size =
            vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size;
        VkBuffer       inefficient_staging_buffer        = VK_NULL_HANDLE;
        VkDeviceMemory inefficient_staging_buffer_memory = VK_NULL_HANDLE;
        PVulkanUtil::createBuffer(m_vulkan_context._physical_device,
                                  m_vulkan_context._device,
                                  inefficient_staging_buffer_size,
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  inefficient_staging_buffer,
                                  inefficient_staging_buffer_memory);

        void* inefficient_staging_buffer_data;
        vkMapMemory(m_vulkan_context._device,
                    inefficient_staging_buffer_memory,
                    0,
                    VK_WHOLE_SIZE,
                    0,
                    &inefficient_staging_buffer_data);

        PMeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexPostition*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
        PMeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
                vertex_varying_enable_blending_buffer_offset);
        PMeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
            reinterpret_cast<PMeshVertex::VulkanMeshVertexVarying*>(
                reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);

        for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
        {
            glm::vec3 normal  = glm::vec3(vertex_buffer_data[vertex_index].nx,
                                         vertex_buffer_data[vertex_index].ny,
                                         vertex_buffer_data[vertex_index].nz);
            glm::vec3 tangent = glm::vec3(vertex_buffer_data[vertex_index].tx,
                                          vertex_buffer_data[vertex_index].ty,
                                          vertex_buffer_data[vertex_index].tz);

            mesh_vertex_positions[vertex_index].position = glm::vec3(vertex_buffer_data[vertex_index].x,
                                                                     vertex_buffer_data[vertex_index].y,
                                                                     vertex_buffer_data[vertex_index].z);

            mesh_vertex_blending_varyings[vertex_index].normal  = normal;
            mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

            mesh_vertex_varyings[vertex_index].texcoord =
                glm::vec2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
        }

        vkUnmapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory);

        // use the vmaAllocator to allocate asset vertex buffer
        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.usage              = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

        bufferInfo.size = vertex_position_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_position_buffer,
                        &now_mesh.mesh_vertex_position_buffer_allocation,
                        NULL);
        bufferInfo.size = vertex_varying_enable_blending_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_varying_enable_blending_buffer,
                        &now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
                        NULL);
        bufferInfo.size = vertex_varying_buffer_size;
        vmaCreateBuffer(m_vulkan_context._assets_allocator,
                        &bufferInfo,
                        &allocInfo,
                        &now_mesh.mesh_vertex_varying_buffer,
                        &now_mesh.mesh_vertex_varying_buffer_allocation,
                        NULL);

        // use the data from staging buffer
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_position_buffer,
                                vertex_position_buffer_offset,
                                0,
                                vertex_position_buffer_size);
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_varying_enable_blending_buffer,
                                vertex_varying_enable_blending_buffer_offset,
                                0,
                                vertex_varying_enable_blending_buffer_size);
        PVulkanUtil::copyBuffer(&m_vulkan_context,
                                inefficient_staging_buffer,
                                now_mesh.mesh_vertex_varying_buffer,
                                vertex_varying_buffer_offset,
                                0,
                                vertex_varying_buffer_size);

        // release staging buffer
        vkDestroyBuffer(m_vulkan_context._device, inefficient_staging_buffer, nullptr);
        vkFreeMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, nullptr);

        // update descriptor set
        VkDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool     = m_descriptor_pool;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts =
            &m_main_camera_pass._descriptor_infos[PMainCameraPass::LayoutType::_per_mesh].layout;
        if (VK_SUCCESS != vkAllocateDescriptorSets(m_vulkan_context._device,
                                                   &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                                                   &now_mesh.mesh_vertex_blending_descriptor_set))
        {
            throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
        }

        VkDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
        mesh_vertex_Joint_binding_storage_buffer_info.offset                 = 0;
        mesh_vertex_Joint_binding_storage_buffer_info.range                  = 1;
        mesh_vertex_Joint_binding_storage_buffer_info.buffer =
            m_global_render_resource._storage_buffer._global_null_descriptor_storage_buffer;
        assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
               m_global_render_resource._storage_buffer._max_storage_buffer_range);

        VkDescriptorSet descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

        VkWriteDescriptorSet descriptor_writes[1];

        VkWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
            descriptor_writes[0];
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext           = NULL;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet          = descriptor_set_to_write;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding      = 0;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
        mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
            &mesh_vertex_Joint_binding_storage_buffer_info;

        vkUpdateDescriptorSets(m_vulkan_context._device,
                               (sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                               descriptor_writes,
                               0,
                               NULL);
    }

    return true;
}

bool Pilot::PVulkanManager::updateIndexBuffer(uint32_t index_buffer_size, void* index_buffer_data, VulkanMesh& now_mesh)
{
    // temp staging buffer
    VkDeviceSize buffer_size = index_buffer_size;

    VkBuffer       inefficient_staging_buffer;
    VkDeviceMemory inefficient_staging_buffer_memory;
    PVulkanUtil::createBuffer(m_vulkan_context._physical_device,
                              m_vulkan_context._device,
                              buffer_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

    void* staging_buffer_data;
    vkMapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, 0, buffer_size, 0, &staging_buffer_data);
    memcpy(staging_buffer_data, index_buffer_data, (size_t)buffer_size);
    vkUnmapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory);

    // use the vmaAllocator to allocate asset index buffer
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size               = buffer_size;
    bufferInfo.usage              = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateBuffer(m_vulkan_context._assets_allocator,
                    &bufferInfo,
                    &allocInfo,
                    &now_mesh.mesh_index_buffer,
                    &now_mesh.mesh_index_buffer_allocation,
                    NULL);

    // use the data from staging buffer
    PVulkanUtil::copyBuffer(
        &m_vulkan_context, inefficient_staging_buffer, now_mesh.mesh_index_buffer, 0, 0, buffer_size);

    // release temp staging buffer
    vkDestroyBuffer(m_vulkan_context._device, inefficient_staging_buffer, nullptr);
    vkFreeMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, nullptr);

    // now index buffer is ready for render commands in cmdbind
    return true;
}

void Pilot::PVulkanManager::updateTextureImageData(const PTextureDataToUpdate& texture_data)
{
    initializeTextureImage(texture_data.now_material->base_color_texture_image,
                           texture_data.now_material->base_color_image_view,
                           texture_data.now_material->base_color_image_allocation,
                           texture_data.base_color_image_width,
                           texture_data.base_color_image_height,
                           texture_data.base_color_image_pixels,
                           texture_data.base_color_image_format);

    initializeTextureImage(texture_data.now_material->metallic_roughness_texture_image,
                           texture_data.now_material->metallic_roughness_image_view,
                           texture_data.now_material->metallic_roughness_image_allocation,
                           texture_data.metallic_roughness_image_width,
                           texture_data.metallic_roughness_image_height,
                           texture_data.metallic_roughness_image_pixels,
                           texture_data.metallic_roughness_image_format);

    initializeTextureImage(texture_data.now_material->normal_texture_image,
                           texture_data.now_material->normal_image_view,
                           texture_data.now_material->normal_image_allocation,
                           texture_data.normal_roughness_image_width,
                           texture_data.normal_roughness_image_height,
                           texture_data.normal_roughness_image_pixels,
                           texture_data.normal_roughness_image_format);

    initializeTextureImage(texture_data.now_material->occlusion_texture_image,
                           texture_data.now_material->occlusion_image_view,
                           texture_data.now_material->occlusion_image_allocation,
                           texture_data.occlusion_image_width,
                           texture_data.occlusion_image_height,
                           texture_data.occlusion_image_pixels,
                           texture_data.occlusion_image_format);
    initializeTextureImage(texture_data.now_material->emissive_texture_image,
                           texture_data.now_material->emissive_image_view,
                           texture_data.now_material->emissive_image_allocation,
                           texture_data.emissive_image_width,
                           texture_data.emissive_image_height,
                           texture_data.emissive_image_pixels,
                           texture_data.emissive_image_format);
}

void Pilot::PVulkanManager::updateGlobalTexturesForIBL(PIBLResourceData& ibl_resource_data)
{
    // brdfLUT texture (x1)
    initializeTextureImage(m_global_render_resource._ibl_resource._brdfLUT_texture_image,
                           m_global_render_resource._ibl_resource._brdfLUT_texture_image_view,
                           m_global_render_resource._ibl_resource._brdfLUT_texture_image_allocation,
                           ibl_resource_data._brdfLUT_texture_image_width,
                           ibl_resource_data._brdfLUT_texture_image_height,
                           ibl_resource_data._brdfLUT_texture_image_pixels,
                           ibl_resource_data._brdfLUT_texture_image_format);

    // irradiance texture (x6)
    uint32_t irradiance_cubemap_miplevels;
    irradiance_cubemap_miplevels =
        static_cast<uint32_t>(std::floor(std::log2(std::max(ibl_resource_data._irradiance_texture_image_width,
                                                            ibl_resource_data._irradiance_texture_image_height)))) +
        1;
    initializeCubeMap(m_global_render_resource._ibl_resource._irradiance_texture_image,
                      m_global_render_resource._ibl_resource._irradiance_texture_image_view,
                      m_global_render_resource._ibl_resource._irradiance_texture_image_allocation,
                      ibl_resource_data._irradiance_texture_image_width,
                      ibl_resource_data._irradiance_texture_image_height,
                      ibl_resource_data._irradiance_texture_image_pixels,
                      ibl_resource_data._irradiance_texture_image_format,
                      irradiance_cubemap_miplevels);

    // specular texture (x6)
    uint32_t specular_cubemap_miplevels;
    specular_cubemap_miplevels =
        static_cast<uint32_t>(std::floor(std::log2(std::max(ibl_resource_data._specular_texture_image_width,
                                                            ibl_resource_data._specular_texture_image_height)))) +
        1;
    initializeCubeMap(m_global_render_resource._ibl_resource._specular_texture_image,
                      m_global_render_resource._ibl_resource._specular_texture_image_view,
                      m_global_render_resource._ibl_resource._specular_texture_image_allocation,
                      ibl_resource_data._specular_texture_image_width,
                      ibl_resource_data._specular_texture_image_height,
                      ibl_resource_data._specular_texture_image_pixels,
                      ibl_resource_data._specular_texture_image_format,
                      specular_cubemap_miplevels);
}

void Pilot::PVulkanManager::updateGlobalTexturesForColorGrading(PColorGradingResourceData& color_grading_resource_data)
{
    // color grading texture (x1)
    // TODO: remove debug information
    if (
    initializeTextureImage(m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image,
                           m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_view,
                           m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_allocation,
                           color_grading_resource_data._color_grading_LUT_texture_image_width,
                           color_grading_resource_data._color_grading_LUT_texture_image_height,
                           color_grading_resource_data._color_grading_LUT_texture_image_pixels,
                           color_grading_resource_data._color_grading_LUT_texture_image_format,
                           1)) {
                               LOG_DEBUG("color grading texture initialized");
                           }
                           else {
                               LOG_WARN("color grading texture not initialized");
                           }
}

void Pilot::PVulkanManager::initializeCubeMap(VkImage&             image,
                                              VkImageView&         image_view,
                                              VmaAllocation&       image_allocation,
                                              uint32_t             texture_image_width,
                                              uint32_t             texture_image_height,
                                              std::array<void*, 6> texture_image_pixels,
                                              PILOT_PIXEL_FORMAT   texture_image_format,
                                              uint32_t             miplevels)
{
    VkDeviceSize texture_layer_byte_size;
    VkDeviceSize cube_byte_size;
    VkFormat     vulkan_image_format;

    switch (texture_image_format)
    {
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8_UNORM:
            texture_layer_byte_size = texture_image_width * texture_image_height * 3;
            vulkan_image_format     = VK_FORMAT_R8G8B8_UNORM;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8_SRGB:
            texture_layer_byte_size = texture_image_width * texture_image_height * 3;
            vulkan_image_format     = VK_FORMAT_R8G8B8_SRGB;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM:
            texture_layer_byte_size = texture_image_width * texture_image_height * 4;
            vulkan_image_format     = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_SRGB:
            texture_layer_byte_size = texture_image_width * texture_image_height * 4;
            vulkan_image_format     = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32_FLOAT:
            texture_layer_byte_size = texture_image_width * texture_image_height * 4 * 2;
            vulkan_image_format     = VK_FORMAT_R32G32_SFLOAT;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32_FLOAT:
            texture_layer_byte_size = texture_image_width * texture_image_height * 4 * 3;
            vulkan_image_format     = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32A32_FLOAT:
            texture_layer_byte_size = texture_image_width * texture_image_height * 4 * 4;
            vulkan_image_format     = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        default:
            texture_layer_byte_size = VkDeviceSize(-1);
            throw std::runtime_error("invalid texture_layer_byte_size");
            break;
    }

    cube_byte_size = texture_layer_byte_size * 6;

    // create cubemap texture image
    // use the vmaAllocator to allocate asset texture image
    VkImageCreateInfo image_create_info {};
    image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    image_create_info.imageType     = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width  = static_cast<uint32_t>(texture_image_width);
    image_create_info.extent.height = static_cast<uint32_t>(texture_image_height);
    image_create_info.extent.depth  = 1;
    image_create_info.mipLevels     = miplevels;
    image_create_info.arrayLayers   = 6;
    image_create_info.format        = vulkan_image_format;
    image_create_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.samples     = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_vulkan_context._assets_allocator, &image_create_info, &allocInfo, &image, &image_allocation, NULL);

    VkBuffer       inefficient_staging_buffer;
    VkDeviceMemory inefficient_staging_buffer_memory;
    PVulkanUtil::createBuffer(m_vulkan_context._physical_device,
                              m_vulkan_context._device,
                              cube_byte_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

    void* data = NULL;
    vkMapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, 0, cube_byte_size, 0, &data);
    for (int i = 0; i < 6; i++)
    {
        memcpy((void*)(static_cast<char*>(data) + texture_layer_byte_size * i),
               texture_image_pixels[i],
               static_cast<size_t>(texture_layer_byte_size));
    }
    vkUnmapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory);

    // layout transitions -- image layout is set from none to destination
    PVulkanUtil::transitionImageLayout(&m_vulkan_context,
                                       image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       6,
                                       miplevels,
                                       VK_IMAGE_ASPECT_COLOR_BIT);
    // copy from staging buffer as destination
    PVulkanUtil::copyBufferToImage(&m_vulkan_context,
                                   inefficient_staging_buffer,
                                   image,
                                   static_cast<uint32_t>(texture_image_width),
                                   static_cast<uint32_t>(texture_image_height),
                                   6);

    vkDestroyBuffer(m_vulkan_context._device, inefficient_staging_buffer, nullptr);
    vkFreeMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, nullptr);

    generateTextureMipMaps(image, vulkan_image_format, texture_image_width, texture_image_height, 6, miplevels);

    image_view = PVulkanUtil::createImageView(m_vulkan_context._device,
                                              image,
                                              vulkan_image_format,
                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                              VK_IMAGE_VIEW_TYPE_CUBE,
                                              6,
                                              miplevels);
}

bool Pilot::PVulkanManager::initializeTextureImage(VkImage&           image,
                                                   VkImageView&       image_view,
                                                   VmaAllocation&     image_allocation,
                                                   uint32_t           texture_image_width,
                                                   uint32_t           texture_image_height,
                                                   void*              texture_image_pixels,
                                                   PILOT_PIXEL_FORMAT texture_image_format,
                                                   uint32_t           miplevels)
{
    if (!texture_image_pixels)
    {
        return false;
    }

    VkDeviceSize texture_byte_size;
    VkFormat     vulkan_image_format;
    switch (texture_image_format)
    {
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8_UNORM:
            texture_byte_size   = texture_image_width * texture_image_height * 3;
            vulkan_image_format = VK_FORMAT_R8G8B8_UNORM;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8_SRGB:
            texture_byte_size   = texture_image_width * texture_image_height * 3;
            vulkan_image_format = VK_FORMAT_R8G8B8_SRGB;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM:
            texture_byte_size   = texture_image_width * texture_image_height * 4;
            vulkan_image_format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_SRGB:
            texture_byte_size   = texture_image_width * texture_image_height * 4;
            vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32_FLOAT:
            texture_byte_size   = texture_image_width * texture_image_height * 4 * 2;
            vulkan_image_format = VK_FORMAT_R32G32_SFLOAT;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32_FLOAT:
            texture_byte_size   = texture_image_width * texture_image_height * 4 * 3;
            vulkan_image_format = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32A32_FLOAT:
            texture_byte_size   = texture_image_width * texture_image_height * 4 * 4;
            vulkan_image_format = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        default:
            throw std::runtime_error("invalid texture_byte_size");
            break;
    }

    // use staging buffer
    VkBuffer       inefficient_staging_buffer;
    VkDeviceMemory inefficient_staging_buffer_memory;
    PVulkanUtil::createBuffer(m_vulkan_context._physical_device,
                              m_vulkan_context._device,
                              texture_byte_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

    void* data;
    vkMapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, 0, texture_byte_size, 0, &data);
    memcpy(data, texture_image_pixels, static_cast<size_t>(texture_byte_size));
    vkUnmapMemory(m_vulkan_context._device, inefficient_staging_buffer_memory);

    // generate mipmapped image
    uint32_t mip_levels =
        (miplevels != 0) ? miplevels : floor(log2(std::max(texture_image_width, texture_image_height))) + 1;

    // use the vmaAllocator to allocate asset texture image
    VkImageCreateInfo image_create_info {};
    image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.flags         = 0;
    image_create_info.imageType     = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width  = texture_image_width;
    image_create_info.extent.height = texture_image_height;
    image_create_info.extent.depth  = 1;
    image_create_info.mipLevels     = mip_levels;
    image_create_info.arrayLayers   = 1;
    image_create_info.format        = vulkan_image_format;
    image_create_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.samples     = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_vulkan_context._assets_allocator, &image_create_info, &allocInfo, &image, &image_allocation, NULL);

    // layout transitions -- image layout is set from none to destination
    PVulkanUtil::transitionImageLayout(&m_vulkan_context,
                                       image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       1,
                                       1,
                                       VK_IMAGE_ASPECT_COLOR_BIT);
    // copy from staging buffer as destination
    PVulkanUtil::copyBufferToImage(
        &m_vulkan_context, inefficient_staging_buffer, image, texture_image_width, texture_image_height, 1);
    // layout transitions -- image layout is set from destination to shader_read
    PVulkanUtil::transitionImageLayout(&m_vulkan_context,
                                       image,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       1,
                                       1,
                                       VK_IMAGE_ASPECT_COLOR_BIT);

    vkDestroyBuffer(m_vulkan_context._device, inefficient_staging_buffer, nullptr);
    vkFreeMemory(m_vulkan_context._device, inefficient_staging_buffer_memory, nullptr);

    // generate mipmapped image
    PVulkanUtil::genMipmappedImage(&m_vulkan_context, image, texture_image_width, texture_image_height, mip_levels);

    image_view = PVulkanUtil::createImageView(m_vulkan_context._device,
                                              image,
                                              vulkan_image_format,
                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                              VK_IMAGE_VIEW_TYPE_2D,
                                              1,
                                              mip_levels);
    return true;
}
