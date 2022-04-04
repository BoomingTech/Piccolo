#pragma once

#include "runtime/core/math/vector4.h"

#include "render/resource_handle.h"

#include <mutex>
#include <vector>

namespace Pilot
{
    class RenderMesh;

    struct PMaterialHandle
    {
        TextureHandle m_image_handle0;
        TextureHandle m_image_handle1;
        TextureHandle m_image_handle2;
        TextureHandle m_image_handle3;
        TextureHandle m_image_handle4;
        bool          operator==(const PMaterialHandle& rhs) const
        {
            return m_image_handle0 == rhs.m_image_handle0 && m_image_handle1 == rhs.m_image_handle1 &&
                   m_image_handle2 == rhs.m_image_handle2 && m_image_handle3 == rhs.m_image_handle3 &&
                   m_image_handle4 == rhs.m_image_handle4;
        }
        size_t getHashValue() const
        {
            return (((m_image_handle0.getHashValue() ^ (m_image_handle1.getHashValue() << 1)) ^
                     (m_image_handle2.getHashValue() << 1)) ^
                    (m_image_handle3.getHashValue() << 1)) ^
                   (m_image_handle4.getHashValue() << 1);
        }
    };

    struct PParticleBillbord
    {
        std::vector<Vector4> m_positions;
        std::mutex           m_mutex;
    };

    struct Material
    {
        bool m_blend       = false;
        bool m_doubleSided = false;

        TextureHandle m_baseColorTexture = PILOT_INVALID_HANDLE;
        Vector4       m_baseColorFactor  = {1.0f, 1.0f, 1.0f, 1.0f};

        TextureHandle m_metallicRoughnessTexture = PILOT_INVALID_HANDLE; // blue = metallic, green = roughness
        float         m_metallicFactor           = 1.0f;
        float         m_roughnessFactor          = 1.0f;

        TextureHandle m_normalTexture = PILOT_INVALID_HANDLE;
        float         m_normalScale   = 1.0f;

        TextureHandle m_occlusionTexture  = PILOT_INVALID_HANDLE;
        float         m_occlusionStrength = 1.0f;

        TextureHandle m_emissiveTexture = PILOT_INVALID_HANDLE;
        Vector3       m_emissiveFactor  = {0.0f, 0.0f, 0.0f};

        size_t m_guid = PILOT_INVALID_GUID;

        bool operator==(const Material& rhs) const
        {
            return m_baseColorTexture == rhs.m_baseColorTexture &&
                   m_metallicRoughnessTexture == rhs.m_metallicRoughnessTexture &&
                   m_normalTexture == rhs.m_normalTexture && m_occlusionTexture == rhs.m_occlusionTexture &&
                   m_emissiveTexture == rhs.m_emissiveTexture;
        }
        size_t getHashValue() const
        {
            return (((m_baseColorTexture.getHashValue() ^ (m_metallicRoughnessTexture.getHashValue() << 1)) ^
                     (m_normalTexture.getHashValue() << 1)) ^
                    (m_occlusionTexture.getHashValue() << 1)) ^
                   (m_emissiveTexture.getHashValue() << 1);
        }
    };

    struct PMeshMaterialPack
    {
        RenderMesh* p_mesh;
        Material*   p_material;
        PMeshMaterialPack() : p_mesh(nullptr), p_material(nullptr) {}
        PMeshMaterialPack(RenderMesh* mesh, Material* material) : p_mesh(mesh), p_material(material) {}
        bool isValid() const { return p_mesh && p_material; }
        bool operator==(const PMeshMaterialPack& rhs) const
        {
            return p_mesh == rhs.p_mesh && p_material == rhs.p_material;
        }
        virtual size_t getHashValue() const { return (size_t)p_mesh ^ ((size_t)p_material << 1); }
    };

} // namespace Pilot