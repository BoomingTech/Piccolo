#pragma once

#include "runtime/function/physics/physics_shape_base.h"

#include "runtime/core/math/vector3.h"

namespace Piccolo
{
    class PhysicsOBBShape : public PhysicsShapeBase
    {
    public:
        PhysicsOBBShape(const Transform& transform, const Vector3 half_dimensions) :
            PhysicsShapeBase(transform), m_half_dimensions(half_dimensions)
        {}
        ~PhysicsOBBShape() override {}

        Vector3 getHalfDimensions() const { return m_half_dimensions; }

    private:
        Vector3 m_half_dimensions;
    };
} // namespace Piccolo
