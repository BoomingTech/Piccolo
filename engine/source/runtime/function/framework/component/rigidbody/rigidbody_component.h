#pragma once

#include "runtime/resource/res_type/components/rigid_body.h"

#include "runtime/function/framework/component/component.h"
#include "runtime/function/physics/physics_actor.h"

namespace Pilot
{
    REFLECTION_TYPE(RigidBodyComponent)
    CLASS(RigidBodyComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(RigidBodyComponent)
    public:
        PhysicsActor* m_physics_actor;

        RigidBodyComponent() {}
        RigidBodyComponent(const RigidBodyActorRes& rigidbody_ast, GObject* parent_object);
        ~RigidBodyComponent();

        void tick(float delta_time) override {}
        void destroy() override {}
    };
} // namespace Pilot
