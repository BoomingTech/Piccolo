#pragma once

#include "runtime/core/math/axis_aligned.h"
#include "runtime/core/math/matrix4.h"

#include <cstdint>
#include <vector>

namespace Piccolo
{
    class RenderEntity
    {
    public:
        uint32_t  m_instance_id {0};
        Matrix4x4 m_model_matrix {Matrix4x4::IDENTITY};

        // mesh
        size_t                 m_mesh_asset_id {0};
        bool                   m_enable_vertex_blending {false};
        std::vector<Matrix4x4> m_joint_matrices;
        AxisAlignedBox         m_bounding_box;

        // material
        size_t  m_material_asset_id {0};
        bool    m_blend {false};
        bool    m_double_sided {false};
        Vector4 m_base_color_factor {1.0f, 1.0f, 1.0f, 1.0f};
        float   m_metallic_factor {1.0f};
        float   m_roughness_factor {1.0f};
        float   m_normal_scale {1.0f};
        float   m_occlusion_strength {1.0f};
        Vector3 m_emissive_factor {0.0f, 0.0f, 0.0f};
    };
} // namespace Piccolo
