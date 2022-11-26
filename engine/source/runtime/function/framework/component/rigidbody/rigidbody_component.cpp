#include "runtime/function/framework/component/rigidbody/rigidbody_component.h"

#include "runtime/engine.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/physics/physics_scene.h"

namespace Piccolo
{
    void RigidBodyComponent::postLoadResource(std::weak_ptr<GObject> parent_object)
    {
        m_parent_object = parent_object;

        const TransformComponent* parent_transform = m_parent_object.lock()->tryGetComponentConst(TransformComponent);
        if (parent_transform == nullptr)
        {
            LOG_ERROR("No transform component in the object");
            return;
        }

        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        m_rigidbody_id = physics_scene->createRigidBody(parent_transform->getTransformConst(), m_rigidbody_res);
    }

    RigidBodyComponent::~RigidBodyComponent()
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        physics_scene->removeRigidBody(m_rigidbody_id);
    }

    void RigidBodyComponent::createRigidBody(const Transform& global_transform)
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        m_rigidbody_id = physics_scene->createRigidBody(global_transform, m_rigidbody_res);
    }

    void RigidBodyComponent::removeRigidBody()
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        physics_scene->removeRigidBody(m_rigidbody_id);
    }

    void RigidBodyComponent::updateGlobalTransform(const Transform& transform, bool is_scale_dirty)
    {
        if (is_scale_dirty)
        {
            removeRigidBody();

            createRigidBody(transform);
        }
        else
        {
            std::shared_ptr<PhysicsScene> physics_scene =
                g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
            ASSERT(physics_scene);

            physics_scene->updateRigidBodyGlobalTransform(m_rigidbody_id, transform);
        }
    }

    void RigidBodyComponent::getShapeBoundingBoxes(std::vector<AxisAlignedBox>& out_bounding_boxes) const
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        physics_scene->getShapeBoundingBoxes(m_rigidbody_id, out_bounding_boxes);
    }

} // namespace Piccolo
