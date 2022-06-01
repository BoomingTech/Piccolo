#pragma once

#include "runtime/core/math/vector3.h"

#include "runtime/function/physics/physics_shape_base.h"

namespace Pilot
{
    class PhysicsAABBShape : public PhysicsShapeBase
    {
    public:
        PhysicsAABBShape(const Transform& transform, const Vector3 half_dimensions) :
            PhysicsShapeBase(transform), m_half_dimensions(half_dimensions)
        {}

        ~PhysicsAABBShape() override {}

        Vector3 getHalfDimensions() const { return m_half_dimensions; }

    private:
        Vector3 m_half_dimensions;
    };
} // namespace Pilot