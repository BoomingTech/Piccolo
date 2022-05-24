#include "runtime/function/render/render_helper.h"
#include "runtime/function/render/glm_wrapper.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_scene.h"

namespace Pilot
{
    ClusterFrustum CreateClusterFrustumFromMatrix(glm::mat4 mat,
                                                  float     x_left,
                                                  float     x_right,
                                                  float     y_top,
                                                  float     y_bottom,
                                                  float     z_near,
                                                  float     z_far)
    {
        ClusterFrustum f;

        // the following is in the vulkan space
        // note that the Y axis is flipped in Vulkan
        assert(y_top < y_bottom);

        assert(x_left < x_right);
        assert(z_near < z_far);

        // calculate the tiled frustum
        // [Fast Extraction of Viewing Frustum Planes from the WorldView - Projection
        // Matrix](http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf)

        // glm uses column vector and column major
        glm::mat4 mat_column = glm::transpose(mat);

        // [vec.xyz 1][mat.col0] / [vec.xyz 1][mat.col3] > x_right
        // [vec.xyz 1][mat.col0 - mat.col3*x_right] > 0
        f.m_plane_right = mat_column[0] - (mat_column[3] * x_right);
        // normalize
        f.m_plane_right *= (1.0 / glm::length(glm::vec3(f.m_plane_right.x, f.m_plane_right.y, f.m_plane_right.z)));

        // for example, we try to calculate the "plane_left" of the tile frustum
        // note that we use the row vector to be consistent with the DirectXMath
        // [vec.xyz 1][mat.col0] / [vec.xyz 1][mat.col3] < x_left
        //
        // evidently, the value "[vec.xyz 1][mat.col3]" is expected to be greater than
        // 0 (w of clip space) and we multiply both sides by "[vec.xyz 1][mat.col3]",
        // the inequality symbol remains the same [vec.xyz 1][mat.col0] < [vec.xyz
        // 1][mat.col3]*x_left
        //
        // since "x_left" is a scalar, the "scalar multiplication" is applied
        // [vec.xyz 1][mat.col0] < [vec.xyz 1][mat.col3*x_left]
        // [vec.xyz 1][mat.col0 - mat.col3*x_left] < 0
        //
        // we follow the "DirectX::BoundingFrustum::Intersects", the normal of the
        // plane is pointing ourward [vec.xyz 1][mat.col3*x_left - mat.col0] > 0
        //
        // the plane can be defined as [x y z 1][A B C D] = 0 and the [A B C D] is
        // exactly [mat.col0 - mat.col3*x_left] and we need to normalize the normal[A
        // B C] of the plane let [A B C D] = [mat.col3*x_left - mat.col0] [A B C D] /=
        // length([A B C].xyz)
        f.m_plane_left = (mat_column[3] * x_left) - mat_column[0];
        // normalize
        f.m_plane_left *= (1.0 / glm::length(glm::vec3(f.m_plane_left.x, f.m_plane_left.y, f.m_plane_left.z)));

        // [vec.xyz 1][mat.col1] / [vec.xyz 1][mat.col3] < y_top
        // [vec.xyz 1][mat.col3*y_top - mat.col1] > 0
        f.m_plane_top = (mat_column[3] * y_top) - mat_column[1];
        // normalize
        f.m_plane_top *= (1.0 / glm::length(glm::vec3(f.m_plane_top.x, f.m_plane_top.y, f.m_plane_top.z)));

        // [vec.xyz 1][mat.col1] / [vec.xyz 1][mat.col3] > y_bottom
        // [vec.xyz 1][mat.col1 - mat.col3*y_bottom] > 0
        f.m_plane_bottom = mat_column[1] - (mat_column[3] * y_bottom);
        // normalize
        f.m_plane_bottom *= (1.0 / glm::length(glm::vec3(f.m_plane_bottom.x, f.m_plane_bottom.y, f.m_plane_bottom.z)));

        // [vec.xyz 1][mat.col2] / [vec.xyz 1][mat.col3] < z_near
        // [vec.xyz 1][mat.col3*z_near - mat.col2] > 0
        f.m_plane_near = (mat_column[3] * z_near) - mat_column[2];
        f.m_plane_near *= (1.0 / glm::length(glm::vec3(f.m_plane_near.x, f.m_plane_near.y, f.m_plane_near.z)));

        // [vec.xyz 1][mat.col2] / [vec.xyz 1][mat.col3] > z_far
        // [vec.xyz 1][mat.col2 - mat.col3*z_far] > 0
        f.m_plane_far = mat_column[2] - (mat_column[3] * z_far);
        f.m_plane_far *= (1.0 / glm::length(glm::vec3(f.m_plane_far.x, f.m_plane_far.y, f.m_plane_far.z)));

        return f;
    }

