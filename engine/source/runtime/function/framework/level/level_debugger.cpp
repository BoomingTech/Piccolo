#include "level_debugger.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/engine.h"
#include "runtime/function/character/character.h"
#include "runtime/function/framework/component/camera/camera_component.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/resource/res_type/components/animation.h"

#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/function/global/global_context.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"
#include "runtime/function/render/render_debug_config.h"
namespace Piccolo
{
    void LevelDebugger::tick(std::shared_ptr<Level> level) const
    {
        if (g_is_editor_mode)
        {
            return;
        }

        if (g_runtime_global_context.m_render_debug_config->animation.show_bone_name)
        {
            showAllBonesName(level);
        }
        if (g_runtime_global_context.m_render_debug_config->animation.show_skeleton)
        {
            showAllBones(level);
        }
        if (g_runtime_global_context.m_render_debug_config->gameObject.show_bounding_box)
        {
            showAllBoundingBox(level);
        }
        if (g_runtime_global_context.m_render_debug_config->camera.show_runtime_info)
        {
            showCameraInfo(level);
        }
    }

    void LevelDebugger::showAllBones(std::shared_ptr<Level> level) const
    {
        const LevelObjectsMap& go_map = level->getAllGObjects();
        for (const auto& gobject_pair : go_map)
        {
            drawBones(gobject_pair.second);
        }
    }

    void LevelDebugger::showBones(std::shared_ptr<Level> level, GObjectID go_id) const
    {
        std::shared_ptr<GObject> gobject = level->getGObjectByID(go_id).lock();
        drawBones(gobject);
    }

    void LevelDebugger::showAllBonesName(std::shared_ptr<Level> level) const
    {
        const LevelObjectsMap& go_map = level->getAllGObjects();
        for (const auto& gobject_pair : go_map)
        {
            drawBonesName(gobject_pair.second);
        }
    }

    void LevelDebugger::showBonesName(std::shared_ptr<Level> level, GObjectID go_id) const
    {
        std::shared_ptr<GObject> gobject = level->getGObjectByID(go_id).lock();
        drawBonesName(gobject);
    }

    void LevelDebugger::showAllBoundingBox(std::shared_ptr<Level> level) const
    {
        const LevelObjectsMap& go_map = level->getAllGObjects();
        for (const auto& gobject_pair : go_map)
        {
            drawBoundingBox(gobject_pair.second);
        }
    }

    void LevelDebugger::showBoundingBox(std::shared_ptr<Level> level, GObjectID go_id) const
    {
        std::shared_ptr<GObject> gobject = level->getGObjectByID(go_id).lock();
        drawBoundingBox(gobject);
    }

    void LevelDebugger::showCameraInfo(std::shared_ptr<Level> level) const
    {
        std::shared_ptr<GObject> gobject = level->getCurrentActiveCharacter().lock()->getObject().lock();
        drawCameraInfo(gobject);
    }
    void LevelDebugger::drawBones(std::shared_ptr<GObject> object) const
    {
        const TransformComponent* transform_component =
            object->tryGetComponentConst<TransformComponent>("TransformComponent");
        const AnimationComponent* animation_component =
            object->tryGetComponentConst<AnimationComponent>("AnimationComponent");

        if (transform_component == nullptr || animation_component == nullptr)
            return;

        DebugDrawGroup* debug_draw_group =
            g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("bone");

        Matrix4x4 object_matrix = Transform(transform_component->getPosition(),
                                            transform_component->getRotation(),
                                            transform_component->getScale())
                                      .getMatrix();

        const Skeleton& skeleton    = animation_component->getSkeleton();
        const Bone*     bones       = skeleton.getBones();
        int32_t         bones_count = skeleton.getBonesCount();
        for (int32_t bone_index = 0; bone_index < bones_count; bone_index++)
        {
            if (bones[bone_index].getParent() == nullptr || bone_index == 1)
                continue;

            Matrix4x4 bone_matrix = Transform(bones[bone_index]._getDerivedPosition(),
                                              bones[bone_index]._getDerivedOrientation(),
                                              bones[bone_index]._getDerivedScale())
                                        .getMatrix();
            Vector4 bone_position(0.0f, 0.0f, 0.0f, 1.0f);
            bone_position = object_matrix * bone_matrix * bone_position;
            bone_position /= bone_position[3];

            Node*     parent_bone        = bones[bone_index].getParent();
            Matrix4x4 parent_bone_matrix = Transform(parent_bone->_getDerivedPosition(),
                                                     parent_bone->_getDerivedOrientation(),
                                                     parent_bone->_getDerivedScale())
                                               .getMatrix();
            Vector4 parent_bone_position(0.0f, 0.0f, 0.0f, 1.0f);
            parent_bone_position = object_matrix * parent_bone_matrix * parent_bone_position;
            parent_bone_position /= parent_bone_position[3];

            debug_draw_group->addLine(Vector3(bone_position.x, bone_position.y, bone_position.z),
                                      Vector3(parent_bone_position.x, parent_bone_position.y, parent_bone_position.z),
                                      Vector4(1.0f, 0.0f, 0.0f, 1.0f),
                                      Vector4(1.0f, 0.0f, 0.0f, 1.0f),
                                      0.0f,
                                      true);
            debug_draw_group->addSphere(Vector3(bone_position.x, bone_position.y, bone_position.z),
                                        0.015f,
                                        Vector4(0.0f, 0.0f, 1.0f, 1.0f),
                                        0.0f,
                                        true);
        }
    }

