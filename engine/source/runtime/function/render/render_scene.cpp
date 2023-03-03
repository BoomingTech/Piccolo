#include "runtime/function/render/render_scene.h"
#include "runtime/function/render/render_helper.h"
#include "runtime/function/render/render_pass.h"
#include "runtime/function/render/render_resource.h"

namespace Piccolo
{
    void RenderScene::clear()
    {
    }

    void RenderScene::updateVisibleObjects(std::shared_ptr<RenderResource> render_resource,
                                           std::shared_ptr<RenderCamera>   camera)
    {
        updateVisibleObjectsDirectionalLight(render_resource, camera);
        updateVisibleObjectsPointLight(render_resource);
        updateVisibleObjectsMainCamera(render_resource, camera);
        updateVisibleObjectsAxis(render_resource);
        updateVisibleObjectsParticle(render_resource);
    }

    void RenderScene::setVisibleNodesReference()
    {
        RenderPass::m_visiable_nodes.p_directional_light_visible_mesh_nodes = &m_directional_light_visible_mesh_nodes;
        RenderPass::m_visiable_nodes.p_point_lights_visible_mesh_nodes      = &m_point_lights_visible_mesh_nodes;
        RenderPass::m_visiable_nodes.p_main_camera_visible_mesh_nodes       = &m_main_camera_visible_mesh_nodes;
        RenderPass::m_visiable_nodes.p_axis_node                            = &m_axis_node;
    }

    GuidAllocator<GameObjectPartId>& RenderScene::getInstanceIdAllocator() { return m_instance_id_allocator; }

    GuidAllocator<MeshSourceDesc>& RenderScene::getMeshAssetIdAllocator() { return m_mesh_asset_id_allocator; }

    GuidAllocator<MaterialSourceDesc>& RenderScene::getMaterialAssetdAllocator()
    {
        return m_material_asset_id_allocator;
    }

    void RenderScene::addInstanceIdToMap(uint32_t instance_id, GObjectID go_id)
    {
        m_mesh_object_id_map[instance_id] = go_id;
    }

    GObjectID RenderScene::getGObjectIDByMeshID(uint32_t mesh_id) const
    {
        auto find_it = m_mesh_object_id_map.find(mesh_id);
        if (find_it != m_mesh_object_id_map.end())
        {
            return find_it->second;
        }
        return GObjectID();
    }

    void RenderScene::deleteEntityByGObjectID(GObjectID go_id)
    {
        for (auto it = m_mesh_object_id_map.begin(); it != m_mesh_object_id_map.end(); it++)
        {
            if (it->second == go_id)
            {
                m_mesh_object_id_map.erase(it);
                break;
            }
        }

        GameObjectPartId part_id = {go_id, 0};
        size_t           find_guid;
        if (m_instance_id_allocator.getElementGuid(part_id, find_guid))
        {
            for (auto it = m_render_entities.begin(); it != m_render_entities.end(); it++)
            {
                if (it->m_instance_id == find_guid)
                {
                    m_render_entities.erase(it);
                    break;
                }
            }
        }
    }

    void RenderScene::clearForLevelReloading()
    {
        m_instance_id_allocator.clear();
        m_mesh_object_id_map.clear();
        m_render_entities.clear();
    }

