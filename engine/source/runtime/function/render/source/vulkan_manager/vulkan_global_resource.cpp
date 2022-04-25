#include "runtime/function/render/include/render/vulkan_manager/vulkan_global_resource.h"
#include "runtime/function/render/include/render/render.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_util.h"

void Pilot::PGlobalRenderResource::initialize(PVulkanContext& context, int frames_in_flight)
{
    initializeIBLSamplers(context);
    initializeStorageBuffer(context, frames_in_flight);
    mapStorageBuffer(context);
}

Pilot::PIBLResourceData Pilot::PGlobalRenderResource::getIBLTextureData(Scene* scene, PilotRenderer* renderer)
{
    if (!scene || !renderer)
    {
        throw std::runtime_error("create IBL textures");
    }

    float empty_image[] = {0.5f, 0.5f, 0.5f, 0.5f};

    TextureHandle      brdfLUT_texture_handle       = scene->m_brdfLUT_texture_handle;
    const SceneImage*  brdfLUT_texture_image        = renderer->f_get_image(brdfLUT_texture_handle);
    void*              brdfLUT_texture_image_pixels = empty_image;
    uint32_t           brdfLUT_texture_image_width  = 1;
    uint32_t           brdfLUT_texture_image_height = 1;
    PILOT_PIXEL_FORMAT brdfLUT_texture_image_format = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32_FLOAT;
    if (brdfLUT_texture_image != NULL)
    {
        brdfLUT_texture_image_pixels = brdfLUT_texture_image->m_pixels;
        brdfLUT_texture_image_width  = static_cast<uint32_t>(brdfLUT_texture_image->m_width);
        brdfLUT_texture_image_height = static_cast<uint32_t>(brdfLUT_texture_image->m_height);
        brdfLUT_texture_image_format = brdfLUT_texture_image->m_format;
    }

    void*              irradiance_texture_image_pixels[6];
    void*              specular_texture_image_pixels[6];
    uint32_t           irradiance_texture_image_width  = 1;
    uint32_t           irradiance_texture_image_height = 1;
    PILOT_PIXEL_FORMAT irradiance_texture_image_format = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32_FLOAT;
    uint32_t           specular_texture_image_width    = 1;
    uint32_t           specular_texture_image_height   = 1;
    PILOT_PIXEL_FORMAT specular_texture_image_format   = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32_FLOAT;
    for (int i = 0; i < 6; i++)
    {
        TextureHandle     irradiance_texture_handle = scene->m_irradiance_texture_handle[i];
        const SceneImage* irradiance_texture_image  = renderer->f_get_image(irradiance_texture_handle);
        irradiance_texture_image_pixels[i]          = empty_image;
        if (irradiance_texture_image != NULL)
        {
            irradiance_texture_image_pixels[i] = irradiance_texture_image->m_pixels;
            irradiance_texture_image_width     = static_cast<uint32_t>(irradiance_texture_image->m_width);
            irradiance_texture_image_height    = static_cast<uint32_t>(irradiance_texture_image->m_height);
            irradiance_texture_image_format    = irradiance_texture_image->m_format;
        }

        TextureHandle     specular_texture_handle = scene->m_specular_texture_handle[i];
        const SceneImage* specular_texture_image  = renderer->f_get_image(specular_texture_handle);
        specular_texture_image_pixels[i]          = empty_image;
        if (specular_texture_image != NULL)
        {
            specular_texture_image_pixels[i] = specular_texture_image->m_pixels;
            specular_texture_image_width     = static_cast<uint32_t>(specular_texture_image->m_width);
            specular_texture_image_height    = static_cast<uint32_t>(specular_texture_image->m_height);
            specular_texture_image_format    = specular_texture_image->m_format;
        }
    }

    Pilot::PIBLResourceData ibl_resource_data;

    ibl_resource_data._brdfLUT_texture_image_pixels = brdfLUT_texture_image_pixels;
    ibl_resource_data._brdfLUT_texture_image_width  = brdfLUT_texture_image_width;
    ibl_resource_data._brdfLUT_texture_image_height = brdfLUT_texture_image_height;
    ibl_resource_data._brdfLUT_texture_image_format = brdfLUT_texture_image_format;

    for (int i = 0; i < 6; i++)
    {
        ibl_resource_data._irradiance_texture_image_pixels[i] = irradiance_texture_image_pixels[i];
        ibl_resource_data._specular_texture_image_pixels[i]   = specular_texture_image_pixels[i];
    }

    ibl_resource_data._irradiance_texture_image_width  = irradiance_texture_image_width;
    ibl_resource_data._irradiance_texture_image_height = irradiance_texture_image_height;
    ibl_resource_data._irradiance_texture_image_format = irradiance_texture_image_format;

    ibl_resource_data._specular_texture_image_width  = specular_texture_image_width;
    ibl_resource_data._specular_texture_image_height = specular_texture_image_height;
    ibl_resource_data._specular_texture_image_format = specular_texture_image_format;

    return ibl_resource_data;
}

