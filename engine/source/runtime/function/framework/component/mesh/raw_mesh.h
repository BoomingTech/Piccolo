#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Piccolo
{
    enum class PrimitiveType
    {
        point,
        line,
        triangle,
        quad
    };

    struct RawVertexBuffer
    {
        uint32_t           vertex_count {0};
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<float> tangents;
        std::vector<float> uvs;
    };

    struct RawIndexBuffer
    {
        PrimitiveType         primitive_type {PrimitiveType::triangle};
        uint32_t              primitive_count {0};
        std::vector<uint32_t> indices;
    };

    struct MaterialTexture
    {
        std::string base_color;
        std::string metallic_roughness;
        std::string normal;
    };

    struct StaticMeshData
    {
        RawVertexBuffer vertex_buffer;
        RawIndexBuffer  index_buffer;
        MaterialTexture material_texture;
    };
} // namespace Piccolo
