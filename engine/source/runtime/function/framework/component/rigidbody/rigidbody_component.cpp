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

        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        const uint32_t body_id = physics_scene->createRigidBody(parent_transform->getTransformConst(), m_rigidbody_res);
        m_physics_actor->setBodyID(body_id);
    }

    RigidBodyComponent::~RigidBodyComponent()
    {
        if (m_physics_actor)
        {
            const uint32_t body_id = m_physics_actor->getBodyID();

            std::shared_ptr<PhysicsScene> physics_scene =
                g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
            ASSERT(physics_scene);

            physics_scene->removeRigidBody(body_id);

            g_runtime_global_context.m_legacy_physics_system->removePhyicsActor(m_physics_actor);
            m_physics_actor = nullptr;
        }
    }

    void RigidBodyComponent::updateGlobalTransform(const Transform& transform)
    {
        m_physics_actor->setGlobalTransform(transform);

        // these code intended to fix transform of rigid bodies, but 
        // in JoltPhysics it removes local transform of shapes...
        // so currently rigid bodies cannot be transformed
        //std::shared_ptr<PhysicsScene> physics_scene =
        //    g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        //ASSERT(physics_scene);

        //physics_scene->updateRigidBodyGlobalTransform(m_physics_actor->getBodyID(), transform);
    }

} // namespace Piccolo
