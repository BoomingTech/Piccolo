#include "runtime/function/physics/collision_detection.h"

namespace Piccolo
{
    CollisionDetection::CollisionDetection() {}

    CollisionDetection::~CollisionDetection() {}

    bool CollisionDetection::IsAABBOverlapped(const AxisAlignedBox& aabb_a, const AxisAlignedBox& aabb_b)
    {
        Vector3 delta      = (aabb_a.getCenter() - aabb_b.getCenter()).absoluteCopy();
        Vector3 total_size = aabb_a.getHalfExtent() + aabb_b.getHalfExtent();

        return delta.x < total_size.x && delta.y < total_size.y && delta.z < total_size.z;
    }

    bool CollisionDetection::ObjectIntersection(PhysicsActor&  actor_a,
                                                PhysicsActor&  actor_b,
                                                unsigned int   id_a,
                                                unsigned int   id_b,
                                                CollisionInfo& collision_info)
    {
        bool collision_status = false;

        const std::vector<RigidBodyShape>& actor_a_shapes = actor_a.getShapes();
        const std::vector<RigidBodyShape>& actor_b_shapes = actor_b.getShapes();

        for (const auto& actor_a_shape_iter : actor_a_shapes)
        {
            for (const auto& actor_b_shape_iter : actor_b_shapes)
            {
                if (actor_a_shape_iter.m_type == RigidBodyShapeType::invalid ||
                    actor_b_shape_iter.m_type == RigidBodyShapeType::invalid)
                {
                    return false;
                }

                collision_info.m_id_a = id_a;
                collision_info.m_id_b = id_b;

                Matrix4x4 shape_mat_a =
                    actor_a.getTransform().getMatrix() * actor_a_shape_iter.m_local_transform.getMatrix();
                Vector3    shape_a_position;
                Quaternion shape_a_rotation;
                Vector3    shape_a_scale;
                shape_mat_a.decomposition(shape_a_position, shape_a_scale, shape_a_rotation);
                Transform shape_transform_a(shape_a_position, shape_a_rotation, shape_a_scale);

                Matrix4x4 shape_mat_b =
                    actor_b.getTransform().getMatrix() * actor_b_shape_iter.m_local_transform.getMatrix();
                Vector3    shape_b_position;
                Quaternion shape_b_rotation;
                Vector3    shape_b_scale;
                shape_mat_b.decomposition(shape_b_position, shape_b_scale, shape_b_rotation);
                Transform shape_transform_b(shape_b_position, shape_b_rotation, shape_b_scale);

                if (actor_a_shape_iter.m_type == RigidBodyShapeType::box &&
                    actor_b_shape_iter.m_type == RigidBodyShapeType::box)
                {
                    collision_status |=
                        AABBIntersection(static_cast<const Box*>(actor_a_shape_iter.m_geometry)->m_half_extents,
                                         static_cast<const Box*>(actor_b_shape_iter.m_geometry)->m_half_extents,
                                         shape_transform_a,
                                         shape_transform_b,
                                         collision_info);
                }

                if (actor_a_shape_iter.m_type == RigidBodyShapeType::sphere &&
                    actor_b_shape_iter.m_type == RigidBodyShapeType::sphere)
                {
                    collision_status |=
                        SphereIntersection(static_cast<const Sphere*>(actor_a_shape_iter.m_geometry)->m_radius,
                                           static_cast<const Sphere*>(actor_b_shape_iter.m_geometry)->m_radius,
                                           shape_transform_a,
                                           shape_transform_b,
                                           collision_info);
                }

                if (actor_a_shape_iter.m_type == RigidBodyShapeType::box &&
                    actor_b_shape_iter.m_type == RigidBodyShapeType::sphere)
                {
                    collision_status |=
                        AABBSphereIntersection(static_cast<const Box*>(actor_a_shape_iter.m_geometry)->m_half_extents,
                                               static_cast<const Sphere*>(actor_b_shape_iter.m_geometry)->m_radius,
                                               shape_transform_a,
                                               shape_transform_b,
                                               collision_info);
                }

                if (actor_a_shape_iter.m_type == RigidBodyShapeType::sphere &&
                    actor_b_shape_iter.m_type == RigidBodyShapeType::box)
                {
                    collision_info.m_id_a = id_b;
                    collision_info.m_id_b = id_a;
                    collision_status |=
                        AABBSphereIntersection(static_cast<const Box*>(actor_b_shape_iter.m_geometry)->m_half_extents,
                                               static_cast<const Sphere*>(actor_a_shape_iter.m_geometry)->m_radius,
                                               shape_transform_b,
                                               shape_transform_a,
                                               collision_info);
                }
            }
        }
        return collision_status;
    }

