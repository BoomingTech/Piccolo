#include "runtime/function/framework/component/rigidbody/rigidbody_component.h"

#include "runtime/core/base/public_singleton.h"

#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/physics/physics_system.h"

namespace Pilot
{
    RigidBodyComponent::RigidBodyComponent(const RigidBodyActorRes& rigidbody_ast, GObject* parent_object) :
        Component(parent_object)
    {
        const TransformComponent* parent_transform = m_parent_object->tryGetComponentConst(TransformComponent);

        m_physics_actor = PhysicsSystem::getInstance().createPhysicsActor(
            parent_object, parent_transform->getTransformConst(), rigidbody_ast);
    }

    RigidBodyComponent::~RigidBodyComponent()
    {
        if (m_physics_actor)
        {
            PhysicsSystem::getInstance().removePhyicsActor(m_physics_actor);
            m_physics_actor = nullptr;
        }
    }

    void RigidBodyComponent::updateGlobalTransform(const Transform& transform)
    {
        m_physics_actor->setGlobalTransform(transform);
    }

} // namespace Pilot
