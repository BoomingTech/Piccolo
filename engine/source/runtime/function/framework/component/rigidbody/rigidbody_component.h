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
        META(Enable)
        RigidBodyComponentRes m_rigidbody_res;

        PhysicsActor* m_physics_actor {nullptr};

        RigidBodyComponent() {}
        RigidBodyComponent(const RigidBodyComponentRes& rigidbody_res, GObject* parent_object);
        ~RigidBodyComponent() override;

        void tick(float delta_time) override {}
        void updateGlobalTransform(const Transform& transform);
    };
} // namespace Pilot
