#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include "runtime/function/render/include/render/glm_wrapper.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <cstring>

namespace Pilot
{
    void PVulkanManager::updateUIRenderSceneViewport(VkViewport render_scene_viewport)
    {
        m_viewport = render_scene_viewport;

        m_scissor.offset = {0, 0};
        m_scissor.extent = m_vulkan_context._swapchain_extent;
    }

    uint32_t PVulkanManager::getGuidOfPickedMesh(Vector2 picked_uv)
    {
        return m_mouse_pick_pass.pick(glm::vec2(picked_uv.x, picked_uv.y));
    }

    float intersectPlaneRay(glm::vec3 normal, float d, glm::vec3 origin, glm::vec3 dir)
    {
        float deno = glm::dot(normal, dir);
        if (fabs(deno) < 0.0001)
        {
            deno = 0.0001;
        }

        return -(glm::dot(normal, origin) + d) / deno;
    }

    size_t PVulkanManager::updateCursorOnAxis(int     axis_mode,
                                              Vector2 cursor_uv,
                                              Vector2 game_engine_window_size,
                                              float   camera_fov,
                                              Vector3 camera_forward,
                                              Vector3 camera_up,
                                              Vector3 camera_right,
                                              Vector3 camera_position)
    {
        m_selected_axis = 3;
        if (m_axis_node.ref_mesh == NULL)
        {
            return m_selected_axis;
        }
        if (m_is_show_axis == false)
        {
            return m_selected_axis;
        }
        else
        {
            glm::mat4 model_matrix = m_axis_node.model_matrix;
            glm::vec3 model_scale;
            glm::quat model_rotation;
            glm::vec3 model_translation;
            glm::vec3 model_skew;
            glm::vec4 model_perspective;
            glm::decompose(model_matrix, model_scale, model_rotation, model_translation, model_skew, model_perspective);
            float     window_forward   = game_engine_window_size.y / 2.0f / glm::tan(glm::radians(camera_fov) / 2.0f);
            glm::vec2 screen_center_uv = glm::vec2(cursor_uv.x, 1 - cursor_uv.y) - glm::vec2(0.5, 0.5);
            glm::vec3 world_ray_dir =
                GLMUtil::fromVec3(camera_forward) * window_forward +
                GLMUtil::fromVec3(camera_right) * (float)game_engine_window_size.x * screen_center_uv.x +
                GLMUtil::fromVec3(camera_up) * (float)game_engine_window_size.y * screen_center_uv.y;

            glm::vec4 local_ray_origin =
                glm::inverse(model_matrix) * glm::vec4(GLMUtil::fromVec3(camera_position), 1.0f);
            glm::vec3 local_ray_origin_xyz = glm::vec3(local_ray_origin.x, local_ray_origin.y, local_ray_origin.z);
            glm::vec3 local_ray_dir        = glm::normalize(glm::inverse(model_rotation)) * world_ray_dir;

            glm::vec3 plane_normals[3] = {glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)};

            float plane_view_depth[3] = {intersectPlaneRay(plane_normals[0], 0, local_ray_origin_xyz, local_ray_dir),
                                         intersectPlaneRay(plane_normals[1], 0, local_ray_origin_xyz, local_ray_dir),
                                         intersectPlaneRay(plane_normals[2], 0, local_ray_origin_xyz, local_ray_dir)};

            glm::vec3 intersect_pt[3] = {
                local_ray_origin_xyz + plane_view_depth[0] * local_ray_dir, // yoz
                local_ray_origin_xyz + plane_view_depth[1] * local_ray_dir, // xoz
                local_ray_origin_xyz + plane_view_depth[2] * local_ray_dir  // xoy
            };

            if (axis_mode == 0 || axis_mode == 2) // transition axis & scale axis
            {
                const float DIST_THRESHOLD   = 0.6f;
                const float EDGE_OF_AXIS_MIN = 0.1f;
                const float EDGE_OF_AXIS_MAX = 2.0f;
                const float AXIS_LENGTH      = 2.0f;

                float max_dist = 0.0f;
                // whether the ray (camera to mouse point) on any plane
                for (int i = 0; i < 3; ++i)
                {
                    float local_ray_dir_proj = glm::abs(glm::dot(local_ray_dir, plane_normals[i]));
                    float cos_alpha          = local_ray_dir_proj / 1.0f; // local_ray_dir_proj / local_ray_dir.length
                    if (cos_alpha <= 0.15)                                // cos(80deg)~cps(100deg)
                    {
                        int   index00   = (i + 1) % 3;
                        int   index01   = 3 - i - index00;
                        int   index10   = (i + 2) % 3;
                        int   index11   = 3 - i - index10;
                        float axis_dist = (glm::abs(intersect_pt[index00][i]) + glm::abs(intersect_pt[index10][i])) / 2;
                        if (axis_dist > DIST_THRESHOLD) // too far from axis
                        {
                            continue;
                        }
                        // which axis is closer
                        if ((intersect_pt[index00][index01] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index00][index01] < AXIS_LENGTH) &&
                            (intersect_pt[index00][index01] > max_dist) &&
                            (glm::abs(intersect_pt[index00][i]) < EDGE_OF_AXIS_MAX))
                        {
                            max_dist        = intersect_pt[index00][index01];
                            m_selected_axis = index01;
                        }
                        if ((intersect_pt[index10][index11] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index10][index11] < AXIS_LENGTH) &&
                            (intersect_pt[index10][index11] > max_dist) &&
                            (glm::abs(intersect_pt[index10][i]) < EDGE_OF_AXIS_MAX))
                        {
                            max_dist        = intersect_pt[index10][index11];
                            m_selected_axis = index11;
                        }
                    }
                }
                // check axis
                if (m_selected_axis == 3)
                {
                    float min_dist = 1e10f;
                    for (int i = 0; i < 3; ++i)
                    {
                        int   index0 = (i + 1) % 3;
                        int   index1 = (i + 2) % 3;
                        float dist =
                            glm::pow(intersect_pt[index0][index1], 2) + glm::pow(intersect_pt[index1][index0], 2);
                        if ((intersect_pt[index0][i] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index0][i] < EDGE_OF_AXIS_MAX) && (dist < DIST_THRESHOLD) &&
                            (dist < min_dist))
                        {
                            min_dist        = dist;
                            m_selected_axis = i;
                        }
                    }
                }
            }
            else if (axis_mode == 1) // rotation axis
            {
                const float DIST_THRESHOLD = 0.2f;

                float min_dist = 1e10f;
                for (int i = 0; i < 3; ++i)
                {
                    const float dist =
                        std::fabs(1 - std::hypot(intersect_pt[i].x, intersect_pt[i].y, intersect_pt[i].z));
                    if ((dist < DIST_THRESHOLD) && (dist < min_dist))
                    {
                        min_dist        = dist;
                        m_selected_axis = i;
                    }
                }
            }
            else
            {
                return m_selected_axis;
            }
        }
        return m_selected_axis;
    }
} // namespace Pilot
