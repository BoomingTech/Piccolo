#include <cassert>
#include <mutex>

#include "editor/include/editor.h"
#include "editor/include/editor_global_context.h"
#include "editor/include/editor_scene_manager.h"

#include "runtime/core/base/macro.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_system.h"

namespace Piccolo
{
    void EditorSceneManager::initialize() {}

    void EditorSceneManager::tick(float delta_time)
    {
        std::shared_ptr<GObject> selected_gobject = getSelectedGObject().lock();
        if (selected_gobject)
        {
            TransformComponent* transform_component = selected_gobject->tryGetComponent(TransformComponent);
            if (transform_component)
            {
                transform_component->setDirtyFlag(true);
            }
        }
    }

    float intersectPlaneRay(Vector3 normal, float d, Vector3 origin, Vector3 dir)
    {
        float deno = normal.dotProduct(dir);
        if (fabs(deno) < 0.0001)
        {
            deno = 0.0001;
        }

        return -(normal.dotProduct(origin) + d) / deno;
    }

    size_t EditorSceneManager::updateCursorOnAxis(Vector2 cursor_uv, Vector2 game_engine_window_size)
    {

        float   camera_fov     = m_camera->getFovYDeprecated();
        Vector3 camera_forward = m_camera->forward();

        Vector3 camera_up       = m_camera->up();
        Vector3 camera_right    = m_camera->right();
        Vector3 camera_position = m_camera->position();

        if (m_selected_gobject_id == k_invalid_gobject_id)
        {
            return m_selected_axis;
        }
        RenderEntity* selected_aixs = getAxisMeshByType(m_axis_mode);
        m_selected_axis             = 3;
        if (m_is_show_axis == false)
        {
            return m_selected_axis;
        }
        else
        {
            Matrix4x4 model_matrix = selected_aixs->m_model_matrix;
            Vector3 model_scale;
            Quaternion model_rotation;
            Vector3 model_translation;
            model_matrix.decomposition(model_translation, model_scale, model_rotation);
            float     window_forward   = game_engine_window_size.y / 2.0f / Math::tan(Math::degreesToRadians(camera_fov) / 2.0f);
            Vector2 screen_center_uv = Vector2(cursor_uv.x, 1 - cursor_uv.y) - Vector2(0.5, 0.5);
            Vector3 world_ray_dir =
                camera_forward * window_forward +
                camera_right * (float)game_engine_window_size.x * screen_center_uv.x +
                camera_up * (float)game_engine_window_size.y * screen_center_uv.y;

            Vector4 local_ray_origin =
                model_matrix.inverse() * Vector4(camera_position, 1.0f);
            Vector3 local_ray_origin_xyz = Vector3(local_ray_origin.x, local_ray_origin.y, local_ray_origin.z);
            Quaternion inversed_rotation = model_rotation.inverse();
            inversed_rotation.normalise();
            Vector3 local_ray_dir        = inversed_rotation * world_ray_dir;

            Vector3 plane_normals[3] = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};

            float plane_view_depth[3] = {intersectPlaneRay(plane_normals[0], 0, local_ray_origin_xyz, local_ray_dir),
                                         intersectPlaneRay(plane_normals[1], 0, local_ray_origin_xyz, local_ray_dir),
                                         intersectPlaneRay(plane_normals[2], 0, local_ray_origin_xyz, local_ray_dir)};

            Vector3 intersect_pt[3] = {
                local_ray_origin_xyz + plane_view_depth[0] * local_ray_dir, // yoz
                local_ray_origin_xyz + plane_view_depth[1] * local_ray_dir, // xoz
                local_ray_origin_xyz + plane_view_depth[2] * local_ray_dir  // xoy
            };

            if ((int)m_axis_mode == 0 || (int)m_axis_mode == 2) // transition axis & scale axis
            {
                const float DIST_THRESHOLD   = 0.6f;
                const float EDGE_OF_AXIS_MIN = 0.1f;
                const float EDGE_OF_AXIS_MAX = 2.0f;
                const float AXIS_LENGTH      = 2.0f;

                float max_dist = 0.0f;
                // whether the ray (camera to mouse point) on any plane
                for (int i = 0; i < 3; ++i)
                {
                    float local_ray_dir_proj = Math::abs(local_ray_dir.dotProduct(plane_normals[i]));
                    float cos_alpha          = local_ray_dir_proj / 1.0f; // local_ray_dir_proj / local_ray_dir.length
                    if (cos_alpha <= 0.15)                                // cos(80deg)~cps(100deg)
                    {
                        int   index00   = (i + 1) % 3;
                        int   index01   = 3 - i - index00;
                        int   index10   = (i + 2) % 3;
                        int   index11   = 3 - i - index10;
                        float axis_dist = (Math::abs(intersect_pt[index00][i]) + Math::abs(intersect_pt[index10][i])) / 2;
                        if (axis_dist > DIST_THRESHOLD) // too far from axis
                        {
                            continue;
                        }
                        // which axis is closer
                        if ((intersect_pt[index00][index01] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index00][index01] < AXIS_LENGTH) &&
                            (intersect_pt[index00][index01] > max_dist) &&
                            (Math::abs(intersect_pt[index00][i]) < EDGE_OF_AXIS_MAX))
                        {
                            max_dist        = intersect_pt[index00][index01];
                            m_selected_axis = index01;
                        }
                        if ((intersect_pt[index10][index11] > EDGE_OF_AXIS_MIN) &&
                            (intersect_pt[index10][index11] < AXIS_LENGTH) &&
                            (intersect_pt[index10][index11] > max_dist) &&
                            (Math::abs(intersect_pt[index10][i]) < EDGE_OF_AXIS_MAX))
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
                            Math::sqr(intersect_pt[index0][index1]) + Math::sqr(intersect_pt[index1][index0]);
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
            else if ((int)m_axis_mode == 1) // rotation axis
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

        g_editor_global_context.m_render_system->setSelectedAxis(m_selected_axis);

        return m_selected_axis;
    }

    RenderEntity* EditorSceneManager::getAxisMeshByType(EditorAxisMode axis_mode)
    {
        RenderEntity* axis_mesh = nullptr;
        switch (axis_mode)
        {
            case EditorAxisMode::TranslateMode:
                axis_mesh = &m_translation_axis;
                break;
            case EditorAxisMode::RotateMode:
                axis_mesh = &m_rotation_axis;
                break;
            case EditorAxisMode::ScaleMode:
                axis_mesh = &m_scale_aixs;
                break;
            default:
                break;
        }
        return axis_mesh;
    }

    void EditorSceneManager::drawSelectedEntityAxis()
    {
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();

        if (g_is_editor_mode && selected_object != nullptr)
        {
            const TransformComponent* transform_component = selected_object->tryGetComponentConst(TransformComponent);

            Vector3    scale;
            Quaternion rotation;
            Vector3    translation;
            transform_component->getMatrix().decomposition(translation, scale, rotation);
            Matrix4x4     translation_matrix = Matrix4x4::getTrans(translation);
            Matrix4x4     scale_matrix       = Matrix4x4::buildScaleMatrix(1.0f, 1.0f, 1.0f);
            Matrix4x4     axis_model_matrix  = translation_matrix * scale_matrix;
            RenderEntity* selected_aixs      = getAxisMeshByType(m_axis_mode);
            if (m_axis_mode == EditorAxisMode::TranslateMode || m_axis_mode == EditorAxisMode::RotateMode)
            {
                selected_aixs->m_model_matrix = axis_model_matrix;
            }
            else if (m_axis_mode == EditorAxisMode::ScaleMode)
            {
                selected_aixs->m_model_matrix = axis_model_matrix * Matrix4x4(rotation);
            }

            g_editor_global_context.m_render_system->setVisibleAxis(*selected_aixs);
        }
        else
        {
            g_editor_global_context.m_render_system->setVisibleAxis(std::nullopt);
        }
    }

    std::weak_ptr<GObject> EditorSceneManager::getSelectedGObject() const
    {
        std::weak_ptr<GObject> selected_object;
        if (m_selected_gobject_id != k_invalid_gobject_id)
        {
            std::shared_ptr<Level> level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
            if (level != nullptr)
            {
                selected_object = level->getGObjectByID(m_selected_gobject_id);
            }
        }
        return selected_object;
    }

    void EditorSceneManager::onGObjectSelected(GObjectID selected_gobject_id)
    {
        if (selected_gobject_id == m_selected_gobject_id)
            return;

        m_selected_gobject_id = selected_gobject_id;

        std::shared_ptr<GObject> selected_gobject = getSelectedGObject().lock();
        if (selected_gobject)
        {
            const TransformComponent* transform_component = selected_gobject->tryGetComponentConst(TransformComponent);
            m_selected_object_matrix                      = transform_component->getMatrix();
        }

        drawSelectedEntityAxis();

        if (m_selected_gobject_id != k_invalid_gobject_id)
        {
            LOG_INFO("select game object " + std::to_string(m_selected_gobject_id));
        }
        else
        {
            LOG_INFO("no game object selected");
        }
    }

    void EditorSceneManager::onDeleteSelectedGObject()
    {
        // delete selected entity
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();
        if (selected_object != nullptr)
        {
            std::shared_ptr<Level> current_active_level =
                g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
            if (current_active_level == nullptr)
                return;

            current_active_level->deleteGObjectByID(m_selected_gobject_id);

            RenderSwapContext& swap_context = g_editor_global_context.m_render_system->getSwapContext();
            swap_context.getLogicSwapData().addDeleteGameObject(GameObjectDesc {selected_object->getID(), {}});
        }
        onGObjectSelected(k_invalid_gobject_id);
    }

    void EditorSceneManager::moveEntity(float     new_mouse_pos_x,
                                        float     new_mouse_pos_y,
                                        float     last_mouse_pos_x,
                                        float     last_mouse_pos_y,
                                        Vector2   engine_window_pos,
                                        Vector2   engine_window_size,
                                        size_t    cursor_on_axis,
                                        Matrix4x4 model_matrix)
    {
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();
        if (selected_object == nullptr)
            return;

        float angularVelocity =
            18.0f / Math::max(engine_window_size.x, engine_window_size.y); // 18 degrees while moving full screen
        Vector2 delta_mouse_move_uv = {(new_mouse_pos_x - last_mouse_pos_x), (new_mouse_pos_y - last_mouse_pos_y)};

        Vector3    model_scale;
        Quaternion model_rotation;
        Vector3    model_translation;
        model_matrix.decomposition(model_translation, model_scale, model_rotation);

        Matrix4x4 axis_model_matrix = Matrix4x4::IDENTITY;
        axis_model_matrix.setTrans(model_translation);

        Matrix4x4 view_matrix = m_camera->getLookAtMatrix();
        Matrix4x4 proj_matrix = m_camera->getPersProjMatrix();

        Vector4 model_world_position_4(model_translation, 1.f);

        Vector4 model_origin_clip_position = proj_matrix * view_matrix * model_world_position_4;
        model_origin_clip_position /= model_origin_clip_position.w;
        Vector2 model_origin_clip_uv =
            Vector2((model_origin_clip_position.x + 1) / 2.0f, (model_origin_clip_position.y + 1) / 2.0f);

        Vector4 axis_x_local_position_4(1, 0, 0, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_x_local_position_4 = Matrix4x4(model_rotation) * axis_x_local_position_4;
        }
        Vector4 axis_x_world_position_4 = axis_model_matrix * axis_x_local_position_4;
        axis_x_world_position_4.w       = 1.0f;
        Vector4 axis_x_clip_position    = proj_matrix * view_matrix * axis_x_world_position_4;
        axis_x_clip_position /= axis_x_clip_position.w;
        Vector2 axis_x_clip_uv((axis_x_clip_position.x + 1) / 2.0f, (axis_x_clip_position.y + 1) / 2.0f);
        Vector2 axis_x_direction_uv = axis_x_clip_uv - model_origin_clip_uv;
        axis_x_direction_uv.normalise();

        Vector4 axis_y_local_position_4(0, 1, 0, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_y_local_position_4 = Matrix4x4(model_rotation) * axis_y_local_position_4;
        }
        Vector4 axis_y_world_position_4 = axis_model_matrix * axis_y_local_position_4;
        axis_y_world_position_4.w       = 1.0f;
        Vector4 axis_y_clip_position    = proj_matrix * view_matrix * axis_y_world_position_4;
        axis_y_clip_position /= axis_y_clip_position.w;
        Vector2 axis_y_clip_uv((axis_y_clip_position.x + 1) / 2.0f, (axis_y_clip_position.y + 1) / 2.0f);
        Vector2 axis_y_direction_uv = axis_y_clip_uv - model_origin_clip_uv;
        axis_y_direction_uv.normalise();

        Vector4 axis_z_local_position_4(0, 0, 1, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_z_local_position_4 = Matrix4x4(model_rotation) * axis_z_local_position_4;
        }
        Vector4 axis_z_world_position_4 = axis_model_matrix * axis_z_local_position_4;
        axis_z_world_position_4.w       = 1.0f;
        Vector4 axis_z_clip_position    = proj_matrix * view_matrix * axis_z_world_position_4;
        axis_z_clip_position /= axis_z_clip_position.w;
        Vector2 axis_z_clip_uv((axis_z_clip_position.x + 1) / 2.0f, (axis_z_clip_position.y + 1) / 2.0f);
        Vector2 axis_z_direction_uv = axis_z_clip_uv - model_origin_clip_uv;
        axis_z_direction_uv.normalise();

        TransformComponent* transform_component = selected_object->tryGetComponent(TransformComponent);

        Matrix4x4 new_model_matrix(Matrix4x4::IDENTITY);
        if (m_axis_mode == EditorAxisMode::TranslateMode) // translate
        {
            Vector3 move_vector = {0, 0, 0};
            if (cursor_on_axis == 0)
            {
                move_vector.x = delta_mouse_move_uv.dotProduct(axis_x_direction_uv) * angularVelocity;
            }
            else if (cursor_on_axis == 1)
            {
                move_vector.y = delta_mouse_move_uv.dotProduct(axis_y_direction_uv) * angularVelocity;
            }
            else if (cursor_on_axis == 2)
            {
                move_vector.z = delta_mouse_move_uv.dotProduct(axis_z_direction_uv) * angularVelocity;
            }
            else
            {
                return;
            }

            Matrix4x4 translate_mat;
            translate_mat.makeTransform(move_vector, Vector3::UNIT_SCALE, Quaternion::IDENTITY);
            new_model_matrix = axis_model_matrix * translate_mat;

            new_model_matrix = new_model_matrix * Matrix4x4(model_rotation);
            new_model_matrix =
                new_model_matrix * Matrix4x4::buildScaleMatrix(model_scale.x, model_scale.y, model_scale.z);

            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;
            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            Matrix4x4 translation_matrix = Matrix4x4::getTrans(new_translation);
            Matrix4x4 scale_matrix       = Matrix4x4::buildScaleMatrix(1.f, 1.f, 1.f);
            Matrix4x4 axis_model_matrix  = translation_matrix * scale_matrix;

            m_translation_axis.m_model_matrix = axis_model_matrix;
            m_rotation_axis.m_model_matrix    = axis_model_matrix;
            m_scale_aixs.m_model_matrix       = axis_model_matrix;

            g_editor_global_context.m_render_system->setVisibleAxis(m_translation_axis);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
        }
        else if (m_axis_mode == EditorAxisMode::RotateMode) // rotate
        {
            float   last_mouse_u = (last_mouse_pos_x - engine_window_pos.x) / engine_window_size.x;
            float   last_mouse_v = (last_mouse_pos_y - engine_window_pos.y) / engine_window_size.y;
            Vector2 last_move_vector(last_mouse_u - model_origin_clip_uv.x, last_mouse_v - model_origin_clip_uv.y);
            float   new_mouse_u = (new_mouse_pos_x - engine_window_pos.x) / engine_window_size.x;
            float   new_mouse_v = (new_mouse_pos_y - engine_window_pos.y) / engine_window_size.y;
            Vector2 new_move_vector(new_mouse_u - model_origin_clip_uv.x, new_mouse_v - model_origin_clip_uv.y);
            Vector3 delta_mouse_uv_3(delta_mouse_move_uv.x, delta_mouse_move_uv.y, 0);
            float   move_radian;
            Vector3 axis_of_rotation = {0, 0, 0};
            if (cursor_on_axis == 0)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_camera->forward().dotProduct(Vector3::UNIT_X) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.x = 1;
            }
            else if (cursor_on_axis == 1)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_camera->forward().dotProduct(Vector3::UNIT_Y) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.y = 1;
            }
            else if (cursor_on_axis == 2)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_camera->forward().dotProduct(Vector3::UNIT_Z) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.z = 1;
            }
            else
            {
                return;
            }
            float move_direction = last_move_vector.x * new_move_vector.y - new_move_vector.x * last_move_vector.y;
            if (move_direction < 0)
            {
                move_radian = -move_radian;
            }

            Quaternion move_rot;
            move_rot.fromAngleAxis(Radian(move_radian), axis_of_rotation);
            new_model_matrix = axis_model_matrix * move_rot;
            new_model_matrix = new_model_matrix * Matrix4x4(model_rotation);
            new_model_matrix =
                new_model_matrix * Matrix4x4::buildScaleMatrix(model_scale.x, model_scale.y, model_scale.z);
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;

            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
            m_scale_aixs.m_model_matrix = new_model_matrix;
        }
        else if (m_axis_mode == EditorAxisMode::ScaleMode) // scale
        {
            Vector3 delta_scale_vector = {0, 0, 0};
            Vector3 new_model_scale    = {0, 0, 0};
            if (cursor_on_axis == 0)
            {
                delta_scale_vector.x = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_x_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else if (cursor_on_axis == 1)
            {
                delta_scale_vector.y = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_y_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else if (cursor_on_axis == 2)
            {
                delta_scale_vector.z = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_z_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else
            {
                return;
            }
            new_model_scale   = model_scale + delta_scale_vector;
            axis_model_matrix = axis_model_matrix * Matrix4x4(model_rotation);
            Matrix4x4 scale_mat;
            scale_mat.makeTransform(Vector3::ZERO, new_model_scale, Quaternion::IDENTITY);
            new_model_matrix = axis_model_matrix * scale_mat;
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;
            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
        }
        setSelectedObjectMatrix(new_model_matrix);
    }

    void EditorSceneManager::uploadAxisResource()
    {
        auto& instance_id_allocator   = g_editor_global_context.m_render_system->getGOInstanceIdAllocator();
        auto& mesh_asset_id_allocator = g_editor_global_context.m_render_system->getMeshAssetIdAllocator();

        // assign some value that won't be used by other game objects
        {
            GameObjectPartId axis_instance_id = {0xFFAA, 0xFFAA};
            MeshSourceDesc   mesh_source_desc = {"%%translation_axis%%"};

            m_translation_axis.m_instance_id   = instance_id_allocator.allocGuid(axis_instance_id);
            m_translation_axis.m_mesh_asset_id = mesh_asset_id_allocator.allocGuid(mesh_source_desc);
        }

        {
            GameObjectPartId axis_instance_id = {0xFFBB, 0xFFBB};
            MeshSourceDesc   mesh_source_desc = {"%%rotate_axis%%"};

            m_rotation_axis.m_instance_id   = instance_id_allocator.allocGuid(axis_instance_id);
            m_rotation_axis.m_mesh_asset_id = mesh_asset_id_allocator.allocGuid(mesh_source_desc);
        }

        {
            GameObjectPartId axis_instance_id = {0xFFCC, 0xFFCC};
            MeshSourceDesc   mesh_source_desc = {"%%scale_axis%%"};

            m_scale_aixs.m_instance_id   = instance_id_allocator.allocGuid(axis_instance_id);
            m_scale_aixs.m_mesh_asset_id = mesh_asset_id_allocator.allocGuid(mesh_source_desc);
        }

        g_editor_global_context.m_render_system->createAxis(
            {m_translation_axis, m_rotation_axis, m_scale_aixs},
            {m_translation_axis.m_mesh_data, m_rotation_axis.m_mesh_data, m_scale_aixs.m_mesh_data});
    }

    size_t EditorSceneManager::getGuidOfPickedMesh(const Vector2& picked_uv) const
    {
        return g_editor_global_context.m_render_system->getGuidOfPickedMesh(picked_uv);
    }
} // namespace Piccolo
