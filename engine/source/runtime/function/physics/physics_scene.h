#pragma once

#include "runtime/core/math/axis_aligned.h"

#include "runtime/function/physics/physics_config.h"

namespace JPH
{
    class PhysicsSystem;
    class JobSystem;
    class TempAllocator;
    class BroadPhaseLayerInterface;
#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
    class DebugRenderer;
#endif
} // namespace JPH

namespace Piccolo
{
    class Transform;
    class RigidBodyComponentRes;
    class RigidBodyShape;

    static constexpr uint32_t s_invalid_rigidbody_id = 0xffffffff;

    struct PhysicsHitInfo
    {
        Vector3  hit_position;
        Vector3  hit_normal;
        float    hit_distance {0.f};
        uint32_t body_id {s_invalid_rigidbody_id};
    };

    class PhysicsScene
    {
        struct JoltPhysics
        {
            JPH::PhysicsSystem*            m_jolt_physics_system {nullptr};
            JPH::JobSystem*                m_jolt_job_system {nullptr};
            JPH::TempAllocator*            m_temp_allocator {nullptr};
            JPH::BroadPhaseLayerInterface* m_jolt_broad_phase_layer_interface {nullptr};

            int m_collision_steps {1};
            int m_integration_substeps {1};
        };

    public:
        PhysicsScene(const Vector3& gravity);
        virtual ~PhysicsScene();

        const Vector3& getGravity() const { return m_config.m_gravity; }

        uint32_t createRigidBody(const Transform& global_transform, const RigidBodyComponentRes& rigidbody_actor_res);
        void     removeRigidBody(uint32_t body_id);

        void updateRigidBodyGlobalTransform(uint32_t body_id, const Transform& global_transform);

        void tick(float delta_time);

        /// cast a ray and find the hits
        /// @ray_origin: origin of ray
        /// @ray_direction: ray direction
        /// @ray_length: ray length, anything beyond this length will not be reported as a hit
        /// @out_hits: the found hits, sorted by distance
        /// @return: true if any hits found, else false
        bool
        raycast(Vector3 ray_origin, Vector3 ray_direction, float ray_length, std::vector<PhysicsHitInfo>& out_hits);

        /// cast a shape and find the hits
        /// @shape: the casted rigidbody shape
        /// @shape_transform: the initial global transform of the casted shape
        /// @sweep_direction: sweep direction
        /// @sweep_length: sweep length, anything beyond this length will not be reported as a hit
        /// @out_hits: the found hits, sorted by distance
        /// @return: true if any hits found, else false
        bool sweep(const RigidBodyShape&        shape,
                   const Matrix4x4&             shape_transform,
                   Vector3                      sweep_direction,
                   float                        sweep_length,
                   std::vector<PhysicsHitInfo>& out_hits);

        /// overlap test
        /// @shape: rigidbody shape
        /// @return: true if overlapped with any rigidbodies
        bool isOverlap(const RigidBodyShape& shape, const Matrix4x4& global_transform);

        void getShapeBoundingBoxes(uint32_t body_id, std::vector<AxisAlignedBox>& out_bounding_boxes) const;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        void drawPhysicsScene(JPH::DebugRenderer* debug_renderer);
#endif

    protected:
        // we use single Jolt physics system for each scene
        JoltPhysics m_physics;

        PhysicsConfig m_config;

        std::vector<uint32_t> m_pending_remove_bodies;
    };
} // namespace Piccolo