    void RenderScene::updateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> render_resource,
                                                           std::shared_ptr<RenderCamera>   camera)
    {
        Matrix4x4 directional_light_proj_view = CalculateDirectionalLightCamera(*this, *camera);

        render_resource->m_mesh_perframe_storage_buffer_object.directional_light_proj_view =
            directional_light_proj_view;
        render_resource->m_mesh_directional_light_shadow_perframe_storage_buffer_object.light_proj_view =
            directional_light_proj_view;

        m_directional_light_visible_mesh_nodes.clear();

        ClusterFrustum frustum =
            CreateClusterFrustumFromMatrix(directional_light_proj_view, -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

        for (const RenderEntity& entity : m_render_entities)
        {
            BoundingBox mesh_asset_bounding_box {entity.m_bounding_box.getMinCorner(),
                                                 entity.m_bounding_box.getMaxCorner()};

            if (TiledFrustumIntersectBox(frustum, BoundingBoxTransform(mesh_asset_bounding_box, entity.m_model_matrix)))
            {
                m_directional_light_visible_mesh_nodes.emplace_back();
                RenderMeshNode& temp_node = m_directional_light_visible_mesh_nodes.back();

                temp_node.model_matrix = &entity.m_model_matrix;

                assert(entity.m_joint_matrices.size() <= s_mesh_vertex_blending_max_joint_count);
                if (!entity.m_joint_matrices.empty())
                {
                    temp_node.joint_count    = static_cast<uint32_t>(entity.m_joint_matrices.size());
                    temp_node.joint_matrices = entity.m_joint_matrices.data();
                }
                temp_node.node_id = entity.m_instance_id;

                VulkanMesh& mesh_asset           = render_resource->getEntityMesh(entity);
                temp_node.ref_mesh               = &mesh_asset;
                temp_node.enable_vertex_blending = entity.m_enable_vertex_blending;

                VulkanPBRMaterial& material_asset = render_resource->getEntityMaterial(entity);
                temp_node.ref_material            = &material_asset;
            }
        }
    }

    void RenderScene::updateVisibleObjectsPointLight(std::shared_ptr<RenderResource> render_resource)
    {
        m_point_lights_visible_mesh_nodes.clear();

        std::vector<BoundingSphere> point_lights_bounding_spheres;
        uint32_t                    point_light_num = static_cast<uint32_t>(m_point_light_list.m_lights.size());
        point_lights_bounding_spheres.resize(point_light_num);
        for (size_t i = 0; i < point_light_num; i++)
        {
            point_lights_bounding_spheres[i].m_center = m_point_light_list.m_lights[i].m_position;
            point_lights_bounding_spheres[i].m_radius = m_point_light_list.m_lights[i].calculateRadius();
        }

        for (const RenderEntity& entity : m_render_entities)
        {
            BoundingBox mesh_asset_bounding_box {entity.m_bounding_box.getMinCorner(),
                                                 entity.m_bounding_box.getMaxCorner()};

            bool intersect_with_point_lights = true;
            for (size_t i = 0; i < point_light_num; i++)
            {
                if (!BoxIntersectsWithSphere(BoundingBoxTransform(mesh_asset_bounding_box, entity.m_model_matrix),
                                             point_lights_bounding_spheres[i]))
                {
                    intersect_with_point_lights = false;
                    break;
                }
            }

            if (intersect_with_point_lights)
            {
                m_point_lights_visible_mesh_nodes.emplace_back();
                RenderMeshNode& temp_node = m_point_lights_visible_mesh_nodes.back();

                temp_node.model_matrix = &entity.m_model_matrix;

                assert(entity.m_joint_matrices.size() <= s_mesh_vertex_blending_max_joint_count);
                if (!entity.m_joint_matrices.empty())
                {
                    temp_node.joint_count    = static_cast<uint32_t>(entity.m_joint_matrices.size());
                    temp_node.joint_matrices = entity.m_joint_matrices.data();
                }
                temp_node.node_id = entity.m_instance_id;

                VulkanMesh& mesh_asset           = render_resource->getEntityMesh(entity);
                temp_node.ref_mesh               = &mesh_asset;
                temp_node.enable_vertex_blending = entity.m_enable_vertex_blending;

                VulkanPBRMaterial& material_asset = render_resource->getEntityMaterial(entity);
                temp_node.ref_material            = &material_asset;
            }
        }
    }

    void RenderScene::updateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> render_resource,
                                                     std::shared_ptr<RenderCamera>   camera)
    {
        m_main_camera_visible_mesh_nodes.clear();

        Matrix4x4 view_matrix      = camera->getViewMatrix();
        Matrix4x4 proj_matrix      = camera->getPersProjMatrix();
        Matrix4x4 proj_view_matrix = proj_matrix * view_matrix;

        ClusterFrustum f = CreateClusterFrustumFromMatrix(proj_view_matrix, -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

        for (const RenderEntity& entity : m_render_entities)
        {
            BoundingBox mesh_asset_bounding_box {entity.m_bounding_box.getMinCorner(),
                                                 entity.m_bounding_box.getMaxCorner()};

            if (TiledFrustumIntersectBox(f, BoundingBoxTransform(mesh_asset_bounding_box, entity.m_model_matrix)))
            {
                m_main_camera_visible_mesh_nodes.emplace_back();
                RenderMeshNode& temp_node = m_main_camera_visible_mesh_nodes.back();
                temp_node.model_matrix    = &entity.m_model_matrix;

                assert(entity.m_joint_matrices.size() <= s_mesh_vertex_blending_max_joint_count);
                if (!entity.m_joint_matrices.empty())
                {
                    temp_node.joint_count    = static_cast<uint32_t>(entity.m_joint_matrices.size());
                    temp_node.joint_matrices = entity.m_joint_matrices.data();
                }
                temp_node.node_id = entity.m_instance_id;

                VulkanMesh& mesh_asset           = render_resource->getEntityMesh(entity);
                temp_node.ref_mesh               = &mesh_asset;
                temp_node.enable_vertex_blending = entity.m_enable_vertex_blending;

                VulkanPBRMaterial& material_asset = render_resource->getEntityMaterial(entity);
                temp_node.ref_material            = &material_asset;
            }
        }
    }

    void RenderScene::updateVisibleObjectsAxis(std::shared_ptr<RenderResource> render_resource)
    {
        if (m_render_axis.has_value())
        {
            RenderEntity& axis = *m_render_axis;

            m_axis_node.model_matrix = axis.m_model_matrix;
            m_axis_node.node_id      = axis.m_instance_id;

            VulkanMesh& mesh_asset             = render_resource->getEntityMesh(axis);
            m_axis_node.ref_mesh               = &mesh_asset;
            m_axis_node.enable_vertex_blending = axis.m_enable_vertex_blending;
        }
    }

    void RenderScene::updateVisibleObjectsParticle(std::shared_ptr<RenderResource> render_resource)
    {
        // TODO
    }
} // namespace Piccolo
