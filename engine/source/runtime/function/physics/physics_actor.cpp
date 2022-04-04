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

    PhysicsActor::~PhysicsActor()
    {
        for (RigidBodyShapeBase* shape : m_rigidbody_shapes)
        {
            if (shape)
            {
                delete shape;
            }
        }
        m_rigidbody_shapes.clear();
    }

    void PhysicsActor::createShapes(const std::vector<Reflection::ReflectionPtr<RigidBodyShapeBase>>& shape_defs)
    {
        for (auto& shape_def : shape_defs)
        {
            const std::string shape_type = shape_def.getTypeName();
            if (shape_type.compare("RigidBodyBoxShape") == 0)
            {
                RigidBodyBoxShape* box_shape_def = new RigidBodyBoxShape;

                (*box_shape_def) = *static_cast<RigidBodyBoxShape*>(shape_def.operator->());

                const TransformComponent* transform_component =
                    m_parent_object->tryGetComponentConst(TransformComponent);

                // contribute world transform
                Matrix4x4 global_transform_matrix =
                    transform_component->getTransformConst().getMatrix() * box_shape_def->m_local_transform.getMatrix();
                global_transform_matrix.decomposition(box_shape_def->m_global_transform.m_position,
                                                      box_shape_def->m_global_transform.m_scale,
                                                      box_shape_def->m_global_transform.m_rotation);

                // contribute local transform
                Matrix4x4 parent_scale_matrix    = Matrix4x4::getScale(transform_component->getScale());
                Matrix4x4 local_transform_matrix = parent_scale_matrix * box_shape_def->m_local_transform.getMatrix();
                local_transform_matrix.decomposition(box_shape_def->m_local_transform.m_position,
                                                     box_shape_def->m_local_transform.m_scale,
                                                     box_shape_def->m_local_transform.m_rotation);

                // clear global scale
                box_shape_def->m_global_transform.m_scale = Vector3::UNIT_SCALE;

                // calculate half extents
                box_shape_def->m_half_extents =
                    Matrix4x4::getScale(transform_component->getScale()) * box_shape_def->m_half_extents;
                box_shape_def->m_half_extents = transform_component->getRotation() * box_shape_def->m_half_extents;

                RigidBodyShapeBase* shape = static_cast<RigidBodyShapeBase*>(box_shape_def);
                shape->m_type             = RigidBodyShapeType::box;
                m_rigidbody_shapes.push_back(shape);
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
} // namespace Pilot