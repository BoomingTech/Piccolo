#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

#include "runtime/function/render/render_type.h"

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Pilot
{
    static const uint32_t m_point_light_shadow_map_dimension       = 2048;
    static const uint32_t m_directional_light_shadow_map_dimension = 4096;

    // TODO: 64 may not be the best
    static uint32_t const m_mesh_per_drawcall_max_instance_count = 64;
    static uint32_t const m_mesh_vertex_blending_max_joint_count = 1024;
    static uint32_t const m_max_point_light_count                = 15;
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
        glm::mat4                   proj_view_matrix;
        glm::vec3                   camera_position;
        float                       _padding_camera_position;
        Vector3                     ambient_light;
        float                       _padding_ambient_light;
        uint32_t                    point_light_num;
        uint32_t                    _padding_point_light_num_1;
        uint32_t                    _padding_point_light_num_2;
        uint32_t                    _padding_point_light_num_3;
        VulkanScenePointLight       scene_point_lights[m_max_point_light_count];
        VulkanSceneDirectionalLight scene_directional_light;
        glm::mat4                   directional_light_proj_view;
    };

    struct VulkanMeshInstance
    {
        float     enable_vertex_blending;
        float     _padding_enable_vertex_blending_1;
        float     _padding_enable_vertex_blending_2;
        float     _padding_enable_vertex_blending_3;
        glm::mat4 model_matrix;
    };

    struct MeshPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[m_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPerdrawcallVertexBlendingStorageBufferObject
    {
        glm::mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
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
        Vector4  point_lights_position_and_radius[m_max_point_light_count];
    };

    struct MeshPointLightShadowPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[m_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPointLightShadowPerdrawcallVertexBlendingStorageBufferObject
    {
        glm::mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
    };

    struct MeshDirectionalLightShadowPerframeStorageBufferObject
    {
        glm::mat4 light_proj_view;
    };

    struct MeshDirectionalLightShadowPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[m_mesh_per_drawcall_max_instance_count];
    };

    struct MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject
    {
        glm::mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
    };

    struct AxisStorageBufferObject
    {
        glm::mat4 model_matrix  = glm::mat4(1.0);
        uint32_t  selected_axis = 3;
    };

    struct ParticleBillboardPerframeStorageBufferObject
    {
        glm::mat4 proj_view_matrix;
        glm::vec3 eye_position;
        float     _padding_eye_position;
        glm::vec3 up_direction;
        float     _padding_up_direction;
    };

    struct ParticleBillboardPerdrawcallStorageBufferObject
    {
        // TODO: 4096 may not be the best
        Vector4 positions[4096];
    };

    struct MeshInefficientPickPerframeStorageBufferObject
    {
        glm::mat4 proj_view_matrix;
        uint32_t  rt_width;
        uint32_t  rt_height;
    };

    struct MeshInefficientPickPerdrawcallStorageBufferObject
    {
        glm::mat4 model_matrices[m_mesh_per_drawcall_max_instance_count];
        uint32_t  node_ids[m_mesh_per_drawcall_max_instance_count];
        float     enable_vertex_blendings[m_mesh_per_drawcall_max_instance_count];
    };

    struct MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject
    {
        glm::mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
    };

    // mesh
    struct VulkanMesh
    {
        bool enable_vertex_blending;

        uint32_t mesh_vertex_count;

        VkBuffer      mesh_vertex_position_buffer;
        VmaAllocation mesh_vertex_position_buffer_allocation;

        VkBuffer      mesh_vertex_varying_enable_blending_buffer;
        VmaAllocation mesh_vertex_varying_enable_blending_buffer_allocation;

        VkBuffer      mesh_vertex_joint_binding_buffer;
        VmaAllocation mesh_vertex_joint_binding_buffer_allocation;

        VkDescriptorSet mesh_vertex_blending_descriptor_set;

        VkBuffer      mesh_vertex_varying_buffer;
        VmaAllocation mesh_vertex_varying_buffer_allocation;

        uint32_t mesh_index_count;

        VkBuffer      mesh_index_buffer;
        VmaAllocation mesh_index_buffer_allocation;
    };

    // material
    struct VulkanPBRMaterial
    {
        VkImage       base_color_texture_image = VK_NULL_HANDLE;
        VkImageView   base_color_image_view    = VK_NULL_HANDLE;
        VmaAllocation base_color_image_allocation;

        VkImage       metallic_roughness_texture_image = VK_NULL_HANDLE;
        VkImageView   metallic_roughness_image_view    = VK_NULL_HANDLE;
        VmaAllocation metallic_roughness_image_allocation;

        VkImage       normal_texture_image = VK_NULL_HANDLE;
        VkImageView   normal_image_view    = VK_NULL_HANDLE;
        VmaAllocation normal_image_allocation;

        VkImage       occlusion_texture_image = VK_NULL_HANDLE;
        VkImageView   occlusion_image_view    = VK_NULL_HANDLE;
        VmaAllocation occlusion_image_allocation;

        VkImage       emissive_texture_image = VK_NULL_HANDLE;
        VkImageView   emissive_image_view    = VK_NULL_HANDLE;
        VmaAllocation emissive_image_allocation;

        VkBuffer      material_uniform_buffer;
        VmaAllocation material_uniform_buffer_allocation;

        VkDescriptorSet material_descriptor_set;
    };

    // nodes
    struct RenderMeshNode
    {
        glm::mat4          model_matrix;
        glm::mat4          joint_matrices[m_mesh_vertex_blending_max_joint_count];
        VulkanMesh*        ref_mesh     = nullptr;
        VulkanPBRMaterial* ref_material = nullptr;
        uint32_t           node_id;
        bool               enable_vertex_blending = false;
    };

    struct RenderAxisNode
    {
        glm::mat4   model_matrix {glm::mat4(1.0f)};
        VulkanMesh* ref_mesh {nullptr};
        uint32_t    node_id;
        bool        enable_vertex_blending {false};
    };

    struct RenderParticleBillboardNode
    {
        std::vector<Vector4> positions;
    };

    struct TextureDataToUpdate
    {
        void*              base_color_image_pixels;
        uint32_t           base_color_image_width;
        uint32_t           base_color_image_height;
        PILOT_PIXEL_FORMAT base_color_image_format;
        void*              metallic_roughness_image_pixels;
        uint32_t           metallic_roughness_image_width;
        uint32_t           metallic_roughness_image_height;
        PILOT_PIXEL_FORMAT metallic_roughness_image_format;
        void*              normal_roughness_image_pixels;
        uint32_t           normal_roughness_image_width;
        uint32_t           normal_roughness_image_height;
        PILOT_PIXEL_FORMAT normal_roughness_image_format;
        void*              occlusion_image_pixels;
        uint32_t           occlusion_image_width;
        uint32_t           occlusion_image_height;
        PILOT_PIXEL_FORMAT occlusion_image_format;
        void*              emissive_image_pixels;
        uint32_t           emissive_image_width;
        uint32_t           emissive_image_height;
        PILOT_PIXEL_FORMAT emissive_image_format;
        VulkanPBRMaterial* now_material;
    };
} // namespace Pilot
