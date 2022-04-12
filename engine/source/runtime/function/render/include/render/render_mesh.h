#pragma once

#include "runtime/core/math/axis_aligned.h"
#include "runtime/core/math/matrix4.h"

#include "render/resource_handle.h"

#include <vector>

namespace Pilot
{
    struct MeshHandle
    {
        VertexBufferHandle m_vertex_handle;
        IndexBufferHandle  m_index_handle;
        AxisAlignedBox     m_bounding_box;
        bool               operator==(const MeshHandle& rhs) const
        {
            return m_vertex_handle == rhs.m_vertex_handle && m_index_handle == rhs.m_index_handle;
        }
        size_t getHashValue() const { return m_vertex_handle.getHashValue() ^ (m_index_handle.getHashValue() << 1); }
    };

    struct SkeletonMeshBinding
    {
        SkeletonBindingBufferHandle m_skeleton_binding_handle;
        AxisAlignedBox              m_bounding_box;
        bool                        operator==(const SkeletonMeshBinding& rhs) const
        {
            return m_skeleton_binding_handle == rhs.m_skeleton_binding_handle;
        }
        size_t getHashValue() const { return m_skeleton_binding_handle.getHashValue(); }
    };

    struct Mesh_PosNormalTangentTex0Vertex
    {
        float x, y, z;    // position
        float nx, ny, nz; // normal
        float tx, ty, tz; // tangent
        float u, v;       // UV coordinates
    };

    struct Mesh_VertexBinding
    {
        int   index0, index1, index2, index3;
        float weight0, weight1, weight2, weight3;
    };

    class RenderMesh
    {
    public:
        // --- per instance members ---
        size_t                 m_instance_id  = 0;
        Matrix4x4              m_model_matrix = Matrix4x4::IDENTITY;
        std::vector<Matrix4x4> m_joint_matrices;
        bool                   m_enable_vertex_blending_1 = false;

        // --- per asset members ---
        AxisAlignedBox m_bounding_box;
        size_t         m_guid = PILOT_INVALID_GUID;
        // index into materials vector
        int m_material = 0;
        // vertices and indices
        VertexBufferHandle m_vertexBuffer = PILOT_INVALID_HANDLE;
        IndexBufferHandle  m_indexBuffer  = PILOT_INVALID_HANDLE;
        // vertex blending
        bool                        m_enable_vertex_blending = false;
        SkeletonBindingBufferHandle skeleton_binding_handle  = PILOT_INVALID_HANDLE;

        RenderMesh() = default;
        bool operator==(const RenderMesh& rhs) const
        {
            return m_vertexBuffer == rhs.m_vertexBuffer && m_indexBuffer == rhs.m_indexBuffer;
        }
        size_t getHashValue() const { return m_vertexBuffer.getHashValue() ^ (m_indexBuffer.getHashValue() << 1); }
    };
} // namespace Pilot
