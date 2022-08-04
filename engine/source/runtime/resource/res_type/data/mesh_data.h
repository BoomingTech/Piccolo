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
        float px;
        float py;
        float pz;
        float nx;
        float ny;
        float nz;
        float tx;
        float ty;
        float tz;
        float u;
        float v;
    };

    REFLECTION_TYPE(SkeletonBinding)
    CLASS(SkeletonBinding, Fields)
    {
        REFLECTION_BODY(SkeletonBinding);

    public:
        int   index0;
        int   index1;
        int   index2;
        int   index3;
        float weight0;
        float weight1;
        float weight2;
        float weight3;
    };
    REFLECTION_TYPE(MeshData)
    CLASS(MeshData, Fields)
    {
        REFLECTION_BODY(MeshData);

    public:
        std::vector<Vertex>          vertex_buffer;
        std::vector<int>             index_buffer;
        std::vector<SkeletonBinding> bind;
    };

} // namespace Piccolo