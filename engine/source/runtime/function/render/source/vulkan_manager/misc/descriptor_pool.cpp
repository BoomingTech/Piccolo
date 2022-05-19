#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

bool Pilot::PVulkanManager::initializeDescriptorPool()
{
    // Since DescriptorSet should be treated as asset in Vulkan, DescriptorPool
    // should be big enough, and thus we can sub-allocate DescriptorSet from
    // DescriptorPool merely as we sub-allocate Buffer/Image from DeviceMemory.

    VkDescriptorPoolSize pool_sizes[5];
    pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    pool_sizes[0].descriptorCount = 3 + 2 + 2 + 2 + 1 + 1 + 3 + 3;
    pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[1].descriptorCount = 1 + 1 + 1 * m_max_vertex_blending_mesh_count+1;
    pool_sizes[2].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[2].descriptorCount = 1 * m_max_material_count+1;
    pool_sizes[3].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[3].descriptorCount = 3 + 5 * m_max_material_count + 1 + 1 +1; // ImGui_ImplVulkan_CreateDeviceObjects
    pool_sizes[4].type            = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    pool_sizes[4].descriptorCount = 4 + 1 + 1 + 2+1;

    VkDescriptorPoolCreateInfo pool_info {};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
    pool_info.pPoolSizes    = pool_sizes;
    pool_info.maxSets =
        1 + 1 + 1 + m_max_material_count + m_max_vertex_blending_mesh_count + 1 + 1; // +skybox + axis descriptor set
    pool_info.flags = 0U;

    if (vkCreateDescriptorPool(m_vulkan_context._device, &pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("create descriptor pool");
    }

    return true;
}
