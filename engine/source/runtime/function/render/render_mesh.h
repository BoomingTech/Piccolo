#pragma once

#include "runtime/core/math/vector2.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

#include <array>
#include "interface/rhi.h"

namespace Piccolo
{
    struct MeshVertex
    {
        struct VulkanMeshVertexPostition
        {
            Vector3 position;
        };

        struct VulkanMeshVertexVaryingEnableBlending
        {
            Vector3 normal;
            Vector3 tangent;
        };

        struct VulkanMeshVertexVarying
        {
            Vector2 texcoord;
        };

        struct VulkanMeshVertexJointBinding
        {
            int indices[4];
            Vector4  weights;
        };

        static std::array<RHIVertexInputBindingDescription, 3> getBindingDescriptions()
        {
            std::array<RHIVertexInputBindingDescription, 3> binding_descriptions {};

            // position
            binding_descriptions[0].binding   = 0;
            binding_descriptions[0].stride    = sizeof(VulkanMeshVertexPostition);
            binding_descriptions[0].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
            // varying blending
            binding_descriptions[1].binding   = 1;
            binding_descriptions[1].stride    = sizeof(VulkanMeshVertexVaryingEnableBlending);
            binding_descriptions[1].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
            // varying
            binding_descriptions[2].binding   = 2;
            binding_descriptions[2].stride    = sizeof(VulkanMeshVertexVarying);
            binding_descriptions[2].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
            return binding_descriptions;
        }

        static std::array<RHIVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
            std::array<RHIVertexInputAttributeDescription, 4> attribute_descriptions {};

            // position
            attribute_descriptions[0].binding  = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format   = RHI_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset   = offsetof(VulkanMeshVertexPostition, position);

            // varying blending
            attribute_descriptions[1].binding  = 1;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format   = RHI_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset   = offsetof(VulkanMeshVertexVaryingEnableBlending, normal);
            attribute_descriptions[2].binding  = 1;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format   = RHI_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[2].offset   = offsetof(VulkanMeshVertexVaryingEnableBlending, tangent);

            // varying
            attribute_descriptions[3].binding  = 2;
            attribute_descriptions[3].location = 3;
            attribute_descriptions[3].format   = RHI_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[3].offset   = offsetof(VulkanMeshVertexVarying, texcoord);

            return attribute_descriptions;
        }
    };
} // namespace Piccolo
