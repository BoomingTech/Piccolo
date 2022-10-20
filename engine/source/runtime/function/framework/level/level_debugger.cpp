#include "level_debugger.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/component/camera/camera_component.h"
#include "runtime/resource/res_type/components/animation.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/engine.h"
#include "runtime/function/character/character.h"

#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/function/global/global_context.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"
#include "runtime/function/render/render_debug_config.h"
namespace Piccolo
{
    void LevelDebugger::tick(std::shared_ptr<Level> level)
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

    void LevelDebugger::showAllBones(std::shared_ptr<Level> level)
    {
        const LevelObjectsMap& go_map = level->getAllGObjects();
        for (std::pair<GObjectID, std::shared_ptr<GObject>> gObject : go_map)
        {
            drawBones(gObject.second);
        }
    }
    void LevelDebugger::showBones(std::shared_ptr<Level> level, GObjectID go_id)
    {
        std::shared_ptr<GObject> gObject = level->getGObjectByID(go_id).lock();
        drawBones(gObject);
    }
    void LevelDebugger::showAllBonesName(std::shared_ptr<Level> level)
    {
        const LevelObjectsMap& go_map = level->getAllGObjects();
        for (std::pair<GObjectID, std::shared_ptr<GObject>> gObject : go_map)
        {
            drawBonesName(gObject.second);
        }
    }
    void LevelDebugger::showBonesName(std::shared_ptr<Level> level, GObjectID go_id)
    {
        std::shared_ptr<GObject> gObject = level->getGObjectByID(go_id).lock();
        drawBonesName(gObject);
    }
    void LevelDebugger::showAllBoundingBox(std::shared_ptr<Level> level)
    {
        const LevelObjectsMap& go_map = level->getAllGObjects();
        for (std::pair<GObjectID, std::shared_ptr<GObject>> gObject : go_map)
        {
            drawBoundingBox(gObject.second);
        }
    }
    void LevelDebugger::showBoundingBox(std::shared_ptr<Level> level, GObjectID go_id)
    {
        std::shared_ptr<GObject> gObject = level->getGObjectByID(go_id).lock();
        drawBoundingBox(gObject);
    }
    void LevelDebugger::showCameraInfo(std::shared_ptr<Level> level)
    {
        std::shared_ptr<GObject> gObject = level->getCurrentActiveCharacter().lock()->getObject().lock();
        drawCameraInfo(gObject);
    }
    void LevelDebugger::drawBones(std::shared_ptr<GObject> object)
    {
        TransformComponent* transform_component = object->tryGetComponent<TransformComponent>("TransformComponent");
        AnimationComponent* animation_component = object->tryGetComponent<AnimationComponent>("AnimationComponent");
        DebugDrawGroup* debug_draw_group = g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("bone");
        if (animation_component && transform_component)
        {
            Matrix4x4 objMat = Transform(transform_component->getPosition(), transform_component->getRotation(), transform_component->getScale()).getMatrix();
            const Skeleton& skeleton = animation_component->getSkeleton();
            const Bone* bones = skeleton.getBones();
            int32_t bones_count = skeleton.getBonesCount();
            for (int32_t i = 0; i < bones_count; i++)
            {
                if (bones[i].getParent() && i != 1)
                {
                    Matrix4x4 boneMat = Transform(bones[i]._getDerivedPosition(), bones[i]._getDerivedOrientation(), bones[i]._getDerivedScale()).getMatrix();
                    Vector4 pos(0.0f, 0.0f, 0.0f, 1.0f);
                    pos = objMat * boneMat * pos;
                    pos /= pos[3];

                    Node* pBone = bones[i].getParent();
                    Matrix4x4 parent_boneMat = Transform(pBone->_getDerivedPosition(), pBone->_getDerivedOrientation(), pBone->_getDerivedScale()).getMatrix();
                    Vector4 parent_pos(0.0f, 0.0f, 0.0f, 1.0f);
                    parent_pos = objMat * parent_boneMat * parent_pos;
                    parent_pos /= parent_pos[3];

                    debug_draw_group->addLine(Vector3(pos.x, pos.y, pos.z), Vector3(parent_pos.x, parent_pos.y, parent_pos.z), Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, true);
                    debug_draw_group->addSphere(Vector3(pos.x, pos.y, pos.z) , 0.015f, Vector4(0.0f, 0.0f, 1.0f, 1.0f), 0.0f, true);
                }
            }
        }
    }

