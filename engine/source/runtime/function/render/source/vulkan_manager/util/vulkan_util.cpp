#include "runtime/function/render/include/render/vulkan_manager/vulkan_util.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"

#include <algorithm>
#include <stdexcept>

std::unordered_map<uint32_t, VkSampler> Pilot::PVulkanUtil::m_mipmap_sampler_map;
VkSampler                               Pilot::PVulkanUtil::m_nearest_sampler = VK_NULL_HANDLE;
VkSampler                               Pilot::PVulkanUtil::m_linear_sampler = VK_NULL_HANDLE;

uint32_t Pilot::PVulkanUtil::findMemoryType(VkPhysicalDevice      physical_device,
                                            uint32_t              type_filter,
                                            VkMemoryPropertyFlags properties_flag)
{
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);
    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
    {
        if (type_filter & (1 << i) &&
            (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties_flag) == properties_flag)
        {
            return i;
        }
    }
    throw std::runtime_error("findMemoryType");
}

VkShaderModule Pilot::PVulkanUtil::createShaderModule(VkDevice device, const std::vector<unsigned char>& shader_code)
{
    VkShaderModuleCreateInfo shader_module_create_info {};
    shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = shader_code.size();
    shader_module_create_info.pCode    = reinterpret_cast<const uint32_t*>(shader_code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &shader_module_create_info, nullptr, &shader_module) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }
    return shader_module;
}

void Pilot::PVulkanUtil::createBuffer(VkPhysicalDevice      physical_device,
                                      VkDevice              device,
                                      VkDeviceSize          size,
                                      VkBufferUsageFlags    usage,
                                      VkMemoryPropertyFlags properties,
                                      VkBuffer&             buffer,
                                      VkDeviceMemory&       buffer_memory)
{
    VkBufferCreateInfo buffer_create_info {};
    buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size        = size;
    buffer_create_info.usage       = usage;                     // use as a vertex/staging/index buffer
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing among queue families

    if (vkCreateBuffer(device, &buffer_create_info, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateBuffer");
    }

    VkMemoryRequirements buffer_memory_requirements; // for allocate_info.allocationSize and
                                                     // allocate_info.memoryTypeIndex
    vkGetBufferMemoryRequirements(device, buffer, &buffer_memory_requirements);

    VkMemoryAllocateInfo buffer_memory_allocate_info {};
    buffer_memory_allocate_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    buffer_memory_allocate_info.allocationSize = buffer_memory_requirements.size;
    buffer_memory_allocate_info.memoryTypeIndex =
        PVulkanUtil::findMemoryType(physical_device, buffer_memory_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &buffer_memory_allocate_info, nullptr, &buffer_memory) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateMemory");
    }

    // bind buffer with buffer memory
    vkBindBufferMemory(device, buffer, buffer_memory, 0); // offset = 0
}

void Pilot::PVulkanUtil::copyBuffer(PVulkanContext* context,
                                    VkBuffer        srcBuffer,
                                    VkBuffer        dstBuffer,
                                    VkDeviceSize    srcOffset,
                                    VkDeviceSize    dstOffset,
                                    VkDeviceSize    size)
{
    assert(context);

    VkCommandBuffer command_buffer = context->beginSingleTimeCommands();

    VkBufferCopy copyRegion = {srcOffset, dstOffset, size};
    vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copyRegion);

    context->endSingleTimeCommands(command_buffer);
}