    Vector3
    CollisionDetection::getClosestPoint(const Vector3& box_size, const Transform& obb_transform, const Vector3& point)
    {
        Vector3 result = obb_transform.m_position;
        Vector3 dir    = point - result;

        float* p_box_size = (float*)&box_size;

        for (int i = 0; i < 3; ++i)
        {
            Matrix3x3 orientation   = Matrix3x3(obb_transform.m_rotation);
            float*    p_orientation = (float*)&orientation;
            Vector3   axis {p_orientation[0], p_orientation[1], p_orientation[2]};

            float distance = dir.dotProduct(axis);

            if (distance > p_box_size[i])
            {
                distance = p_box_size[i];
            }
            if (distance < -p_box_size[i])
            {
                distance = -p_box_size[i];
            }

            result += (axis * distance);
        }

        return result;
    }

    bool CollisionDetection::AABBIntersection(const Vector3&   box_a_size,
                                              const Vector3&   box_b_size,
                                              const Transform& world_transform_a,
                                              const Transform& world_transform_b,
                                              CollisionInfo&   collision_info)
    {
        AxisAlignedBox bounding_a(world_transform_a.m_position, box_a_size);
        AxisAlignedBox bounding_b(world_transform_b.m_position, box_b_size);

        if (IsAABBOverlapped(bounding_a, bounding_b))
        {
            static const Vector3 faces[6] = {
                Vector3 {-1, 0, 0},
                Vector3 {1, 0, 0},
                Vector3 {0, -1, 0},
                Vector3 {0, 1, 0},
                Vector3 {0, 0, -1},
                Vector3 {0, 0, 1},
            };

            Vector3 max_a = bounding_a.getMaxCorner();
            Vector3 min_a = bounding_a.getMinCorner();
            Vector3 max_b = bounding_b.getMaxCorner();
            Vector3 min_b = bounding_b.getMinCorner();

            float distances[6] = {
                (max_b.x - min_a.x),
                (max_a.x - min_b.x),
                (max_b.y - min_a.y),
                (max_a.y - min_b.y),
                (max_b.z - min_a.z),
                (max_a.z - min_b.z),
            };
            float   penetration = FLT_MAX;
            Vector3 best_axis;

            for (int i = 0; i < 6; i++)
            {
                if (distances[i] < penetration)
                {
                    penetration = distances[i];
                    best_axis   = faces[i];
                }
            }
            collision_info.addContactPoint(Vector3(), Vector3(), best_axis, penetration);

            return true;
        }

        return false;
    }

    bool CollisionDetection::SphereIntersection(float            sphere_a_radius,
                                                float            sphere_b_radius,
                                                const Transform& world_transform_a,
                                                const Transform& world_transform_b,
                                                CollisionInfo&   collision_info)
    {
        float   radii  = sphere_a_radius + sphere_b_radius;
        Vector3 normal = world_transform_b.m_position - world_transform_a.m_position;

        float length = normal.length();

        if (length < radii)
        {
            float penetration = (radii - length);
            normal.normalise();
            Vector3 localA = normal * sphere_a_radius;
            Vector3 localB = -normal * sphere_b_radius;

            collision_info.addContactPoint(localA, localB, normal, penetration);
            return true;
        }

        return false;
    }

    bool CollisionDetection::AABBSphereIntersection(const Vector3&   box_size,
                                                    float            sphere_radius,
                                                    const Transform& world_transform_a,
                                                    const Transform& world_transform_b,
                                                    CollisionInfo&   collision_info)
    {
        Vector3 delta = world_transform_b.m_position - world_transform_a.m_position;

        Vector3 closest_point_on_box = Vector3::clamp(delta, -box_size, box_size);

        Vector3 local_point = delta - closest_point_on_box;
        float   distance    = local_point.length();

        if (distance < sphere_radius)
        {
            Vector3 collision_normal = local_point.normalisedCopy();
            float   penetration      = (sphere_radius - distance);

            Vector3 localA = Vector3();
            Vector3 localB = -collision_normal * sphere_radius;

            collision_info.addContactPoint(localA, localB, collision_normal, penetration);

            return true;
        }
        return false;
    }

