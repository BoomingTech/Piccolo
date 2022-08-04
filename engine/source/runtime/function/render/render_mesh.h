#pragma once

#include "runtime/core/math/vector3.h"

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Piccolo
{
    struct MeshVertex
    {
        struct VulkanMeshVertexPostition
        {
            glm::vec3 position;
        };

        struct VulkanMeshVertexVaryingEnableBlending
        {
            glm::vec3 normal;
            glm::vec3 tangent;
        };

        struct VulkanMeshVertexVarying
        {
            glm::vec2 texcoord;
        };

        struct VulkanMeshVertexJointBinding
        {
            glm::ivec4 indices;
            glm::vec4  weights;
        };

        static std::array<VkVertexInputBindingDescription, 3> getBindingDescriptions()
        {
            std::array<VkVertexInputBindingDescription, 3> binding_descriptions {};

            // position
            binding_descriptions[0].binding   = 0;
            binding_descriptions[0].stride    = sizeof(VulkanMeshVertexPostition);
            binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // varying blending
            binding_descriptions[1].binding   = 1;
            binding_descriptions[1].stride    = sizeof(VulkanMeshVertexVaryingEnableBlending);
            binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // varying
            binding_descriptions[2].binding   = 2;
            binding_descriptions[2].stride    = sizeof(VulkanMeshVertexVarying);
            binding_descriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_descriptions;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions {};

            // position
            attribute_descriptions[0].binding  = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset   = offsetof(VulkanMeshVertexPostition, position);

            // varying blending
            attribute_descriptions[1].binding  = 1;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset   = offsetof(VulkanMeshVertexVaryingEnableBlending, normal);
            attribute_descriptions[2].binding  = 1;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[2].offset   = offsetof(VulkanMeshVertexVaryingEnableBlending, tangent);

            // varying
            attribute_descriptions[3].binding  = 2;
            attribute_descriptions[3].location = 3;
            attribute_descriptions[3].format   = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[3].offset   = offsetof(VulkanMeshVertexVarying, texcoord);

            return attribute_descriptions;
        }
    };
} // namespace Piccolo