void Pilot::PVulkanUtil::createImage(VkPhysicalDevice      physical_device,
                                     VkDevice              device,
                                     uint32_t              image_width,
                                     uint32_t              image_height,
                                     VkFormat              format,
                                     VkImageTiling         image_tiling,
                                     VkImageUsageFlags     image_usage_flags,
                                     VkMemoryPropertyFlags memory_property_flags,
                                     VkImage&              image,
                                     VkDeviceMemory&       memory,
                                     VkImageCreateFlags    image_create_flags,
                                     uint32_t              array_layers,
                                     uint32_t              miplevels)
{
    VkImageCreateInfo image_create_info {};
    image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.flags         = image_create_flags;
    image_create_info.imageType     = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width  = image_width;
    image_create_info.extent.height = image_height;
    image_create_info.extent.depth  = 1;
    image_create_info.mipLevels     = miplevels;
    image_create_info.arrayLayers   = array_layers;
    image_create_info.format        = format;
    image_create_info.tiling        = image_tiling;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage         = image_usage_flags;
    image_create_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &image_create_info, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physical_device, memRequirements.memoryTypeBits, memory_property_flags);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, memory, 0);
}

VkImageView Pilot::PVulkanUtil::createImageView(VkDevice           device,
                                                VkImage&           image,
                                                VkFormat           format,
                                                VkImageAspectFlags image_aspect_flags,
                                                VkImageViewType    view_type,
                                                uint32_t           layout_count,
                                                uint32_t           miplevels)
{
    VkImageViewCreateInfo image_view_create_info {};
    image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image                           = image;
    image_view_create_info.viewType                        = view_type;
    image_view_create_info.format                          = format;
    image_view_create_info.subresourceRange.aspectMask     = image_aspect_flags;
    image_view_create_info.subresourceRange.baseMipLevel   = 0;
    image_view_create_info.subresourceRange.levelCount     = miplevels;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount     = layout_count;

    VkImageView image_view;
    if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) != VK_SUCCESS)
    {
        return image_view;
        // todo
    }

    return image_view;
}

void Pilot::PVulkanUtil::transitionImageLayout(PVulkanContext*    context,
                                               VkImage            image,
                                               VkImageLayout      old_layout,
                                               VkImageLayout      new_layout,
                                               uint32_t           layer_count,
                                               uint32_t           miplevels,
                                               VkImageAspectFlags aspect_mask_bits)
{
    assert(context);

    VkCommandBuffer commandBuffer = context->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = old_layout;
    barrier.newLayout                       = new_layout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = aspect_mask_bits;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = miplevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = layer_count;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // for getGuidAndDepthOfMouseClickOnRenderSceneForUI() get depthimage
    else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    // for generating mipmapped image
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    context->endSingleTimeCommands(commandBuffer);
}