    bool CollisionDetection::OBBIntersection(const Vector3&   box_a_size,
                                             const Vector3&   box_b_size,
                                             const Transform& world_transform_a,
                                             const Transform& world_transform_b,
                                             CollisionInfo&   collision_info)
    {
        Vector3 v = world_transform_a.m_position - world_transform_b.m_position;

        // compute a's basis
        std::vector<Vector3> volume_a_axis;
        volume_a_axis.push_back(world_transform_a.m_rotation * Vector3 {1, 0, 0}); // a_x
        volume_a_axis.push_back(world_transform_a.m_rotation * Vector3 {0, 1, 0}); // a_y
        volume_a_axis.push_back(world_transform_a.m_rotation * Vector3 {0, 0, 1}); // a_z

        // compute b's basis
        std::vector<Vector3> volume_b_axis;
        volume_b_axis.push_back(world_transform_b.m_rotation * Vector3 {1, 0, 0}); // b_x
        volume_b_axis.push_back(world_transform_b.m_rotation * Vector3 {0, 1, 0}); // b_y
        volume_b_axis.push_back(world_transform_b.m_rotation * Vector3 {0, 0, 1}); // b_z

        Vector3 T {v.dotProduct(volume_a_axis[0]), v.dotProduct(volume_a_axis[1]), v.dotProduct(volume_a_axis[2])};

        Matrix3x3 R, RF;
        float     ra, rb, t;

        float* p_r  = (float*)&R;
        float* p_rf = (float*)&RF;
        float* p_t  = (float*)&T;

        float* half_dimensions_a = (float*)&box_a_size;
        float* half_dimensions_b = (float*)&box_b_size;

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                p_r[i * 3 + j]  = volume_a_axis[i].dotProduct(volume_b_axis[j]);
                p_rf[i * 3 + j] = 1e-6f + fabsf(p_r[i * 3 + j]);
            }
        }

        // a's basis vectors
        for (int i = 0; i < 3; ++i)
        {
            ra = half_dimensions_a[i];
            rb = half_dimensions_b[0] * p_rf[i * 3 + 0] + half_dimensions_b[1] * p_rf[i * 3 + 1] +
                 half_dimensions_b[2] * p_rf[i * 3 + 2];
            t = fabsf(p_t[i]);

            if (t > ra + rb)
            {
                return false;
            }
        }

        // b's basis vectors
        for (int i = 0; i < 3; ++i)
        {
            ra = half_dimensions_a[0] * p_rf[0 * 3 + i] + half_dimensions_a[1] * p_rf[1 * 3 + i] +
                 half_dimensions_a[2] * p_rf[2 * 3 + i];
            rb = half_dimensions_b[i];
            t  = fabsf(p_t[0] * p_r[0 * 3 + i] + p_t[1] * p_r[1 * 3 + i] + p_t[2] * p_r[2 * 3 + i]);

            if (t > ra + rb)
            {
                return false;
            }
        }

        // 9 cross products
        // L = A0 * B0
        ra = half_dimensions_a[1] * p_rf[2 * 3 + 0] + half_dimensions_a[2] * p_rf[1 * 3 + 0];
        rb = half_dimensions_b[1] * p_rf[0 * 3 + 2] + half_dimensions_b[2] * p_rf[0 * 3 + 1];
        t  = fabsf(p_t[2] * p_r[1 * 3 + 0] - p_t[1] * p_r[2 * 3 + 0]);
        if (t > ra + rb)
            return false;

        // L = A0 * B1
        ra = half_dimensions_a[1] * p_rf[2 * 3 + 1] + half_dimensions_a[2] * p_rf[1 * 3 + 1];
        rb = half_dimensions_b[0] * p_rf[0 * 3 + 2] + half_dimensions_b[2] * p_rf[0 * 3 + 0];
        t  = fabsf(p_t[2] * p_r[1 * 3 + 1] - p_t[1] * p_r[2 * 3 + 1]);
        if (t > ra + rb)
            return false;

        // L = A0 * B2
        ra = half_dimensions_a[1] * p_rf[2 * 3 + 2] + half_dimensions_a[2] * p_rf[1 * 3 + 2];
        rb = half_dimensions_b[0] * p_rf[0 * 3 + 1] + half_dimensions_b[1] * p_rf[0 * 3 + 0];
        t  = fabsf(p_t[2] * p_r[1 * 3 + 2] - p_t[1] * p_r[2 * 3 + 2]);
        if (t > ra + rb)
            return false;

        // L = A1 * B0
        ra = half_dimensions_a[0] * p_rf[2 * 3 + 0] + half_dimensions_a[2] * p_rf[0 * 3 + 0];
        rb = half_dimensions_b[1] * p_rf[1 * 3 + 2] + half_dimensions_b[2] * p_rf[1 * 3 + 1];
        t  = fabsf(p_t[0] * p_r[2 * 3 + 0] - p_t[2] * p_r[0 * 3 + 0]);
        if (t > ra + rb)
            return false;

        // L = A1 * B1
        ra = half_dimensions_a[0] * p_rf[2 * 3 + 1] + half_dimensions_a[2] * p_rf[0 * 3 + 1];
        rb = half_dimensions_b[0] * p_rf[1 * 3 + 2] + half_dimensions_b[2] * p_rf[1 * 3 + 0];
        t  = fabsf(p_t[0] * p_r[2 * 3 + 1] - p_t[2] * p_r[0 * 3 + 1]);
        if (t > ra + rb)
            return false;

        // L = A1 * B2
        ra = half_dimensions_a[0] * p_rf[2 * 3 + 2] + half_dimensions_a[2] * p_rf[0 * 3 + 2];
        rb = half_dimensions_b[0] * p_rf[1 * 3 + 1] + half_dimensions_b[1] * p_rf[1 * 3 + 0];
        t  = fabsf(p_t[0] * p_r[2 * 3 + 2] - p_t[2] * p_r[0 * 3 + 2]);
        if (t > ra + rb)
            return false;

        // L = A2 * B0
        ra = half_dimensions_a[0] * p_rf[1 * 3 + 0] + half_dimensions_a[1] * p_rf[0 * 3 + 0];
        rb = half_dimensions_b[1] * p_rf[2 * 3 + 2] + half_dimensions_b[2] * p_rf[2 * 3 + 1];
        t  = fabsf(p_t[1] * p_r[0 * 3 + 0] - p_t[0] * p_r[1 * 3 + 0]);
        if (t > ra + rb)
            return false;

        // L = A2 * B1
        ra = half_dimensions_a[0] * p_rf[1 * 3 + 1] + half_dimensions_a[1] * p_rf[0 * 3 + 1];
        rb = half_dimensions_b[0] * p_rf[2 * 3 + 2] + half_dimensions_b[2] * p_rf[2 * 3 + 0];
        t  = fabsf(p_t[1] * p_r[0 * 3 + 1] - p_t[0] * p_r[1 * 3 + 1]);
        if (t > ra + rb)
            return false;

        // L = A2 * B2
        ra = half_dimensions_a[0] * p_rf[1 * 3 + 2] + half_dimensions_a[1] * p_rf[0 * 3 + 2];
        rb = half_dimensions_b[0] * p_rf[2 * 3 + 1] + half_dimensions_b[1] * p_rf[2 * 3 + 0];
        t  = fabsf(p_t[1] * p_r[0 * 3 + 2] - p_t[0] * p_r[1 * 3 + 2]);
        if (t > ra + rb)
            return false;

        return true;
    }

    bool CollisionDetection::OBBSphereIntersection(const Vector3&   box_size,
                                                   float            sphere_radius,
                                                   const Transform& world_transform_a,
                                                   const Transform& world_transform_b,
                                                   CollisionInfo&   collision_info)
    {
        Vector3 box_center    = world_transform_a.m_position;
        Vector3 sphere_center = world_transform_b.m_position;

        Vector3 closest_point = getClosestPoint(box_size, world_transform_a, sphere_center);

        float dis_sq = (closest_point - sphere_center).dotProduct(closest_point - sphere_center);
        if (dis_sq > sphere_radius * sphere_radius)
        {
            return false;
        }

        Vector3 normal;
        if (CMP(dis_sq, 0.0f))
        {
            if (CMP((closest_point - box_center).dotProduct(closest_point - box_center), 0.0f))
            {
                return false;
            }
            normal = (closest_point - box_center).normalisedCopy();
        }
        else
        {
            normal = (sphere_center - closest_point).normalisedCopy();
        }

        Vector3 outside_point = sphere_center - normal * sphere_radius;

        float distance = sqrtf((closest_point - outside_point).dotProduct(closest_point - outside_point));

        collision_info.addContactPoint(
            Vector3(), closest_point + (outside_point - closest_point) * 0.5f, normal, distance * 0.5f);

        return true;
    }

    bool CollisionDetection::RayIntersection(const Ray& r, PhysicsActor& actor_a, RayCollision& collisions)
    {
        bool collided = false;

        return collided;
    }

    bool CollisionDetection::RayBoxIntersection(const Ray&     r,
                                                const Vector3& box_pos,
                                                const Vector3& box_size,
                                                RayCollision&  collision)
    {
        Vector3 box_min = box_pos - box_size;
        Vector3 box_max = box_pos + box_size;

        Vector3 ray_pos = r.getStartPoint();
        Vector3 ray_dir = r.getDirection();

        Vector3 t_vals {-1, -1, -1};

        if (ray_dir.x > 0)
        {
            t_vals.x = (box_min.x - ray_pos.x) / ray_dir.x;
        }
        else if (ray_dir.x < 0)
        {
            t_vals.x = (box_max.x - ray_pos.x) / ray_dir.x;
        }

        if (ray_dir.y > 0)
        {
            t_vals.y = (box_min.y - ray_pos.y) / ray_dir.y;
        }
        else if (ray_dir.y < 0)
        {
            t_vals.y = (box_max.y - ray_pos.y) / ray_dir.y;
        }

        if (ray_dir.z > 0)
        {
            t_vals.z = (box_min.z - ray_pos.z) / ray_dir.z;
        }
        else if (ray_dir.z < 0)
        {
            t_vals.z = (box_max.z - ray_pos.z) / ray_dir.z;
        }

        float best_t = Vector3::getMaxElement(t_vals);

        if (best_t < 0.0f)
        {
            return false;
        }

        Vector3     intersection = ray_pos + (ray_dir * best_t);
        const float epsilon      = 0.0001f;

        if (intersection.x + epsilon < box_min.x || intersection.x - epsilon > box_max.x)
        {
            return false;
        }

        if (intersection.y + epsilon < box_min.y || intersection.y - epsilon > box_max.y)
        {
            return false;
        }

        if (intersection.z + epsilon < box_min.z || intersection.z - epsilon > box_max.z)
        {
            return false;
        }

        collision.m_collided_point = intersection;
        collision.m_ray_distance   = best_t;

        return true;
    }

    bool CollisionDetection::RayAABBIntersection(const Ray&       r,
                                                 const Transform& world_transform,
                                                 const Vector3&   box_size,
                                                 RayCollision&    collision)
    {
        Vector3 box_pos = world_transform.m_position;
        return RayBoxIntersection(r, box_pos, box_size, collision);
    }

    bool CollisionDetection::RayOBBIntersection(const Ray&       r,
                                                const Transform& world_transform,
                                                const Vector3&   box_size,
                                                RayCollision&    collision)
    {
        Quaternion orientation  = world_transform.m_rotation;
        Vector3    obb_position = world_transform.m_position;

        Matrix3x3 transform     = Matrix3x3(orientation);
        Matrix3x3 inv_transform = Matrix3x3(orientation.conjugate());
        Vector3   local_ray_pos = r.getStartPoint() - obb_position;

        Ray temp_ray(inv_transform * local_ray_pos, inv_transform * r.getDirection());

        bool collided = RayBoxIntersection(temp_ray, Vector3(), box_size, collision);

        if (collided)
        {
            collision.m_collided_point = transform * collision.m_collided_point + obb_position;
        }

        return collided;
    }

    bool CollisionDetection::RaySphereIntersection(const Ray&       r,
                                                   const Transform& world_transform,
                                                   float            sphere_radius,
                                                   RayCollision&    collision)
    {
        Vector3 sphere_pos = world_transform.m_position;

        Vector3 dir = (sphere_pos - r.getStartPoint());

        float sphere_proj = dir.dotProduct(r.getDirection());

        if (sphere_proj < 0.0f)
        {
            return false;
        }

        Vector3 point = r.getStartPoint() + (r.getDirection() * sphere_proj);

        float sphere_dist = (point - sphere_pos).length();

        if (sphere_dist > sphere_radius)
        {
            return false;
        }

        float offset = sqrt((sphere_radius * sphere_radius) - (sphere_dist * sphere_dist));

        collision.m_ray_distance   = sphere_proj - offset;
        collision.m_collided_point = r.getStartPoint() + (r.getDirection() * collision.m_ray_distance);

        return true;
    }
} // namespace Piccolo
