#pragma once

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

#include "runtime/function/render/render_type.h"
#include "interface/rhi.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Piccolo
{
    static const uint32_t s_point_light_shadow_map_dimension       = 2048;
    static const uint32_t s_directional_light_shadow_map_dimension = 4096;

    // TODO: 64 may not be the best
    static uint32_t const s_mesh_per_drawcall_max_instance_count = 64;
    static uint32_t const s_mesh_vertex_blending_max_joint_count = 1024;
    static uint32_t const s_max_point_light_count                = 15;
    // should sync the macros in "shader_include/constants.h"

    struct VulkanSceneDirectionalLight
    {
        Vector3 direction;
        float   _padding_direction;
        Vector3 color;
        float   _padding_color;
    };

    struct VulkanScenePointLight
    {
        Vector3 position;
        float   radius;
        Vector3 intensity;
        float   _padding_intensity;
    };

    struct MeshPerframeStorageBufferObject
    {
        Matrix4x4                   proj_view_matrix;
        Vector3                     camera_position;
        float                       _padding_camera_position;
        Vector3                     ambient_light;
        float                       _padding_ambient_light;
        uint32_t                    point_light_num;
        uint32_t                    _padding_point_light_num_1;
        uint32_t                    _padding_point_light_num_2;
        uint32_t                    _padding_point_light_num_3;
        VulkanScenePointLight       scene_point_lights[s_max_point_light_count];
        VulkanSceneDirectionalLight scene_directional_light;
        Matrix4x4                   directional_light_proj_view;
    };

    struct VulkanMeshInstance
    {
        float     enable_vertex_blending;
        float     _padding_enable_vertex_blending_1;
        float     _padding_enable_vertex_blending_2;
        float     _padding_enable_vertex_blending_3;
        Matrix4x4 model_matrix;
    };

    struct MeshPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPerMaterialUniformBufferObject
    {
        Vector4 baseColorFactor {0.0f, 0.0f, 0.0f, 0.0f};

        float metallicFactor    = 0.0f;
        float roughnessFactor   = 0.0f;
        float normalScale       = 0.0f;
        float occlusionStrength = 0.0f;

        Vector3  emissiveFactor  = {0.0f, 0.0f, 0.0f};
        uint32_t is_blend        = 0;
        uint32_t is_double_sided = 0;
    };

    struct MeshPointLightShadowPerframeStorageBufferObject
    {
        uint32_t point_light_num;
        uint32_t _padding_point_light_num_1;
        uint32_t _padding_point_light_num_2;
        uint32_t _padding_point_light_num_3;
        Vector4  point_lights_position_and_radius[s_max_point_light_count];
    };

    struct MeshPointLightShadowPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPointLightShadowPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshDirectionalLightShadowPerframeStorageBufferObject
    {
        Matrix4x4 light_proj_view;
    };

    struct MeshDirectionalLightShadowPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    struct AxisStorageBufferObject
    {
        Matrix4x4 model_matrix  = Matrix4x4::IDENTITY;
        uint32_t  selected_axis = 3;
    };

    struct ParticleBillboardPerframeStorageBufferObject
    {
        Matrix4x4 proj_view_matrix;
        Vector3   right_direction;
        float     _padding_right_position;
        Vector3   up_direction;
        float     _padding_up_direction;
        Vector3   foward_direction;
        float     _padding_forward_position;
    };

    struct ParticleCollisionPerframeStorageBufferObject
    {
        Matrix4x4 view_matrix;
        Matrix4x4 proj_view_matrix;
        Matrix4x4 proj_inv_matrix;
    };

    // TODO: 4096 may not be the best
    static constexpr int s_particle_billboard_buffer_size = 4096;
    struct ParticleBillboardPerdrawcallStorageBufferObject
    {
        Vector4 positions[s_particle_billboard_buffer_size];
        Vector4 sizes[s_particle_billboard_buffer_size];
        Vector4 colors[s_particle_billboard_buffer_size];
    };

    struct MeshInefficientPickPerframeStorageBufferObject
    {
        Matrix4x4 proj_view_matrix;
        uint32_t  rt_width;
        uint32_t  rt_height;
    };

    struct MeshInefficientPickPerdrawcallStorageBufferObject
    {
        Matrix4x4 model_matrices[s_mesh_per_drawcall_max_instance_count];
        uint32_t  node_ids[s_mesh_per_drawcall_max_instance_count];
        float     enable_vertex_blendings[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    // mesh
    struct VulkanMesh
    {
        bool enable_vertex_blending;

        uint32_t mesh_vertex_count;

        RHIBuffer*    mesh_vertex_position_buffer;
        VmaAllocation mesh_vertex_position_buffer_allocation;

        RHIBuffer*    mesh_vertex_varying_enable_blending_buffer;
        VmaAllocation mesh_vertex_varying_enable_blending_buffer_allocation;

        RHIBuffer*    mesh_vertex_joint_binding_buffer;
        VmaAllocation mesh_vertex_joint_binding_buffer_allocation;

        RHIDescriptorSet* mesh_vertex_blending_descriptor_set;

        RHIBuffer*    mesh_vertex_varying_buffer;
        VmaAllocation mesh_vertex_varying_buffer_allocation;

        uint32_t mesh_index_count;

        RHIBuffer*    mesh_index_buffer;
        VmaAllocation mesh_index_buffer_allocation;
    };

    // material
    struct VulkanPBRMaterial
    {
        RHIImage*       base_color_texture_image;
        RHIImageView*   base_color_image_view;
        VmaAllocation   base_color_image_allocation;

        RHIImage*       metallic_roughness_texture_image;
        RHIImageView*   metallic_roughness_image_view;
        VmaAllocation   metallic_roughness_image_allocation;

        RHIImage*       normal_texture_image;
        RHIImageView*   normal_image_view;
        VmaAllocation   normal_image_allocation;

        RHIImage*       occlusion_texture_image;
        RHIImageView*   occlusion_image_view;
        VmaAllocation   occlusion_image_allocation;

        RHIImage*       emissive_texture_image;
        RHIImageView*   emissive_image_view;
        VmaAllocation   emissive_image_allocation;

        RHIBuffer*      material_uniform_buffer;
        VmaAllocation   material_uniform_buffer_allocation;

        RHIDescriptorSet* material_descriptor_set;
    };

    // nodes
    struct RenderMeshNode
    {
        const Matrix4x4*   model_matrix {nullptr};
        const Matrix4x4*   joint_matrices {nullptr};
        uint32_t           joint_count {0};
        VulkanMesh*        ref_mesh {nullptr};
        VulkanPBRMaterial* ref_material {nullptr};
        uint32_t           node_id;
        bool               enable_vertex_blending {false};
    };

    struct RenderAxisNode
    {
        Matrix4x4   model_matrix {Matrix4x4::IDENTITY};
        VulkanMesh* ref_mesh {nullptr};
        uint32_t    node_id;
        bool        enable_vertex_blending {false};
    };

    struct TextureDataToUpdate
    {
        void*              base_color_image_pixels;
        uint32_t           base_color_image_width;
        uint32_t           base_color_image_height;
        RHIFormat base_color_image_format;
        void*              metallic_roughness_image_pixels;
        uint32_t           metallic_roughness_image_width;
        uint32_t           metallic_roughness_image_height;
        RHIFormat metallic_roughness_image_format;
        void*              normal_roughness_image_pixels;
        uint32_t           normal_roughness_image_width;
        uint32_t           normal_roughness_image_height;
        RHIFormat normal_roughness_image_format;
        void*              occlusion_image_pixels;
        uint32_t           occlusion_image_width;
        uint32_t           occlusion_image_height;
        RHIFormat occlusion_image_format;
        void*              emissive_image_pixels;
        uint32_t           emissive_image_width;
        uint32_t           emissive_image_height;
        RHIFormat emissive_image_format;
        VulkanPBRMaterial* now_material;
    };
} // namespace Piccolo
