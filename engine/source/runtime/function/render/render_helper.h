#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

namespace Piccolo
{
    class RenderScene;
    class RenderCamera;

    static inline uint32_t roundUp(uint32_t value, uint32_t alignment)
    {
        uint32_t temp = value + alignment - static_cast<uint32_t>(1);
        return (temp - temp % alignment);
    }

    // TODO: support cluster lighting
    struct ClusterFrustum
    {
        // we don't consider the near and far plane currently
        Vector4 m_plane_right;
        Vector4 m_plane_left;
        Vector4 m_plane_top;
        Vector4 m_plane_bottom;
        Vector4 m_plane_near;
        Vector4 m_plane_far;
    };

    struct BoundingBox
    {
        Vector3 min_bound {std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
        Vector3 max_bound {std::numeric_limits<float>::min(),
                           std::numeric_limits<float>::min(),
                           std::numeric_limits<float>::min()};

        BoundingBox() {}

        BoundingBox(const Vector3& minv, const Vector3& maxv)
        {
            min_bound = minv;
            max_bound = maxv;
        }

        void merge(const BoundingBox& rhs)
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

    struct BoundingSphere
    {
        Vector3   m_center;
        float     m_radius;
    };

    struct FrustumPoints
    {
        Vector3 m_frustum_points;
    };

    ClusterFrustum CreateClusterFrustumFromMatrix(Matrix4x4 mat,
                                                  float     x_left,
                                                  float     x_right,
                                                  float     y_top,
                                                  float     y_bottom,
                                                  float     z_near,
                                                  float     z_far);

    bool TiledFrustumIntersectBox(ClusterFrustum const& f, BoundingBox const& b);

    BoundingBox BoundingBoxTransform(BoundingBox const& b, Matrix4x4 const& m);

    bool BoxIntersectsWithSphere(BoundingBox const& b, BoundingSphere const& s);

    Matrix4x4 CalculateDirectionalLightCamera(RenderScene& scene, RenderCamera& camera);
} // namespace Piccolo
