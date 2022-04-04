#pragma once
#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Pilot
{
    enum class RigidBodyShapeType : unsigned char
    {
        box,
        sphere,
        capsule,
        invalid
    };

    REFLECTION_TYPE(RigidBodyShapeBase)
    CLASS(RigidBodyShapeBase, WhiteListFields)
    {
        REFLECTION_BODY(RigidBodyShapeBase);

    public:
        Transform          m_global_transform;
        RigidBodyShapeType m_type {RigidBodyShapeType::invalid};
        META(Enable)
        Transform m_local_transform;
    };

    REFLECTION_TYPE(RigidBodyBoxShape)
    CLASS(RigidBodyBoxShape : public RigidBodyShapeBase, Fields)
    {
        REFLECTION_BODY(RigidBodyBoxShape);

    public:
        Vector3 m_half_extents {0.5f, 0.5f, 0.5f};
    };

    REFLECTION_TYPE(RigidBodySphereShape)
    CLASS(RigidBodySphereShape : public RigidBodyShapeBase, Fields)
    {
        REFLECTION_BODY(RigidBodySphereShape);

    public:
        float m_radius {0.5f};
    };

    REFLECTION_TYPE(RigidBodyCapsuleShape)
    CLASS(RigidBodyCapsuleShape : public RigidBodyShapeBase, Fields)
    {
        REFLECTION_BODY(RigidBodyCapsuleShape);

    public:
        float m_radius {0.3f};
        float m_half_height {0.7f};
    };

    REFLECTION_TYPE(RigidBodyActorRes)
    CLASS(RigidBodyActorRes, Fields)
    {
        REFLECTION_BODY(RigidBodyActorRes);

    public:
        std::vector<Reflection::ReflectionPtr<RigidBodyShapeBase>> m_shapes;
        float                                                      m_inverse_mass;
        int                                                        m_actor_type;

        ~RigidBodyActorRes()
        {
            for (auto& shape : m_shapes)
            {
                PILOT_REFLECTION_DELETE(shape);
            }
            m_shapes.clear();
        }
    };
} // namespace Pilot