    bool TiledFrustumIntersectBox(ClusterFrustum const& f, BoundingBox const& b)
    {
        // we follow the "DirectX::BoundingFrustum::Intersects"

        // Center of the box.
        glm::vec4 box_center((b.max_bound.x + b.min_bound.x) * 0.5,
                             (b.max_bound.y + b.min_bound.y) * 0.5,
                             (b.max_bound.z + b.min_bound.z) * 0.5,
                             1.0);

        // Distance from the center to each side.
        // half extent //more exactly
        glm::vec3 box_extents((b.max_bound.x - b.min_bound.x) * 0.5,
                              (b.max_bound.y - b.min_bound.y) * 0.5,
                              (b.max_bound.z - b.min_bound.z) * 0.5);

        // plane_right
        {
            float signed_distance_from_plane_right = glm::dot(f.m_plane_right, box_center);
            float radius_project_plane_right =
                glm::dot(glm::abs(glm::vec3(f.m_plane_right.x, f.m_plane_right.y, f.m_plane_right.z)), box_extents);

            bool intersecting_or_inside_right = signed_distance_from_plane_right < radius_project_plane_right;
            if (!intersecting_or_inside_right)
            {
                return false;
            }
        }

        // plane_left
        {
            float signed_distance_from_plane_left = glm::dot(f.m_plane_left, box_center);
            float radius_project_plane_left =
                glm::dot(glm::abs(glm::vec3(f.m_plane_left.x, f.m_plane_left.y, f.m_plane_left.z)), box_extents);

            bool intersecting_or_inside_left = signed_distance_from_plane_left < radius_project_plane_left;
            if (!intersecting_or_inside_left)
            {
                return false;
            }
        }

        // plane_top
        {
            float signed_distance_from_plane_top = glm::dot(f.m_plane_top, box_center);
            float radius_project_plane_top =
                glm::dot(glm::abs(glm::vec3(f.m_plane_top.x, f.m_plane_top.y, f.m_plane_top.z)), box_extents);

            bool intersecting_or_inside_top = signed_distance_from_plane_top < radius_project_plane_top;
            if (!intersecting_or_inside_top)
            {
                return false;
            }
        }

        // plane_bottom
        {
            float signed_distance_from_plane_bottom = glm::dot(f.m_plane_bottom, box_center);
            float radius_project_plane_bottom =
                glm::dot(glm::abs(glm::vec3(f.m_plane_bottom.x, f.m_plane_bottom.y, f.m_plane_bottom.z)), box_extents);

            bool intersecting_or_inside_bottom = signed_distance_from_plane_bottom < radius_project_plane_bottom;
            if (!intersecting_or_inside_bottom)
            {
                return false;
            }
        }

        // plane_near
        {
            float signed_distance_from_plane_near = glm::dot(f.m_plane_near, box_center);
            float radius_project_plane_near =
                glm::dot(glm::abs(glm::vec3(f.m_plane_near.x, f.m_plane_near.y, f.m_plane_near.z)), box_extents);

            bool intersecting_or_inside_near = signed_distance_from_plane_near < radius_project_plane_near;
            if (!intersecting_or_inside_near)
            {
                return false;
            }
        }

        // plane_far
        {
            float signed_distance_from_plane_far = glm::dot(f.m_plane_far, box_center);
            float radius_project_plane_far =
                glm::dot(glm::abs(glm::vec3(f.m_plane_far.x, f.m_plane_far.y, f.m_plane_far.z)), box_extents);

            bool intersecting_or_inside_far = signed_distance_from_plane_far < radius_project_plane_far;
            if (!intersecting_or_inside_far)
            {
                return false;
            }
        }

        return true;
    }

