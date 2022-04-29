#include "runtime/function/physics/physics_system.h"

#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/physics/physics_actor.h"
#include "runtime/function/physics/physics_shape_base.h"

#include <algorithm>

namespace Pilot
{
    void PhysicsSystem::tick(float delta_time)
    {
        m_delta_time = delta_time;

        m_delta_time_offset += m_delta_time;

        float iteration_delta_time = 1.0f / 120.0f;

        // If the physics engine cant catch up, it'll just have to run bigger timesteps.
        if (m_delta_time_offset > 8 * iteration_delta_time)
        {
            iteration_delta_time = 1.0f / 15.0f;
        }
        else if (m_delta_time_offset > 4 * iteration_delta_time)
        {
            iteration_delta_time = 1.0f / 30.0f;
        }
        else if (m_delta_time_offset > 2 * iteration_delta_time)
        {
            iteration_delta_time = 1.0f / 60.0f;
        }

        // int constraint_iteration_count = 10;
        // iteration_delta_time = delta_time;

        while (m_delta_time_offset > iteration_delta_time)
        {
            integrateAccelerate(iteration_delta_time); // Update accelerations from external forces

            collideAndResolve();

            updateCollisionList(); // Remove any old collisions

            integrateVelocity(iteration_delta_time); // update positions from new velocity changes

            m_delta_time_offset -= iteration_delta_time;
        }
        clearForces();
    }

    PhysicsActor* PhysicsSystem::createPhysicsActor(GObject*                     gobject,
                                                    const Transform&             global_transform,
                                                    const RigidBodyComponentRes& rigid_body_actor_res)
    {
        PhysicsActor* actor = new PhysicsActor(gobject, global_transform);

        actor->createShapes(rigid_body_actor_res.m_shapes, global_transform);
        actor->setInverseMass(rigid_body_actor_res.m_inverse_mass);
        actor->setActorType(rigid_body_actor_res.m_actor_type);
        Transform actor_transform(global_transform.m_position, global_transform.m_rotation, Vector3::UNIT_SCALE);

        actor->setGlobalTransform(actor_transform);

        m_physics_actors.push_back(actor);

        return actor;
    }

    void PhysicsSystem::removePhyicsActor(PhysicsActor* actor)
    {
        auto iter = std::find(m_physics_actors.begin(), m_physics_actors.end(), actor);
        if (iter != m_physics_actors.end())
        {
            m_physics_actors.erase(iter);
            delete actor;
        }
    }

    void PhysicsSystem::collideAndResolve()
    {
        CollisionInfo collision_info;
        for (int i = 0; i < m_physics_actors.size(); ++i)
        {
            for (int j = i + 1; j < m_physics_actors.size(); ++j)
            {
                bool intersected = CollisionDetection::ObjectIntersection(
                    *m_physics_actors[i], *m_physics_actors[j], i, j, collision_info);
                if (intersected == false)
                    continue;

                impulseResolveCollision(*m_physics_actors[i], *m_physics_actors[j], collision_info.m_contact_point);
                collision_info.m_frame_left = m_num_collision_frames;

                m_all_collisions.insert(collision_info);
            }
        }
    }

    bool PhysicsSystem::raycast(const Vector3& ray_start, const Vector3& ray_direction, Vector3& out_hit_position)
    {
        bool is_hit = false;

        Ray   ray(ray_start, ray_direction);
        float distance = FLT_MAX;
        for (int i = 0; i < m_physics_actors.size(); ++i)
        {
            RayCollision collision;
            is_hit = CollisionDetection::RayIntersection(ray, *m_physics_actors[i], collision);
            if (is_hit)
            {
                if (collision.m_ray_distance < distance)
                {
                    distance         = collision.m_ray_distance;
                    out_hit_position = collision.m_collided_point;
                }
            }
        }

        return is_hit;
    }

    bool PhysicsSystem::overlapByCapsule(const Vector3& position, const Capsule& capsule)
    {
        // currently only overlap by aabb
        const float    capsule_height = capsule.m_half_height + capsule.m_radius;
        Vector3        center         = position + capsule_height * Vector3::UNIT_Z;
        Vector3        half_extent    = Vector3(capsule.m_radius, capsule.m_radius, capsule_height);
        AxisAlignedBox bounding(center, half_extent);
        return overlap(bounding);
    }

    bool PhysicsSystem::overlap(const AxisAlignedBox& query_bouding)
    {
        for (int i = 0; i < m_physics_actors.size(); i++)
        {
            for (auto& shape : m_physics_actors[i]->getShapes())
            {
                if (shape.m_type == RigidBodyShapeType::invalid)
                    continue;

                bool is_hit = CollisionDetection::IsAABBOverlapped(query_bouding, shape.m_bounding_box);

                if (is_hit)
                    return true;
            }
        }

        return false;
    }

    void PhysicsSystem::clearForces()
    {
        for (auto& object_iter : m_physics_actors)
        {
            object_iter->clearForces();
        }
    }

