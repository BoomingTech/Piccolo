#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>
namespace Piccolo
{

    REFLECTION_TYPE(Vertex)
    CLASS(Vertex, Fields)
    {
        REFLECTION_BODY(Vertex);

    public:
        float m_px;
        float m_py;
        float m_pz;
        float m_nx;
        float m_ny;
        float m_nz;
        float m_tx;
        float m_ty;
        float m_tz;
        float m_u;
        float m_v;
    };

    REFLECTION_TYPE(SkeletonBinding)
    CLASS(SkeletonBinding, Fields)
    {
        REFLECTION_BODY(SkeletonBinding);

    public:
        int   m_index0;
        int   m_index1;
        int   m_index2;
        int   m_index3;
        float m_weight0;
        float m_weight1;
        float m_weight2;
        float m_weight3;
    };

    REFLECTION_TYPE(MeshData)
    CLASS(MeshData, Fields)
    {
        REFLECTION_BODY(MeshData);

    public:
        std::vector<Vertex>          m_vertex_buffer;
        std::vector<int>             m_index_buffer;
        std::vector<SkeletonBinding> m_bind;
    };

} // namespace Piccolo