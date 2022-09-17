#include "runtime/function/framework/component/rigidbody/rigidbody_component.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/physics/physics_scene.h"
#include "runtime/function/physics/physics_system.h"

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

        m_physics_actor = g_runtime_global_context.m_legacy_physics_system->createPhysicsActor(
            parent_object, parent_transform->getTransformConst(), m_rigidbody_res);

        createRigidBody(parent_transform->getTransformConst());
    }

    RigidBodyComponent::~RigidBodyComponent()
    {
        if (m_physics_actor)
        {
            removeRigidBody();

            g_runtime_global_context.m_legacy_physics_system->removePhyicsActor(m_physics_actor);
            m_physics_actor = nullptr;
        }
    }

    void RigidBodyComponent::createRigidBody(const Transform& global_transform)
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        const uint32_t body_id = physics_scene->createRigidBody(global_transform, m_rigidbody_res);
        m_physics_actor->setBodyID(body_id);
    }

    void RigidBodyComponent::removeRigidBody()
    {
        const uint32_t body_id = m_physics_actor->getBodyID();

        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        physics_scene->removeRigidBody(body_id);
    }

    void RigidBodyComponent::updateGlobalTransform(const Transform& transform, bool is_scale_dirty)
    {
        m_physics_actor->setGlobalTransform(transform);

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

            physics_scene->updateRigidBodyGlobalTransform(m_physics_actor->getBodyID(), transform);
        }
    }

} // namespace Piccolo
