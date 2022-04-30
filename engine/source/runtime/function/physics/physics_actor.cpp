#include "runtime/function/physics/physics_actor.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"

namespace Pilot
{
    PhysicsActor::PhysicsActor(GObject* gobject, const Transform& global_transform) :
        m_parent_object {gobject}, m_global_transform {global_transform}
    {
        m_friction = 0.8f;
    }

    PhysicsActor::~PhysicsActor() { m_rigidbody_shapes.clear(); }

    void PhysicsActor::createShapes(const std::vector<RigidBodyShape>& shape_defs, const Transform& global_transform)
    {
        m_rigidbody_shapes = shape_defs;
        for (RigidBodyShape& shape : m_rigidbody_shapes)
        {
            const std::string shape_type = shape.m_geometry.getTypeName();
            if (shape_type == "Box")
            {
                Box* box_shape_geom = new Box;

                (*box_shape_geom) = *static_cast<Box*>(shape.m_geometry.getPtr());

                shape.m_geometry.getPtrReference() = box_shape_geom;
                shape.m_type                       = RigidBodyShapeType::box;
            }
            else
            {
                LOG_ERROR("Unsupported shape type: " + shape_type);
                continue;
            }
        }
    }

    Vector3 PhysicsActor::getLinearVelocity() const { return m_linear_velocity; }

    void PhysicsActor::setLinearVelocity(const Vector3& velocity) { m_linear_velocity = velocity; }

    Vector3 PhysicsActor::getAngularVelocity() const { return m_angular_velocity; }

    void PhysicsActor::setAngularVelocity(const Vector3& velocity) { m_angular_velocity = velocity; }

    Vector3 PhysicsActor::getTorque() const { return m_torque; }

    void PhysicsActor::addTorque(const Vector3& torque) { m_torque += torque; }

    Vector3 PhysicsActor::getForce() const { return m_force; }

    void PhysicsActor::addForce(const Vector3& force) { m_force += force; }

    void PhysicsActor::addForceAtPosition(const Vector3& force, const Vector3& position) {}

    void PhysicsActor::clearForces()
    {
        m_force  = Vector3::ZERO;
        m_torque = Vector3::ZERO;
    }

    void PhysicsActor::setInverseMass(float inverse_mass) { m_inverse_mass = inverse_mass; }

    float PhysicsActor::getInverseMass() const { return m_inverse_mass; }

    void PhysicsActor::applyLinearImpulse(const Vector3& force) { m_linear_velocity += force * m_inverse_mass; }

    void PhysicsActor::applyAngularImpulse(const Vector3& force)
    {
        m_angular_velocity += force * m_inverse_inertia_tensor;
    }

    void PhysicsActor::initCubeInertia()
    {
        Vector3 dimensions       = m_global_transform.m_scale * 2;
        Vector3 square_dimension = dimensions * dimensions;

        m_inverse_inertia.x = (12.0f * m_inverse_mass) / (square_dimension.y + square_dimension.z);
        m_inverse_inertia.y = (12.0f * m_inverse_mass) / (square_dimension.x + square_dimension.z);
        m_inverse_inertia.z = (12.0f * m_inverse_mass) / (square_dimension.x + square_dimension.y);
    }

    void PhysicsActor::initSphereInertia()
    {
        float radius  = Vector3::getMaxElement(m_global_transform.m_scale);
        float element = 2.5f * m_inverse_mass / (radius * radius);

        m_inverse_inertia = Vector3(element, element, element);
    }

    void PhysicsActor::updateInertiaTensor()
    {
        Quaternion q = m_global_transform.m_rotation;

        Matrix3x3 inverse_orientation = Matrix3x3(q.conjugate());
        Matrix3x3 orientation         = Matrix3x3(q);

        m_inverse_inertia_tensor = orientation * Matrix3x3::scale(m_inverse_inertia) * inverse_orientation;
    }

    Matrix3x3 PhysicsActor::getInertiaTensor() const { return m_inverse_inertia_tensor; }

    void PhysicsActor::setGlobalTransform(const Transform& global_transform)
    {
        for (RigidBodyShape& shape : m_rigidbody_shapes)
        {
            if (shape.m_type == RigidBodyShapeType::box)
            {
                Matrix4x4 global_transform_matrix = global_transform.getMatrix() * shape.m_local_transform.getMatrix();
                global_transform_matrix.decomposition(shape.m_global_transform.m_position,
                                                      shape.m_global_transform.m_scale,
                                                      shape.m_global_transform.m_rotation);

                Box* box = static_cast<Box*>(shape.m_geometry);

                AxisAlignedBox bounding;
                for (unsigned char i = 0; i < 2; ++i)
                {
                    for (unsigned char j = 0; j < 2; ++j)
                    {
                        for (unsigned char k = 0; k < 2; ++k)
                        {
                            Vector3 point = box->m_half_extents;
                            if (i == 0)
                                point.x = -point.x;
                            if (j == 0)
                                point.y = -point.y;
                            if (k == 0)
                                point.z = -point.z;

                            point = shape.m_global_transform.getMatrix() * point;

                            bounding.merge(point);
                        }
                    }
                }

                shape.m_bounding_box.update(bounding.getCenter(), bounding.getHalfExtent());
            }
        }
    }

} // namespace Pilot