void Pilot::PVulkanUtil::copyBufferToImage(PVulkanContext* context,
                                           VkBuffer        buffer,
                                           VkImage         image,
                                           uint32_t        width,
                                           uint32_t        height,
                                           uint32_t        layer_count)
{
    assert(context);

    VkCommandBuffer commandBuffer = context->beginSingleTimeCommands();

    VkBufferImageCopy region {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = layer_count;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    context->endSingleTimeCommands(commandBuffer);
}

void Pilot::PVulkanUtil::genMipmappedImage(PVulkanContext* context,
                                           VkImage         image,
                                           uint32_t        width,
                                           uint32_t        height,
                                           uint32_t        mip_levels)
{
    assert(context);

    VkCommandBuffer commandBuffer = context->beginSingleTimeCommands();

    for (uint32_t i = 1; i < mip_levels; i++)
    {
        VkImageBlit imageBlit {};
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel   = i - 1;
        imageBlit.srcOffsets[1].x           = std::max((int32_t)(width >> (i - 1)), 1);
        imageBlit.srcOffsets[1].y           = std::max((int32_t)(height >> (i - 1)), 1);
        imageBlit.srcOffsets[1].z           = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel   = i;
        imageBlit.dstOffsets[1].x           = std::max((int32_t)(width >> i), 1);
        imageBlit.dstOffsets[1].y           = std::max((int32_t)(height >> i), 1);
        imageBlit.dstOffsets[1].z           = 1;

        VkImageSubresourceRange mipSubRange {};
        mipSubRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
        mipSubRange.baseMipLevel = i;
        mipSubRange.levelCount   = 1;
        mipSubRange.layerCount   = 1;

        VkImageMemoryBarrier barrier {};
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask       = 0;
        barrier.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = image;
        barrier.subresourceRange    = mipSubRange;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        vkCmdBlitImage(commandBuffer,
                       image,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &imageBlit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
    }

    VkImageSubresourceRange mipSubRange {};
    mipSubRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
    mipSubRange.baseMipLevel = 0;
    mipSubRange.levelCount   = mip_levels;
    mipSubRange.layerCount   = 1;

    VkImageMemoryBarrier barrier {};
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image;
    barrier.subresourceRange    = mipSubRange;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);

    context->endSingleTimeCommands(commandBuffer);
}

VkSampler Pilot::PVulkanUtil::getOrCreateMipmapSampler(VkPhysicalDevice physical_device,
                                                       VkDevice         device,
                                                       uint32_t         width,
                                                       uint32_t         height)
{
    assert(width > 0 && height > 0);

    VkSampler sampler;
    uint32_t  mip_levels   = floor(log2(std::max(width, height))) + 1;
    auto      find_sampler = m_mipmap_sampler_map.find(mip_levels);
    if (find_sampler != m_mipmap_sampler_map.end())
    {
        return find_sampler->second;
    }
    else
    {
        VkPhysicalDeviceProperties physical_device_properties {};
        vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

        VkSamplerCreateInfo samplerInfo {};
        samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter    = VK_FILTER_LINEAR;
        samplerInfo.minFilter    = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy    = physical_device_properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        samplerInfo.maxLod = mip_levels - 1;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            assert(0);
        }
    }

    m_mipmap_sampler_map.insert(std::make_pair(mip_levels, sampler));

    return sampler;
}

void Pilot::PVulkanUtil::destroyMipmappedSampler(VkDevice device)
{
    for (auto sampler : m_mipmap_sampler_map)
    {
        vkDestroySampler(device, sampler.second, nullptr);
    }
    m_mipmap_sampler_map.clear();
}

VkSampler Pilot::PVulkanUtil::getOrCreateNearestSampler(VkPhysicalDevice physical_device, VkDevice device)
{
    if (m_nearest_sampler == VK_NULL_HANDLE)
    {
        VkPhysicalDeviceProperties physical_device_properties {};
        vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

        VkSamplerCreateInfo samplerInfo {};

        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_NEAREST;
        samplerInfo.minFilter               = VK_FILTER_NEAREST;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.anisotropyEnable        = VK_FALSE;
        samplerInfo.maxAnisotropy           = physical_device_properties.limits.maxSamplerAnisotropy; // close :1.0f
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 8.0f; // todo: m_irradiance_texture_miplevels
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &m_nearest_sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }
    }

    return m_nearest_sampler;
}

VkSampler Pilot::PVulkanUtil::getOrCreateLinearSampler(VkPhysicalDevice physical_device, VkDevice device) 
{
    if (m_linear_sampler == VK_NULL_HANDLE)
    {
        VkPhysicalDeviceProperties physical_device_properties {};
        vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

        VkSamplerCreateInfo samplerInfo {};

        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.anisotropyEnable        = VK_FALSE;
        samplerInfo.maxAnisotropy           = physical_device_properties.limits.maxSamplerAnisotropy; // close :1.0f
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 8.0f; // todo: m_irradiance_texture_miplevels
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &m_linear_sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }
    }

    return m_linear_sampler;
}

void Pilot::PVulkanUtil::destroyNearestSampler(VkDevice device)
{
    vkDestroySampler(device, m_nearest_sampler, nullptr);
    m_nearest_sampler = VK_NULL_HANDLE;
}

void Pilot::PVulkanUtil::destroyLinearSampler(VkDevice device)
{
    vkDestroySampler(device, m_linear_sampler, nullptr);
    m_linear_sampler = VK_NULL_HANDLE;
}