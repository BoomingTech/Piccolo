#pragma once

#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_type.h"
#include "runtime/function/render/interface/rhi.h"

#include "runtime/function/render/render_common.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <map>
#include <vector>
#include <cmath>

namespace Piccolo
{
    class RHI;
    class RenderPassBase;
    class RenderCamera;

    struct IBLResource
    {
        RHIImage* _brdfLUT_texture_image;
        RHIImageView* _brdfLUT_texture_image_view;
        RHISampler* _brdfLUT_texture_sampler;
        VmaAllocation _brdfLUT_texture_image_allocation;

        RHIImage* _irradiance_texture_image;
        RHIImageView* _irradiance_texture_image_view;
        RHISampler* _irradiance_texture_sampler;
        VmaAllocation _irradiance_texture_image_allocation;

        RHIImage* _specular_texture_image;
        RHIImageView* _specular_texture_image_view;
        RHISampler* _specular_texture_sampler;
        VmaAllocation _specular_texture_image_allocation;
    };

    struct IBLResourceData
    {
        void* _brdfLUT_texture_image_pixels;
        uint32_t             _brdfLUT_texture_image_width;
        uint32_t             _brdfLUT_texture_image_height;
        RHIFormat   _brdfLUT_texture_image_format;
        std::array<void*, 6> _irradiance_texture_image_pixels;
        uint32_t             _irradiance_texture_image_width;
        uint32_t             _irradiance_texture_image_height;
        RHIFormat   _irradiance_texture_image_format;
        std::array<void*, 6> _specular_texture_image_pixels;
        uint32_t             _specular_texture_image_width;
        uint32_t             _specular_texture_image_height;
        RHIFormat   _specular_texture_image_format;
    };

    struct ColorGradingResource
    {
        RHIImage* _color_grading_LUT_texture_image;
        RHIImageView* _color_grading_LUT_texture_image_view;
        VmaAllocation _color_grading_LUT_texture_image_allocation;
    };

    struct ColorGradingResourceData
    {
        void* _color_grading_LUT_texture_image_pixels;
        uint32_t           _color_grading_LUT_texture_image_width;
        uint32_t           _color_grading_LUT_texture_image_height;
        RHIFormat _color_grading_LUT_texture_image_format;
    };

    struct StorageBuffer
    {
        // limits
        uint32_t _min_uniform_buffer_offset_alignment{ 256 };
        uint32_t _min_storage_buffer_offset_alignment{ 256 };
        uint32_t _max_storage_buffer_range{ 1 << 27 };
        uint32_t _non_coherent_atom_size{ 256 };

        RHIBuffer* _global_upload_ringbuffer;
        RHIDeviceMemory* _global_upload_ringbuffer_memory;
        void* _global_upload_ringbuffer_memory_pointer;
        std::vector<uint32_t> _global_upload_ringbuffers_begin;
        std::vector<uint32_t> _global_upload_ringbuffers_end;
        std::vector<uint32_t> _global_upload_ringbuffers_size;

        RHIBuffer* _global_null_descriptor_storage_buffer;
        RHIDeviceMemory* _global_null_descriptor_storage_buffer_memory;

        // axis
        RHIBuffer* _axis_inefficient_storage_buffer;
        RHIDeviceMemory* _axis_inefficient_storage_buffer_memory;
        void* _axis_inefficient_storage_buffer_memory_pointer;
    };

    struct GlobalRenderResource
    {
        IBLResource          _ibl_resource;
        ColorGradingResource _color_grading_resource;
        StorageBuffer        _storage_buffer;
    };

    class RenderResource : public RenderResourceBase
    {
    public:
        void clear() override final;

        virtual void uploadGlobalRenderResource(std::shared_ptr<RHI> rhi,
            LevelResourceDesc    level_resource_desc) override final;

        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi,
            RenderEntity         render_entity,
            RenderMeshData       mesh_data,
            RenderMaterialData   material_data) override final;

        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi,
            RenderEntity         render_entity,
            RenderMeshData       mesh_data) override final;

        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi,
            RenderEntity         render_entity,
            RenderMaterialData   material_data) override final;

        virtual void updatePerFrameBuffer(std::shared_ptr<RenderScene>  render_scene,
            std::shared_ptr<RenderCamera> camera) override final;

        VulkanMesh& getEntityMesh(RenderEntity entity);

        VulkanPBRMaterial& getEntityMaterial(RenderEntity entity);

        void resetRingBufferOffset(uint8_t current_frame_index);

        // global rendering resource, include IBL data, global storage buffer
        GlobalRenderResource m_global_render_resource;

        // storage buffer objects
        MeshPerframeStorageBufferObject                 m_mesh_perframe_storage_buffer_object;
        MeshPointLightShadowPerframeStorageBufferObject m_mesh_point_light_shadow_perframe_storage_buffer_object;
        MeshDirectionalLightShadowPerframeStorageBufferObject
                                                       m_mesh_directional_light_shadow_perframe_storage_buffer_object;
        AxisStorageBufferObject                        m_axis_storage_buffer_object;
        MeshInefficientPickPerframeStorageBufferObject m_mesh_inefficient_pick_perframe_storage_buffer_object;
        ParticleBillboardPerframeStorageBufferObject   m_particlebillboard_perframe_storage_buffer_object;
        ParticleCollisionPerframeStorageBufferObject   m_particle_collision_perframe_storage_buffer_object;

        // cached mesh and material
        std::map<size_t, VulkanMesh>        m_vulkan_meshes;
        std::map<size_t, VulkanPBRMaterial> m_vulkan_pbr_materials;

        // descriptor set layout in main camera pass will be used when uploading resource
        RHIDescriptorSetLayout* const* m_mesh_descriptor_set_layout {nullptr};
        RHIDescriptorSetLayout* const* m_material_descriptor_set_layout {nullptr};

    private:
        void createAndMapStorageBuffer(std::shared_ptr<RHI> rhi);
        void createIBLSamplers(std::shared_ptr<RHI> rhi);
        void createIBLTextures(std::shared_ptr<RHI>                        rhi,
                               std::array<std::shared_ptr<TextureData>, 6> irradiance_maps,
                               std::array<std::shared_ptr<TextureData>, 6> specular_maps);

        VulkanMesh& getOrCreateVulkanMesh(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMeshData mesh_data);
        VulkanPBRMaterial&
        getOrCreateVulkanMaterial(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMaterialData material_data);

        void updateMeshData(std::shared_ptr<RHI>                          rhi,
                            bool                                          enable_vertex_blending,
                            uint32_t                                      index_buffer_size,
                            void*                                         index_buffer_data,
                            uint32_t                                      vertex_buffer_size,
                            struct MeshVertexDataDefinition const*        vertex_buffer_data,
                            uint32_t                                      joint_binding_buffer_size,
                            struct MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                            VulkanMesh&                                   now_mesh);
        void updateVertexBuffer(std::shared_ptr<RHI>                          rhi,
                                bool                                          enable_vertex_blending,
                                uint32_t                                      vertex_buffer_size,
                                struct MeshVertexDataDefinition const*        vertex_buffer_data,
                                uint32_t                                      joint_binding_buffer_size,
                                struct MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                uint32_t                                      index_buffer_size,
                                uint16_t*                                     index_buffer_data,
                                VulkanMesh&                                   now_mesh);
        void updateIndexBuffer(std::shared_ptr<RHI> rhi,
                               uint32_t             index_buffer_size,
                               void*                index_buffer_data,
                               VulkanMesh&          now_mesh);
        void updateTextureImageData(std::shared_ptr<RHI> rhi, const TextureDataToUpdate& texture_data);
    };
} // namespace Piccolo
