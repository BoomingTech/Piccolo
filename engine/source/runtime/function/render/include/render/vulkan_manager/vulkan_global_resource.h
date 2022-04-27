#pragma once

#include "vulkan_context.h"
#include <vulkan/vulkan.h>

#include <array>
#include <vector>

#include "runtime/function/render/include/render/framebuffer.h"

namespace Pilot
{
    struct PIBLResource
    {
        VkImage       _brdfLUT_texture_image      = VK_NULL_HANDLE;
        VkImageView   _brdfLUT_texture_image_view = VK_NULL_HANDLE;
        VkSampler     _brdfLUT_texture_sampler    = VK_NULL_HANDLE;
        VmaAllocation _brdfLUT_texture_image_allocation;

        VkImage       _irradiance_texture_image      = VK_NULL_HANDLE;
        VkImageView   _irradiance_texture_image_view = VK_NULL_HANDLE;
        VkSampler     _irradiance_texture_sampler    = VK_NULL_HANDLE;
        VmaAllocation _irradiance_texture_image_allocation;

        VkImage       _specular_texture_image      = VK_NULL_HANDLE;
        VkImageView   _specular_texture_image_view = VK_NULL_HANDLE;
        VkSampler     _specular_texture_sampler    = VK_NULL_HANDLE;
        VmaAllocation _specular_texture_image_allocation;
    };

    struct PIBLResourceData
    {
        void*                _brdfLUT_texture_image_pixels;
        uint32_t             _brdfLUT_texture_image_width;
        uint32_t             _brdfLUT_texture_image_height;
        PILOT_PIXEL_FORMAT   _brdfLUT_texture_image_format;
        std::array<void*, 6> _irradiance_texture_image_pixels;
        uint32_t             _irradiance_texture_image_width;
        uint32_t             _irradiance_texture_image_height;
        PILOT_PIXEL_FORMAT   _irradiance_texture_image_format;
        std::array<void*, 6> _specular_texture_image_pixels;
        uint32_t             _specular_texture_image_width;
        uint32_t             _specular_texture_image_height;
        PILOT_PIXEL_FORMAT   _specular_texture_image_format;
    };

    struct PColorGradingResource
    {
        VkImage       _color_grading_LUT_texture_image      = VK_NULL_HANDLE;
        VkImageView   _color_grading_LUT_texture_image_view = VK_NULL_HANDLE;
        VmaAllocation _color_grading_LUT_texture_image_allocation;
    };

    struct PColorGradingResourceData
    {
        void*              _color_grading_LUT_texture_image_pixels;
        uint32_t           _color_grading_LUT_texture_image_width;
        uint32_t           _color_grading_LUT_texture_image_height;
        PILOT_PIXEL_FORMAT _color_grading_LUT_texture_image_format;
    };

    struct PStorageBuffer
    {
        // limits
        uint32_t _min_uniform_buffer_offset_alignment = 256;
        uint32_t _min_storage_buffer_offset_alignment = 256;
        uint32_t _max_storage_buffer_range            = 1 << 27;
        uint32_t _non_coherent_atom_size              = 256;

        VkBuffer              _global_upload_ringbuffer;
        VkDeviceMemory        _global_upload_ringbuffer_memory;
        void*                 _global_upload_ringbuffer_memory_pointer;
        std::vector<uint32_t> _global_upload_ringbuffers_begin;
        std::vector<uint32_t> _global_upload_ringbuffers_end;
        std::vector<uint32_t> _global_upload_ringbuffers_size;

        VkBuffer       _global_null_descriptor_storage_buffer;
        VkDeviceMemory _global_null_descriptor_storage_buffer_memory;

        // axis
        VkBuffer       _axis_inefficient_storage_buffer;
        VkDeviceMemory _axis_inefficient_storage_buffer_memory;
        void*          _axis_inefficient_storage_buffer_memory_pointer;
    };

    class PGlobalRenderResource
    {
    public:
        PIBLResource          _ibl_resource;
        PColorGradingResource _color_grading_resource;
        PStorageBuffer        _storage_buffer;

        void                      initialize(PVulkanContext& context, int frames_in_flight = 3);
        PIBLResourceData          getIBLTextureData(Scene* scene, class PilotRenderer* renderer);
        PColorGradingResourceData getColorGradingTextureData(Scene* scene, class PilotRenderer* renderer);
        void                      clear(PVulkanContext& context);

    private:
        void initializeIBLSamplers(PVulkanContext& context);
        void initializeStorageBuffer(PVulkanContext& context, int frames_in_flight);
        void mapStorageBuffer(PVulkanContext& context);
        void unmapStorageBuffer(PVulkanContext& context);
    };
} // namespace Pilot
