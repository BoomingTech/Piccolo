#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "runtime/function/render/include/render/framebuffer.h"

namespace Pilot
{
    static inline uint32_t roundUp(uint32_t value, uint32_t alignment)
    {
        uint32_t temp = value + alignment - static_cast<uint32_t>(1);
        return (temp - temp % alignment);
    }

    // TODO: support cluster lighting
    struct cluster_frustum_t
    {
        // we don't consider the near and far plane currently
        glm::vec4 m_plane_right;
        glm::vec4 m_plane_left;
        glm::vec4 m_plane_top;
        glm::vec4 m_plane_bottom;
        glm::vec4 m_plane_near;
        glm::vec4 m_plane_far;
    };

    struct bounding_box_t
    {
        Vector3 min_bound {std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
        Vector3 max_bound {std::numeric_limits<float>::min(),
                           std::numeric_limits<float>::min(),
                           std::numeric_limits<float>::min()};

        bounding_box_t() {}

        bounding_box_t(const Vector3& minv, const Vector3& maxv)
        {
            min_bound = minv;
            max_bound = maxv;
        }

        void merge(const bounding_box_t& rhs)
        {
            min_bound.makeFloor(rhs.min_bound);
            max_bound.makeCeil(rhs.max_bound);
        }

        void merge(const Vector3& point)
        {
            min_bound.makeFloor(point);
            max_bound.makeCeil(point);
        }
    };

    struct bounding_sphere_t
    {
        glm::vec3 m_center;
        float     m_radius;
    };

    struct frustum_points_t
    {
        glm::vec3 m_frustum_points;
    };

    cluster_frustum_t cluster_frustum_create_from_mat(glm::mat4 mat,
                                                      float     x_left,
                                                      float     x_right,
                                                      float     y_top,
                                                      float     y_bottom,
                                                      float     z_near,
                                                      float     z_far);
    bool              tiled_frustum_intersect_box(cluster_frustum_t const& f, bounding_box_t const& b);
    bounding_box_t    bounding_box_transform(bounding_box_t const& b, glm::mat4 const& m);
    bool              box_intersect_sphere(bounding_box_t const& b, bounding_sphere_t const& s);
    glm::mat4         calculate_directional_light_camera(class Scene& scene);
} // namespace Pilot