Pilot::PColorGradingResourceData Pilot::PGlobalRenderResource::getColorGradingTextureData(Scene*               scene,
                                                                                          class PilotRenderer* renderer)
{
    if (!scene || !renderer)
    {
        throw std::runtime_error("create color grading textures");
    }

    float empty_image[] = {0.5f, 0.5f, 0.5f, 0.5f};

    TextureHandle      color_grading_LUT_texture_handle       = scene->m_color_grading_LUT_texture_handle;
    const SceneImage*  color_grading_LUT_texture_image        = renderer->f_get_image(color_grading_LUT_texture_handle);
    void*              color_grading_LUT_texture_image_pixels = empty_image;
    uint32_t           color_grading_LUT_texture_image_width  = 1;
    uint32_t           color_grading_LUT_texture_image_height = 1;
    PILOT_PIXEL_FORMAT color_grading_LUT_texture_image_format = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32_FLOAT;
    if (color_grading_LUT_texture_image != NULL)
    {
        color_grading_LUT_texture_image_pixels = color_grading_LUT_texture_image->m_pixels;
        color_grading_LUT_texture_image_width  = static_cast<uint32_t>(color_grading_LUT_texture_image->m_width);
        color_grading_LUT_texture_image_height = static_cast<uint32_t>(color_grading_LUT_texture_image->m_height);
        color_grading_LUT_texture_image_format = color_grading_LUT_texture_image->m_format;
    }

    Pilot::PColorGradingResourceData color_grading_resource_data;

    color_grading_resource_data._color_grading_LUT_texture_image_pixels = color_grading_LUT_texture_image_pixels;
    color_grading_resource_data._color_grading_LUT_texture_image_width  = color_grading_LUT_texture_image_width;
    color_grading_resource_data._color_grading_LUT_texture_image_height = color_grading_LUT_texture_image_height;
    color_grading_resource_data._color_grading_LUT_texture_image_format = color_grading_LUT_texture_image_format;

    return color_grading_resource_data;
}

void Pilot::PGlobalRenderResource::clear(PVulkanContext& context)
{
    vmaDestroyImage(context._assets_allocator,
                    _ibl_resource._brdfLUT_texture_image,
                    _ibl_resource._brdfLUT_texture_image_allocation);
    vmaDestroyImage(context._assets_allocator,
                    _ibl_resource._irradiance_texture_image,
                    _ibl_resource._irradiance_texture_image_allocation);
    vmaDestroyImage(context._assets_allocator,
                    _ibl_resource._specular_texture_image,
                    _ibl_resource._specular_texture_image_allocation);
    vkDestroySampler(context._device, _ibl_resource._brdfLUT_texture_sampler, NULL);
    vkDestroySampler(context._device, _ibl_resource._irradiance_texture_sampler, NULL);
    vkDestroySampler(context._device, _ibl_resource._specular_texture_sampler, NULL);

    vmaDestroyImage(context._assets_allocator,
                    _color_grading_resource._color_grading_LUT_texture_image,
                    _color_grading_resource._color_grading_LUT_texture_image_allocation);
}