    BoundingBox BoundingBoxTransform(BoundingBox const& b, glm::mat4 const& m)
    {
        // we follow the "BoundingBox::Transform"

        glm::vec3 const g_BoxOffset[8] = {glm::vec3(-1.0f, -1.0f, 1.0f),
                                          glm::vec3(1.0f, -1.0f, 1.0f),
                                          glm::vec3(1.0f, 1.0f, 1.0f),
                                          glm::vec3(-1.0f, 1.0f, 1.0f),
                                          glm::vec3(-1.0f, -1.0f, -1.0f),
                                          glm::vec3(1.0f, -1.0f, -1.0f),
                                          glm::vec3(1.0f, 1.0f, -1.0f),
                                          glm::vec3(-1.0f, 1.0f, -1.0f)};

        size_t const CORNER_COUNT = 8;

        // Load center and extents.
        // Center of the box.
        glm::vec3 center((b.max_bound.x + b.min_bound.x) * 0.5,
                         (b.max_bound.y + b.min_bound.y) * 0.5,
                         (b.max_bound.z + b.min_bound.z) * 0.5);

        // Distance from the center to each side.
        // half extent //more exactly
        glm::vec3 extents((b.max_bound.x - b.min_bound.x) * 0.5,
                          (b.max_bound.y - b.min_bound.y) * 0.5,
                          (b.max_bound.z - b.min_bound.z) * 0.5);

        glm::vec3 min;
        glm::vec3 max;

        // Compute and transform the corners and find new min/max bounds.
        for (size_t i = 0; i < CORNER_COUNT; ++i)
        {
            glm::vec3 corner_before = extents * g_BoxOffset[i] + center;
            glm::vec4 corner_with_w = m * glm::vec4(corner_before.x, corner_before.y, corner_before.z, 1.0);
            glm::vec3 corner        = glm::vec3(corner_with_w.x / corner_with_w.w,
                                         corner_with_w.y / corner_with_w.w,
                                         corner_with_w.z / corner_with_w.w);

            if (0 == i)
            {
                min = corner;
                max = corner;
            }
            else
            {
                min = glm::min(min, corner);
                max = glm::max(max, corner);
            }
        }

        BoundingBox b_out;
        b_out.max_bound = GLMUtil::toVec3(max);
        b_out.min_bound = GLMUtil::toVec3(min);
        return b_out;
    }

