#include "runtime/function/render/include/render/render.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include "runtime/function/render/include/render/glm_wrapper.h"

void Pilot::PVulkanManager::culling(class Scene&                scene,
                                    class PilotRenderer*        pilot_renderer,
                                    struct SceneReleaseHandles& release_handles)
{
    scene.lock();

    // Directional Light Mesh
    {
        glm::mat4 directional_light_proj_view = calculate_directional_light_camera(scene);

        m_mesh_perframe_storage_buffer_object.directional_light_proj_view              = directional_light_proj_view;
        m_mesh_directional_light_shadow_perframe_storage_buffer_object.light_proj_view = directional_light_proj_view;

        m_directional_light_visible_mesh_nodes.clear();

        cluster_frustum_t frustum =
            cluster_frustum_create_from_mat(directional_light_proj_view, -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

        for (RenderMesh const& mesh : scene.getMeshes())
        {
            bounding_box_t mesh_asset_bounding_box {mesh.m_bounding_box.getMinCorner(),
                                                    mesh.m_bounding_box.getMaxCorner()};

            // Frustum Culling
            if (tiled_frustum_intersect_box(
                    frustum, bounding_box_transform(mesh_asset_bounding_box, GLMUtil::fromMat4x4(mesh.m_model_matrix))))
            {
                PVulkanMeshNode temp_node;

                temp_node.model_matrix = GLMUtil::fromMat4x4(mesh.m_model_matrix);

                assert(mesh.m_joint_matrices.size() <= m_mesh_vertex_blending_max_joint_count);
                for (size_t joint_index = 0; joint_index < mesh.m_joint_matrices.size(); ++joint_index)
                {
                    temp_node.joint_matrices[joint_index] = GLMUtil::fromMat4x4(mesh.m_joint_matrices[joint_index]);
                }
                temp_node.node_id = mesh.m_instance_id;

                SkeletonBindingBufferHandle invalid_skeleton_binding_handle = PILOT_INVALID_HANDLE;
                bool has_skeleton_binding_handle = (!(invalid_skeleton_binding_handle == mesh.skeleton_binding_handle));

                VulkanMesh& mesh_asset           = this->syncMesh(mesh, has_skeleton_binding_handle, pilot_renderer);
                temp_node.ref_mesh               = &mesh_asset;
                temp_node.enable_vertex_blending = mesh.m_enable_vertex_blending_1;

                const auto&        material       = scene.getMaterials()[mesh.m_material];
                VulkanPBRMaterial& material_asset = this->syncMaterial(material, pilot_renderer);
                temp_node.ref_material            = &material_asset;

                m_directional_light_visible_mesh_nodes.push_back(temp_node);

                // upload mesh and material data finished, can be cleared
                MeshHandle mesh_handle;
                mesh_handle.m_vertex_handle = mesh.m_vertexBuffer;
                mesh_handle.m_index_handle  = mesh.m_indexBuffer;
                release_handles.mesh_handles.push_back(mesh_handle);

                if (has_skeleton_binding_handle)
                    release_handles.skeleton_binding_handles.push_back(mesh.skeleton_binding_handle);

                PMaterialHandle material_handle;
                material_handle.m_image_handle0 = material.m_baseColorTexture;
                material_handle.m_image_handle1 = material.m_metallicRoughnessTexture;
                material_handle.m_image_handle2 = material.m_normalTexture;
                material_handle.m_image_handle3 = material.m_occlusionTexture;
                material_handle.m_image_handle4 = material.m_emissiveTexture;
                release_handles.material_handles.push_back(material_handle);
            }
        }
    }

    // Point Lights Mesh
    {
        m_point_lights_visible_mesh_nodes.clear();

        std::vector<bounding_sphere_t> point_lights_bounding_spheres;
        uint32_t                       point_light_num = static_cast<uint32_t>(scene.m_pointLights.m_lights.size());
        point_lights_bounding_spheres.resize(point_light_num);
        for (size_t i = 0; i < point_light_num; i++)
        {
            point_lights_bounding_spheres[i].m_center = GLMUtil::fromVec3(scene.m_pointLights.m_lights[i].m_position);
            point_lights_bounding_spheres[i].m_radius = scene.m_pointLights.m_lights[i].calculateRadius();
        }

        for (RenderMesh const& mesh : scene.getMeshes())
        {
            bounding_box_t mesh_asset_bounding_box {mesh.m_bounding_box.getMinCorner(),
                                                    mesh.m_bounding_box.getMaxCorner()};

            // Culling
            bool intersect_with_point_lights = true;
            for (size_t i = 0; i < point_light_num; i++)
            {
                if (!box_intersect_sphere(
                        bounding_box_transform(mesh_asset_bounding_box, GLMUtil::fromMat4x4(mesh.m_model_matrix)),
                        point_lights_bounding_spheres[i]))
                {
                    intersect_with_point_lights = false;
                    break;
                }
            }

            if (intersect_with_point_lights)
            {
                PVulkanMeshNode temp_node;

                temp_node.model_matrix = GLMUtil::fromMat4x4(mesh.m_model_matrix);

                assert(mesh.m_joint_matrices.size() <= m_mesh_vertex_blending_max_joint_count);
                for (size_t joint_index = 0; joint_index < mesh.m_joint_matrices.size(); ++joint_index)
                {
                    temp_node.joint_matrices[joint_index] = GLMUtil::fromMat4x4(mesh.m_joint_matrices[joint_index]);
                }
                temp_node.node_id = mesh.m_instance_id;

                SkeletonBindingBufferHandle invalid_skeleton_binding_handle = PILOT_INVALID_HANDLE;
                bool has_skeleton_binding_handle = (!(invalid_skeleton_binding_handle == mesh.skeleton_binding_handle));

                VulkanMesh& mesh_asset           = this->syncMesh(mesh, has_skeleton_binding_handle, pilot_renderer);
                temp_node.ref_mesh               = &mesh_asset;
                temp_node.enable_vertex_blending = mesh.m_enable_vertex_blending_1;

                const auto&        material       = scene.getMaterials()[mesh.m_material];
                VulkanPBRMaterial& material_asset = this->syncMaterial(material, pilot_renderer);
                temp_node.ref_material            = &material_asset;

                m_point_lights_visible_mesh_nodes.push_back(temp_node);

                // upload mesh and material data finished, can be cleared
                MeshHandle mesh_handle;
                mesh_handle.m_vertex_handle = mesh.m_vertexBuffer;
                mesh_handle.m_index_handle  = mesh.m_indexBuffer;
                release_handles.mesh_handles.push_back(mesh_handle);

                if (has_skeleton_binding_handle)
                    release_handles.skeleton_binding_handles.push_back(mesh.skeleton_binding_handle);

                PMaterialHandle material_handle;
                material_handle.m_image_handle0 = material.m_baseColorTexture;
                material_handle.m_image_handle1 = material.m_metallicRoughnessTexture;
                material_handle.m_image_handle2 = material.m_normalTexture;
                material_handle.m_image_handle3 = material.m_occlusionTexture;
                material_handle.m_image_handle4 = material.m_emissiveTexture;
                release_handles.material_handles.push_back(material_handle);
            }
        }
    }

    // Main Camera Mesh
    {
        m_main_camera_visible_mesh_nodes.clear();

        PCamera*  camera           = scene.m_camera.get();
        Matrix4x4 view_matrix      = camera->getViewMatrix();
        Matrix4x4 proj_matrix      = camera->getPersProjMatrix();
        Matrix4x4 proj_view_matrix = (proj_matrix * view_matrix);

        cluster_frustum_t f =
            cluster_frustum_create_from_mat(GLMUtil::fromMat4x4(proj_view_matrix), -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

        for (RenderMesh const& mesh : scene.getMeshes())
        {
            bounding_box_t mesh_asset_bounding_box {mesh.m_bounding_box.getMinCorner(),
                                                    mesh.m_bounding_box.getMaxCorner()};

            // Frustum Culling
            if (tiled_frustum_intersect_box(
                    f, bounding_box_transform(mesh_asset_bounding_box, GLMUtil::fromMat4x4(mesh.m_model_matrix))))
            {
                PVulkanMeshNode temp_node;

                temp_node.model_matrix = GLMUtil::fromMat4x4(mesh.m_model_matrix);

                assert(mesh.m_joint_matrices.size() <= m_mesh_vertex_blending_max_joint_count);
                for (size_t joint_index = 0; joint_index < mesh.m_joint_matrices.size(); ++joint_index)
                {
                    temp_node.joint_matrices[joint_index] = GLMUtil::fromMat4x4(mesh.m_joint_matrices[joint_index]);
                }
                temp_node.node_id = mesh.m_instance_id;

                SkeletonBindingBufferHandle invalid_skeleton_binding_handle = PILOT_INVALID_HANDLE;
                bool has_skeleton_binding_handle = (!(invalid_skeleton_binding_handle == mesh.skeleton_binding_handle));

                VulkanMesh& mesh_asset           = this->syncMesh(mesh, has_skeleton_binding_handle, pilot_renderer);
                temp_node.ref_mesh               = &mesh_asset;
                temp_node.enable_vertex_blending = mesh.m_enable_vertex_blending_1;

                const auto&        material       = scene.getMaterials()[mesh.m_material];
                VulkanPBRMaterial& material_asset = this->syncMaterial(material, pilot_renderer);
                temp_node.ref_material            = &material_asset;

                m_main_camera_visible_mesh_nodes.push_back(temp_node);

                // upload mesh and material data finished, can be cleared
                MeshHandle mesh_handle;
                mesh_handle.m_vertex_handle = mesh.m_vertexBuffer;
                mesh_handle.m_index_handle  = mesh.m_indexBuffer;
                release_handles.mesh_handles.push_back(mesh_handle);

                if (has_skeleton_binding_handle)
                    release_handles.skeleton_binding_handles.push_back(mesh.skeleton_binding_handle);

                PMaterialHandle material_handle;
                material_handle.m_image_handle0 = material.m_baseColorTexture;
                material_handle.m_image_handle1 = material.m_metallicRoughnessTexture;
                material_handle.m_image_handle2 = material.m_normalTexture;
                material_handle.m_image_handle3 = material.m_occlusionTexture;
                material_handle.m_image_handle4 = material.m_emissiveTexture;
                release_handles.material_handles.push_back(material_handle);
            }
        }
    }

    // Axis
    {
        const auto& axis_meshes = scene.getAxisMeshs();
        if (axis_meshes.empty())
        {
            m_is_show_axis = false;
        }
        else
        {
            m_is_show_axis   = true;
            const auto& axis = axis_meshes[0]; // only one axis from scene.getAxisMeshs()

            m_axis_node.model_matrix = GLMUtil::fromMat4x4(axis.m_model_matrix);
            m_axis_node.node_id      = axis.m_instance_id;

            SkeletonBindingBufferHandle invalid_skeleton_binding_handle = PILOT_INVALID_HANDLE;
            bool has_skeleton_binding_handle = (!(invalid_skeleton_binding_handle == axis.skeleton_binding_handle));

            VulkanMesh& axis_mesh_asset        = this->syncMesh(axis, has_skeleton_binding_handle, pilot_renderer);
            m_axis_node.ref_mesh               = &axis_mesh_asset;
            m_axis_node.enable_vertex_blending = axis.m_enable_vertex_blending_1;

            // upload mesh and material data finished, can be cleared
            MeshHandle mesh_handle;
            mesh_handle.m_vertex_handle = axis.m_vertexBuffer;
            mesh_handle.m_index_handle  = axis.m_indexBuffer;
            release_handles.mesh_handles.push_back(mesh_handle);

            if (has_skeleton_binding_handle)
            {
                release_handles.skeleton_binding_handles.push_back(axis.skeleton_binding_handle);
            }
        }
    }

    // Particle
    {
        m_main_camera_visible_particlebillboard_nodes.clear();
        m_main_camera_visible_particlebillboard_nodes.resize(scene.m_particlebillboards.size());

        for (uint32_t now_particlenum = 0; now_particlenum < scene.m_particlebillboards.size(); ++now_particlenum)
        {
            PParticleBillbord& particle_billboard = (*scene.m_particlebillboards[now_particlenum]);

            std::lock_guard lock(particle_billboard.m_mutex);

            m_main_camera_visible_particlebillboard_nodes[now_particlenum].positions = particle_billboard.m_positions;
        }
    }
    
    scene.unlock();
}