void Pilot::PGlobalRenderResource::initializeIBLSamplers(PVulkanContext& context)
{
    VkPhysicalDeviceProperties physical_device_properties {};
    vkGetPhysicalDeviceProperties(context._physical_device, &physical_device_properties);

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    samplerInfo.anisotropyEnable = VK_TRUE;                                                // close:false
    samplerInfo.maxAnisotropy    = physical_device_properties.limits.maxSamplerAnisotropy; // close :1.0f

    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    // global textures for IBL
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(context._device, &samplerInfo, nullptr, &_ibl_resource._brdfLUT_texture_sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("vk create sampler");
    }

    samplerInfo.minLod     = 0.0f;
    samplerInfo.maxLod     = 8.0f; // todo: m_irradiance_texture_miplevels
    samplerInfo.mipLodBias = 0.0f;
    if (vkCreateSampler(context._device, &samplerInfo, nullptr, &_ibl_resource._irradiance_texture_sampler) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("vk create sampler");
    }
    if (vkCreateSampler(context._device, &samplerInfo, nullptr, &_ibl_resource._specular_texture_sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("vk create sampler");
    }
}

void Pilot::PGlobalRenderResource::initializeStorageBuffer(PVulkanContext& context, int frames_in_flight)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(context._physical_device, &properties);

    _storage_buffer._min_uniform_buffer_offset_alignment =
        static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
    _storage_buffer._min_storage_buffer_offset_alignment =
        static_cast<uint32_t>(properties.limits.minStorageBufferOffsetAlignment);
    _storage_buffer._max_storage_buffer_range = properties.limits.maxStorageBufferRange;
    _storage_buffer._non_coherent_atom_size   = properties.limits.nonCoherentAtomSize;

    // In Vulkan, the storage buffer should be pre-allocated.
    // The size is 128MB in NVIDIA D3D11
    // driver(https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0).
    uint32_t global_storage_buffer_size = 1024 * 1024 * 128;
    PVulkanUtil::createBuffer(context._physical_device,
                              context._device,
                              global_storage_buffer_size,
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              _storage_buffer._global_upload_ringbuffer,
                              _storage_buffer._global_upload_ringbuffer_memory);

    _storage_buffer._global_upload_ringbuffers_begin.resize(frames_in_flight);
    _storage_buffer._global_upload_ringbuffers_end.resize(frames_in_flight);
    _storage_buffer._global_upload_ringbuffers_size.resize(frames_in_flight);
    for (uint32_t i = 0; i < frames_in_flight; ++i)
    {
        _storage_buffer._global_upload_ringbuffers_begin[i] = (global_storage_buffer_size * i) / frames_in_flight;
        _storage_buffer._global_upload_ringbuffers_size[i] = (global_storage_buffer_size * (i + 1)) / frames_in_flight -
                                                             (global_storage_buffer_size * i) / frames_in_flight;
    }

    // axis
    PVulkanUtil::createBuffer(context._physical_device,
                              context._device,
                              sizeof(AxisStorageBufferObject),
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              _storage_buffer._axis_inefficient_storage_buffer,
                              _storage_buffer._axis_inefficient_storage_buffer_memory);

    // null descriptor
    PVulkanUtil::createBuffer(context._physical_device,
                              context._device,
                              64,
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              0,
                              _storage_buffer._global_null_descriptor_storage_buffer,
                              _storage_buffer._global_null_descriptor_storage_buffer_memory);
    static_assert(64 >= sizeof(PMeshVertex::VulkanMeshVertexJointBinding), "");
}

void Pilot::PGlobalRenderResource::mapStorageBuffer(PVulkanContext& context)
{
    // TODO: Unmap when program terminates
    vkMapMemory(context._device,
                _storage_buffer._global_upload_ringbuffer_memory,
                0,
                VK_WHOLE_SIZE,
                0,
                &_storage_buffer._global_upload_ringbuffer_memory_pointer);

    vkMapMemory(context._device,
                _storage_buffer._axis_inefficient_storage_buffer_memory,
                0,
                VK_WHOLE_SIZE,
                0,
                &_storage_buffer._axis_inefficient_storage_buffer_memory_pointer);
}

void Pilot::PGlobalRenderResource::unmapStorageBuffer(PVulkanContext& context) {}
