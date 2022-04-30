#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/core/math/transform.h"
#include "runtime/function/physics/collision_detection.h"
#include "runtime/function/physics/physics_actor.h"

#include "runtime/resource/res_type/components/rigid_body.h"

#include <set>

namespace Pilot
{
    class PhysicsSystem : public PublicSingleton<PhysicsSystem>
    {
    public:
        void tick(float delta_time);

        void setIsUseGravity(bool is_use_gravity) { m_is_use_gravity = is_use_gravity; }
        void setGravity(const Vector3& g) { m_gravity = g; }
        void setGlobalDamping(float damping) { m_global_damping = damping; }

        PhysicsActor* createPhysicsActor(GObject*                     gobject,
                                         const Transform&             global_transform,
                                         const RigidBodyComponentRes& rigid_body_actor_res);
        void          removePhyicsActor(PhysicsActor* gobject);

        bool raycast(const Vector3& ray_start, const Vector3& ray_direction, Vector3& out_hit_position);
        bool overlapByCapsule(const Vector3& position, const Capsule& capsule);

    protected:
        void collideAndResolve();

        void clearForces();

        void integrateAccelerate(float delta_time);
        void integrateVelocity(float delta_time);

        void updateCollisionList();

        void impulseResolveCollision(PhysicsActor& object_a, PhysicsActor& object_b, ContactPoint& contact_point) const;

        bool overlap(const AxisAlignedBox& query_bouding);

    private:
        std::vector<PhysicsActor*> m_physics_actors;
        std::set<CollisionInfo>    m_all_collisions;

        unsigned int m_num_collision_frames {5};
        float        m_delta_time_offset {0.f};
        float        m_global_damping {0.95f};
        float        m_delta_time {0.f};

        Vector3 m_gravity {0.0f, 0.0f, -9.8f};
        bool    m_is_use_gravity {true};
    };
} // namespace Pilot