    bool BoxIntersectsWithSphere(BoundingBox const& b, BoundingSphere const& s)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            if (s.m_center[i] < b.min_bound[i])
            {
                if ((b.min_bound[i] - s.m_center[i]) > s.m_radius)
                {
                    return false;
                }
            }
            else if (s.m_center[i] > b.max_bound[i])
            {
                if ((s.m_center[i] - b.max_bound[i]) > s.m_radius)
                {
                    return false;
                }
            }
        }

        return true;
    }

    glm::mat4 CalculateDirectionalLightCamera(RenderScene& scene, RenderCamera& camera)
    {
        Matrix4x4 proj_view_matrix;
        {
            Matrix4x4 view_matrix = camera.getViewMatrix();
            Matrix4x4 proj_matrix = camera.getPersProjMatrix();
            proj_view_matrix      = proj_matrix * view_matrix;
        }

        BoundingBox frustum_bounding_box;
        // CascadedShadowMaps11 / CreateFrustumPointsFromCascadeInterval
        {
            glm::vec3 const g_frustum_points_ndc_space[8] = {glm::vec3(-1.0f, -1.0f, 1.0f),
                                                             glm::vec3(1.0f, -1.0f, 1.0f),
                                                             glm::vec3(1.0f, 1.0f, 1.0f),
                                                             glm::vec3(-1.0f, 1.0f, 1.0f),
                                                             glm::vec3(-1.0f, -1.0f, 0.0f),
                                                             glm::vec3(1.0f, -1.0f, 0.0f),
                                                             glm::vec3(1.0f, 1.0f, 0.0f),
                                                             glm::vec3(-1.0f, 1.0f, 0.0f)};

            glm::mat4 inverse_proj_view_matrix = glm::inverse(GLMUtil::fromMat4x4(proj_view_matrix));

            frustum_bounding_box.min_bound = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
            frustum_bounding_box.max_bound = Vector3(FLT_MIN, FLT_MIN, FLT_MIN);

            size_t const CORNER_COUNT = 8;
            for (size_t i = 0; i < CORNER_COUNT; ++i)
            {
                glm::vec4 frustum_point_with_w = inverse_proj_view_matrix * glm::vec4(g_frustum_points_ndc_space[i].x,
                                                                                      g_frustum_points_ndc_space[i].y,
                                                                                      g_frustum_points_ndc_space[i].z,
                                                                                      1.0);
                Vector3   frustum_point        = Vector3(frustum_point_with_w.x / frustum_point_with_w.w,
                                                frustum_point_with_w.y / frustum_point_with_w.w,
                                                frustum_point_with_w.z / frustum_point_with_w.w);

                frustum_bounding_box.merge(frustum_point);
            }
        }

        BoundingBox scene_bounding_box;
        {
            scene_bounding_box.min_bound = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
            scene_bounding_box.max_bound = Vector3(FLT_MIN, FLT_MIN, FLT_MIN);

            for (const RenderEntity& entity : scene.m_render_entities)
            {
                BoundingBox mesh_asset_bounding_box {entity.m_bounding_box.getMinCorner(),
                                                     entity.m_bounding_box.getMaxCorner()};

                BoundingBox mesh_bounding_box_world =
                    BoundingBoxTransform(mesh_asset_bounding_box, GLMUtil::fromMat4x4(entity.m_model_matrix));
                scene_bounding_box.merge(mesh_bounding_box_world);
            }
        }

        // CascadedShadowMaps11 / ComputeNearAndFar
        glm::mat4 light_view;
        glm::mat4 light_proj;
        {
            glm::vec3 box_center((frustum_bounding_box.max_bound.x + frustum_bounding_box.min_bound.x) * 0.5,
                                 (frustum_bounding_box.max_bound.y + frustum_bounding_box.min_bound.y) * 0.5,
                                 (frustum_bounding_box.max_bound.z + frustum_bounding_box.min_bound.z));
            glm::vec3 box_extents((frustum_bounding_box.max_bound.x - frustum_bounding_box.min_bound.x) * 0.5,
                                  (frustum_bounding_box.max_bound.y - frustum_bounding_box.min_bound.y) * 0.5,
                                  (frustum_bounding_box.max_bound.z - frustum_bounding_box.min_bound.z) * 0.5);

            glm::vec3 eye =
                box_center + GLMUtil::fromVec3(scene.m_directional_light.m_direction) * glm::length(box_extents);
            glm::vec3 center = box_center;
            light_view       = glm::lookAtRH(eye, center, glm::vec3(0.0, 0.0, 1.0));

            BoundingBox frustum_bounding_box_light_view = BoundingBoxTransform(frustum_bounding_box, light_view);
            BoundingBox scene_bounding_box_light_view   = BoundingBoxTransform(scene_bounding_box, light_view);

            light_proj = glm::orthoRH(
                std::max(frustum_bounding_box_light_view.min_bound.x, scene_bounding_box_light_view.min_bound.x),
                std::min(frustum_bounding_box_light_view.max_bound.x, scene_bounding_box_light_view.max_bound.x),
                std::max(frustum_bounding_box_light_view.min_bound.y, scene_bounding_box_light_view.min_bound.y),
                std::min(frustum_bounding_box_light_view.max_bound.y, scene_bounding_box_light_view.max_bound.y),
                -scene_bounding_box_light_view.max_bound
                     .z, // the objects which are nearer than the frustum bounding box may caster shadow as well
                -std::max(frustum_bounding_box_light_view.min_bound.z, scene_bounding_box_light_view.min_bound.z));
        }

        glm::mat4 light_proj_view = (light_proj * light_view);
        return light_proj_view;
    }
} // namespace Pilot