    void LevelDebugger::drawBonesName(std::shared_ptr<GObject> object) const
    {
        const TransformComponent* transform_component =
            object->tryGetComponentConst<TransformComponent>("TransformComponent");
        const AnimationComponent* animation_component =
            object->tryGetComponentConst<AnimationComponent>("AnimationComponent");

        if (transform_component == nullptr || animation_component == nullptr)
            return;

        DebugDrawGroup* debug_draw_group =
            g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("bone name");

        Matrix4x4 object_matrix = Transform(transform_component->getPosition(),
                                            transform_component->getRotation(),
                                            transform_component->getScale())
                                      .getMatrix();

        const Skeleton& skeleton    = animation_component->getSkeleton();
        const Bone*     bones       = skeleton.getBones();
        int32_t         bones_count = skeleton.getBonesCount();
        for (int32_t bone_index = 0; bone_index < bones_count; bone_index++)
        {
            if (bones[bone_index].getParent() == nullptr || bone_index == 1)
                continue;

            Matrix4x4 bone_matrix = Transform(bones[bone_index]._getDerivedPosition(),
                                              bones[bone_index]._getDerivedOrientation(),
                                              bones[bone_index]._getDerivedScale())
                                        .getMatrix();
            Vector4 bone_position(0.0f, 0.0f, 0.0f, 1.0f);
            bone_position = object_matrix * bone_matrix * bone_position;
            bone_position /= bone_position[3];

            debug_draw_group->addText(bones[bone_index].getName(),
                                      Vector4(1.0f, 0.0f, 0.0f, 1.0f),
                                      Vector3(bone_position.x, bone_position.y, bone_position.z),
                                      8,
                                      false);
        }
    }

    void LevelDebugger::drawBoundingBox(std::shared_ptr<GObject> object) const
    {
        const RigidBodyComponent* rigidbody_component =
            object->tryGetComponentConst<RigidBodyComponent>("RigidBodyComponent");
        if (rigidbody_component == nullptr)
            return;

        std::vector<AxisAlignedBox> bounding_boxes;
        rigidbody_component->getShapeBoundingBoxes(bounding_boxes);
        for (size_t bounding_box_index = 0; bounding_box_index < bounding_boxes.size(); bounding_box_index++)
        {
            AxisAlignedBox  bounding_box = bounding_boxes[bounding_box_index];
            DebugDrawGroup* debug_draw_group =
                g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("bounding box");
            Vector3 center =
                Vector3(bounding_box.getCenter().x, bounding_box.getCenter().y, bounding_box.getCenter().z);
            Vector3 halfExtent =
                Vector3(bounding_box.getHalfExtent().x, bounding_box.getHalfExtent().y, bounding_box.getHalfExtent().z);

            debug_draw_group->addBox(
                center, halfExtent, Vector4(1.0f, 0.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
        }
    }

    void LevelDebugger::drawCameraInfo(std::shared_ptr<GObject> object) const
    {
        const CameraComponent* camera_component = object->tryGetComponentConst<CameraComponent>("CameraComponent");
        if (camera_component == nullptr)
            return;

        DebugDrawGroup* debug_draw_group =
            g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("show camera info");

        std::ostringstream buffer;
        buffer << "camera mode: ";
        switch (camera_component->getCameraMode())
        {
            case CameraMode::first_person:
                buffer << "first person";
                break;
            case CameraMode::third_person:
                buffer << "third person";
                break;
            case CameraMode::free:
                buffer << "free";
                break;
            case CameraMode::invalid:
                buffer << "invalid";
                break;
        }
        buffer << std::endl;

        Vector3 position  = camera_component->getPosition();
        Vector3 forward   = camera_component->getForward();
        Vector3 direction = forward - position;
        buffer << "camera position: (" << position.x << "," << position.y << "," << position.z << ")" << std::endl;
        buffer << "camera direction : (" << direction.x << "," << direction.y << "," << direction.z << ")";
        debug_draw_group->addText(buffer.str(), Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector3(-1.0f, -0.2f, 0.0f), 10, true);
    }
} // namespace Piccolo