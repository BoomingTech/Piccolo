#include "runtime/function/render/include/render/render.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

Pilot::VulkanMesh&
Pilot::PVulkanManager::syncMesh(RenderMesh const& mesh, bool enable_vertex_blending, PilotRenderer* pilot_renderer)
{
    size_t assetid = mesh.m_guid;

    auto iter = m_vulkan_meshes.find(assetid);
    if (iter != m_vulkan_meshes.end())
    {
        return iter->second;
    }
    else
    {
        VulkanMesh temp;
        auto       res = m_vulkan_meshes.insert(std::make_pair(assetid, std::move(temp)));
        assert(res.second);

        const SceneMemory* index_buffer_memory = pilot_renderer->f_get_memory(mesh.m_indexBuffer);
        uint32_t           index_buffer_size   = static_cast<uint32_t>(index_buffer_memory->m_size);
        void*              index_buffer_data   = index_buffer_memory->m_data;

        const SceneMemory*               vertex_buffer_memory = pilot_renderer->f_get_memory(mesh.m_vertexBuffer);
        uint32_t                         vertex_buffer_size   = static_cast<uint32_t>(vertex_buffer_memory->m_size);
        Mesh_PosNormalTangentTex0Vertex* vertex_buffer_data =
            reinterpret_cast<Mesh_PosNormalTangentTex0Vertex*>(vertex_buffer_memory->m_data);

        VulkanMesh& now_mesh = res.first->second;

        if (enable_vertex_blending)
        {
            const SceneMemory* joint_binding_buffer_memory = pilot_renderer->f_get_memory(mesh.skeleton_binding_handle);
            uint32_t           joint_binding_buffer_size   = static_cast<uint32_t>(joint_binding_buffer_memory->m_size);
            Mesh_VertexBinding* joint_binding_buffer_data =
                reinterpret_cast<Mesh_VertexBinding*>(joint_binding_buffer_memory->m_data);
            updateMeshData(enable_vertex_blending,
                           index_buffer_size,
                           index_buffer_data,
                           vertex_buffer_size,
                           vertex_buffer_data,
                           joint_binding_buffer_size,
                           joint_binding_buffer_data,
                           now_mesh);
        }
        else
        {
            updateMeshData(enable_vertex_blending,
                           index_buffer_size,
                           index_buffer_data,
                           vertex_buffer_size,
                           vertex_buffer_data,
                           0,
                           NULL,
                           now_mesh);
        }

        return now_mesh;
    }
}