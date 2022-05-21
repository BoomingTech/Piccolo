#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace Pilot
{
    enum class PILOT_PIXEL_FORMAT : uint8_t
    {
        PILOT_PIXEL_FORMAT_UNKNOWN = 0,
        PILOT_PIXEL_FORMAT_R8G8B8_UNORM,
        PILOT_PIXEL_FORMAT_R8G8B8_SRGB,
        PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM,
        PILOT_PIXEL_FORMAT_R8G8B8A8_SRGB,
        PILOT_PIXEL_FORMAT_R32G32_FLOAT,
        PILOT_PIXEL_FORMAT_R32G32B32_FLOAT,
        PILOT_PIXEL_FORMAT_R32G32B32A32_FLOAT
    };

    enum class PILOT_IMAGE_TYPE : uint8_t
    {
        PILOT_IMAGE_TYPE_UNKNOWM = 0,
        PILOT_IMAGE_TYPE_2D
    };

    enum class RENDER_PIPELINE_TYPE : uint8_t
    {
        FORWARD_PIPELINE = 0,
        DEFERRED_PIPELINE,
        PIPELINE_TYPE_COUNT
    };

    class BufferData
    {
    public:
        size_t m_size {0};
        void*  m_data {nullptr};

        BufferData() = delete;
        BufferData(size_t size)
        {
            m_size = size;
            m_data = malloc(size);
        }
        ~BufferData()
        {
            if (m_data)
            {
                free(m_data);
            }
        }
        bool isValid() const { return m_data != nullptr; }
    };

    class TextureData
    {
    public:
        uint32_t m_width {0};
        uint32_t m_height {0};
        uint32_t m_depth {0};
        uint32_t m_mip_levels {0};
        uint32_t m_array_layers {0};
        void*    m_pixels {nullptr};

        PILOT_PIXEL_FORMAT m_format {PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_UNKNOWN};
        PILOT_IMAGE_TYPE   m_type {PILOT_IMAGE_TYPE::PILOT_IMAGE_TYPE_UNKNOWM};

        TextureData() = default;
        ~TextureData()
        {
            if (m_pixels)
            {
                free(m_pixels);
            }
        }
        bool isValid() const { return m_pixels != nullptr; }
    };

    struct MeshVertexDataDefinition
    {
        float x, y, z;    // position
        float nx, ny, nz; // normal
        float tx, ty, tz; // tangent
        float u, v;       // UV coordinates
    };

    struct MeshVertexBindingDataDefinition
    {
        int   index0, index1, index2, index3;
        float weight0, weight1, weight2, weight3;
    };

    struct MeshSourceDesc
    {
        std::string mesh_file;

        bool   operator==(const MeshSourceDesc& rhs) const { return mesh_file == rhs.mesh_file; }
        size_t getHashValue() const { return std::hash<std::string> {}(mesh_file); }
    };

    struct MaterialSourceDesc
    {
        std::string base_color_file;
        std::string metallic_roughness_file;
        std::string normal_file;
        std::string occlusion_file;
        std::string emissive_file;

        bool operator==(const MaterialSourceDesc& rhs) const
        {
            return base_color_file == rhs.base_color_file && metallic_roughness_file == rhs.metallic_roughness_file &&
                   normal_file == rhs.normal_file && occlusion_file == rhs.occlusion_file &&
                   emissive_file == rhs.emissive_file;
        }
        size_t getHashValue() const
        {
            size_t h0 = std::hash<std::string> {}(base_color_file);
            size_t h1 = std::hash<std::string> {}(metallic_roughness_file);
            size_t h2 = std::hash<std::string> {}(normal_file);
            size_t h3 = std::hash<std::string> {}(occlusion_file);
            size_t h4 = std::hash<std::string> {}(emissive_file);
            return (((h0 ^ (h1 << 1)) ^ (h2 << 1)) ^ (h3 << 1)) ^ (h4 << 1);
        }
    };

    struct StaticMeshData
    {
        std::shared_ptr<BufferData> m_vertex_buffer;
        std::shared_ptr<BufferData> m_index_buffer;
    };

    struct RenderMeshData
    {
        StaticMeshData              m_static_mesh_data;
        std::shared_ptr<BufferData> m_skeleton_binding_buffer;
    };

    struct RenderMaterialData
    {
        std::shared_ptr<TextureData> m_base_color_texture;
        std::shared_ptr<TextureData> m_metallic_roughness_texture;
        std::shared_ptr<TextureData> m_normal_texture;
        std::shared_ptr<TextureData> m_occlusion_texture;
        std::shared_ptr<TextureData> m_emissive_texture;
    };
} // namespace Pilot

template<>
struct std::hash<Pilot::MeshSourceDesc>
{
    size_t operator()(const Pilot::MeshSourceDesc& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::MaterialSourceDesc>
{
    size_t operator()(const Pilot::MaterialSourceDesc& rhs) const noexcept { return rhs.getHashValue(); }
};