    void LevelDebugger::drawBonesName(std::shared_ptr<GObject> object)
    {
        TransformComponent* transform_component = object->tryGetComponent<TransformComponent>("TransformComponent");
        AnimationComponent* animation_component = object->tryGetComponent<AnimationComponent>("AnimationComponent");
        DebugDrawGroup* debug_draw_group = g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("bone name");
        if (animation_component && transform_component)
        {
            Matrix4x4 objMat = Transform(transform_component->getPosition(), transform_component->getRotation(), transform_component->getScale()).getMatrix();
            const Skeleton& skeleton = animation_component->getSkeleton();
            const Bone* bones = skeleton.getBones();
            int32_t bones_count = skeleton.getBonesCount();
            for (int32_t i = 0; i < bones_count; i++)
            {
                if (bones[i].getParent() && i != 1)
                {
                    Matrix4x4 boneMat = Transform(bones[i]._getDerivedPosition(), bones[i]._getDerivedOrientation(), bones[i]._getDerivedScale()).getMatrix();
                    Vector4 pos(0.0f, 0.0f, 0.0f, 1.0f);
                    pos = objMat * boneMat * pos;
                    pos /= pos[3];

                    debug_draw_group->addText(bones[i].getName(), Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector3(pos.x, pos.y, pos.z), 8, false);
                }
            }
        }
    }

    void LevelDebugger::drawBoundingBox(std::shared_ptr<GObject> object)
    {
        RigidBodyComponent* rigidbody_component = object->tryGetComponent<RigidBodyComponent>("RigidBodyComponent");
        if (rigidbody_component)
        {
            const PhysicsActor* physics_actor = rigidbody_component->getPhysicsActor();
            const std::vector<RigidBodyShape>& shapes = physics_actor->getShapes();
            for (size_t i = 0; i < shapes.size(); i++)
            {
                AxisAlignedBox box = shapes[i].m_bounding_box;
                DebugDrawGroup* drawGroup = g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("binding box");
                Vector3 center = Vector3(box.getCenter().x, box.getCenter().y, box.getCenter().z);
                Vector3 halfExtent = Vector3(box.getHalfExtent().x, box.getHalfExtent().y, box.getHalfExtent().z);

                drawGroup->addBox(center, halfExtent, Vector4(1.0f, 0.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
            }
        }
    }

    void LevelDebugger::drawCameraInfo(std::shared_ptr<GObject> object)
    {
        CameraComponent* camera_component = object->tryGetComponent<CameraComponent>("CameraComponent");
        if (camera_component)
        {
            DebugDrawGroup* drawGroup = g_runtime_global_context.m_debugdraw_manager->tryGetOrCreateDebugDrawGroup("show camera info");
            std::ostringstream buffer;
            switch (camera_component->getCameraMode())
            {
            case CameraMode::first_person:
                buffer << "camera mod: first person" << "\n";
                break;
            case CameraMode::third_person:
                buffer << "camera mod: third person" << "\n";
                break;
            case CameraMode::free:
                buffer << "camera mod: free" << "\n";
                break;
            case CameraMode::invalid:
                buffer << "camera mod: invalid" << "\n";
                break;
            }
            Vector3 position = camera_component->getPosition();
            Vector3 forward = camera_component->getForward();
            Vector3 direction = forward - position;
            buffer << "camera position: (" << position.x << "," << position.y << "," << position.z << ")" << "\n";
            buffer << "camera direction : (" << direction.x << "," << direction.y << "," << direction.z << ")";
            drawGroup->addText(buffer.str(), Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector3(-1.0f, -0.2f, 0.0f), 10, true);
        }
    }
}