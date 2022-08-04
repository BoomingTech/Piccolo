#pragma once

#include "runtime/core/math/matrix3.h"
#include "runtime/core/math/transform.h"
#include "runtime/core/math/vector3.h"

#include "runtime/resource/res_type/components/rigid_body.h"

#include <vector>

namespace Piccolo
{
    class GObject;
    class PhysicsActor
    {
    public:
        PhysicsActor(std::weak_ptr<GObject> gobject, const Transform& global_transform);
        ~PhysicsActor();

        void createShapes(const std::vector<RigidBodyShape>& shape_defs, const Transform& global_transform);

        Vector3 getLinearVelocity() const;
        void    setLinearVelocity(const Vector3& velocity);

        Vector3 getAngularVelocity() const;
        void    setAngularVelocity(const Vector3& velocity);

        Vector3 getTorque() const;
        void    addTorque(const Vector3& torque);

        Vector3 getForce() const;
        void    addForce(const Vector3& force);
        void    addForceAtPosition(const Vector3& force, const Vector3& position);
        void    clearForces();

        void  setInverseMass(float inverse_mass);
        float getInverseMass() const;

        void applyLinearImpulse(const Vector3& force);
        void applyAngularImpulse(const Vector3& force);

        void initCubeInertia();
        void initSphereInertia();

        void      updateInertiaTensor();
        Matrix3x3 getInertiaTensor() const;

        const std::vector<RigidBodyShape>& getShapes() const { return m_rigidbody_shapes; }
        Transform&                         getTransform() { return m_global_transform; }

        std::weak_ptr<GObject> getParentGO() const { return m_parent_object; }

        void setActorType(int type) { m_actor_type = type; }
        int  getActorType() const { return m_actor_type; }

        void setGlobalTransform(const Transform& global_transform);

        void setBodyID(uint32_t body_id) { m_body_id = body_id; }
        uint32_t getBodyID() const { return m_body_id; }

    protected:
        std::vector<RigidBodyShape> m_rigidbody_shapes;

        std::weak_ptr<GObject> m_parent_object;

        Transform m_global_transform;

        float m_inverse_mass {0.f};
        float m_friction {0.f};

        Vector3 m_linear_velocity;
        Vector3 m_force;

        Vector3 m_angular_velocity;
        Vector3 m_torque;

        Vector3   m_inverse_inertia;
        Matrix3x3 m_inverse_inertia_tensor;

        int m_actor_type {0};

        uint32_t m_body_id{0xffffffff};
    };
}; // namespace Piccolo
