#pragma once

#include "runtime/resource/res_type/data/basic_shape.h"

#include "runtime/core/math/axis_aligned.h"
#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    enum class RigidBodyShapeType : unsigned char
    {
        box,
        sphere,
        capsule,
        invalid
    };

    REFLECTION_TYPE(RigidBodyShape)
    CLASS(RigidBodyShape, WhiteListFields)
    {
        REFLECTION_BODY(RigidBodyShape);

    public:
        Transform          m_global_transform;
        AxisAlignedBox     m_bounding_box;
        RigidBodyShapeType m_type {RigidBodyShapeType::invalid};

        META(Enable)
        Transform m_local_transform;
        META(Enable)
        Reflection::ReflectionPtr<Geometry> m_geometry;

        RigidBodyShape() = default;
        RigidBodyShape(const RigidBodyShape& res);

        ~RigidBodyShape();
    };

    REFLECTION_TYPE(RigidBodyComponentRes)
    CLASS(RigidBodyComponentRes, Fields)
    {
        REFLECTION_BODY(RigidBodyComponentRes);

    public:
        std::vector<RigidBodyShape> m_shapes;
        float                       m_inverse_mass;
        int                         m_actor_type;
    };
} // namespace Piccolo