    void PhysicsSystem::integrateAccelerate(float delta_time)
    {
        for (auto& object_iter : m_physics_actors)
        {
            float inverse_mass = object_iter->getInverseMass();

            Vector3 linear_velocity = object_iter->getLinearVelocity();
            Vector3 force           = object_iter->getForce();
            Vector3 accel           = force * inverse_mass;

            if (m_is_use_gravity && inverse_mass > 0)
            {
                accel = accel + m_gravity;
            }

            linear_velocity = linear_velocity + accel * delta_time;
            object_iter->setLinearVelocity(linear_velocity);

            Vector3 torque           = object_iter->getTorque();
            Vector3 angular_velocity = object_iter->getAngularVelocity();

            object_iter->updateInertiaTensor(); // update tensor vs orientation

            Vector3 angular_accelerate = object_iter->getInertiaTensor() * torque;

            angular_velocity = angular_velocity + angular_accelerate * delta_time; // integrate angular accel
            object_iter->setAngularVelocity(angular_velocity);
        }
    }

    void PhysicsSystem::integrateVelocity(float delta_time)
    {
        float frame_damping = powf(m_global_damping, delta_time);

        for (auto& object_iter : m_physics_actors)
        {
            Transform& transform = object_iter->getTransform();

            Vector3 linear_velocity = object_iter->getLinearVelocity();
            transform.m_position += linear_velocity * delta_time;

            linear_velocity = linear_velocity * frame_damping;
            object_iter->setLinearVelocity(linear_velocity);

            Quaternion& orientation      = transform.m_rotation;
            Vector3     angular_velocity = object_iter->getAngularVelocity();

            Quaternion delta_angular {(angular_velocity * delta_time * 0.5f).x,
                                      (angular_velocity * delta_time * 0.5f).y,
                                      (angular_velocity * delta_time * 0.5f).z,
                                      0.0f};
            orientation = orientation + (delta_angular * orientation);
            orientation.normalise();

            angular_velocity = angular_velocity * frame_damping;
            object_iter->setAngularVelocity(angular_velocity);

            // Vector3 pos = m_objects[i].getTransform()->getLocalPosition();
            // std::cout << "actor id : " << i++ << "   after tick pos: " << pos.x << " " << pos.y << " " << pos.z <<
            // std::endl;
        }
    }

    void PhysicsSystem::updateCollisionList()
    {
        for (auto iter = m_all_collisions.begin(); iter != m_all_collisions.end();)
        {
            if (iter->m_frame_left == m_num_collision_frames)
            {
                // iter->m_actor_a->onCollisionBegin(iter->m_actor_b);
                // iter->m_actor_b->onCollisionEnd(iter->m_actor_a);
                // PMLog("collision between " + iter->m_id_a + iter->m_id_b);
            }

            iter->m_frame_left = iter->m_frame_left - 1;

            if (iter->m_frame_left < 0)
            {
                // iter->m_actor_a->onCollisionBegin(iter->m_actor_b);
                // iter->m_actor_b->onCollisionEnd(iter->m_actor_a);
                // PMLog("collision between " + iter->m_id_a + iter->m_id_b);
                iter = m_all_collisions.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    void PhysicsSystem::impulseResolveCollision(PhysicsActor& actor_a,
                                                PhysicsActor& actor_b,
                                                ContactPoint& contact_point) const
    {
        float total_mass = actor_a.getInverseMass() + actor_b.getInverseMass();

        if (total_mass == 0)
        {
            return;
        }

        Transform& transform_a = actor_a.getTransform();
        Transform& transform_b = actor_b.getTransform();

        transform_a.m_position -=
            contact_point.m_normal * contact_point.m_penetration * (actor_a.getInverseMass() / total_mass);
        transform_b.m_position +=
            contact_point.m_normal * contact_point.m_penetration * (actor_a.getInverseMass() / total_mass);

        Vector3 relative_a = contact_point.m_location_a;
        Vector3 relative_b = contact_point.m_location_b;

        Vector3 ang_velocity_a = actor_a.getAngularVelocity().crossProduct(relative_a);
        Vector3 ang_velocity_b = actor_b.getAngularVelocity().crossProduct(relative_b);

        Vector3 full_velocity_a = actor_a.getLinearVelocity() + ang_velocity_a;
        Vector3 full_velocity_b = actor_b.getLinearVelocity() + ang_velocity_b;

        Vector3 contact_velocity = full_velocity_b - full_velocity_a;

        float impulse_force = contact_velocity.dotProduct(contact_point.m_normal);

        Vector3 inertia_a =
            (actor_a.getInertiaTensor() * relative_a.crossProduct(contact_point.m_normal)).crossProduct(relative_a);
        Vector3 inertia_b =
            (actor_b.getInertiaTensor() * relative_b.crossProduct(contact_point.m_normal)).crossProduct(relative_b);

        float angular_effect = (inertia_a + inertia_b).dotProduct(contact_point.m_normal);

        float restitution = 0; // 0.66f; // disperse some kinectic energy

        float impulse = (-(1.0f + restitution) * impulse_force) / (total_mass + angular_effect);

        Vector3 fullImpulse = contact_point.m_normal * impulse;

        actor_a.applyLinearImpulse(-fullImpulse);
        actor_b.applyLinearImpulse(fullImpulse);

        actor_a.applyAngularImpulse(relative_a.crossProduct(-fullImpulse));
        actor_b.applyAngularImpulse(relative_b.crossProduct(fullImpulse));
    }
} // namespace Pilot