#pragma once

#include "runtime/function/physics/physics_shape_base.h"

namespace Piccolo
{
    class PhysicsSphereShape : public PhysicsShapeBase
    {
    public:
        PhysicsSphereShape(const Transform& transform, float radius) : PhysicsShapeBase(transform), m_radius(radius)
        {
            // PhysicsShapeBase::PhysicsShapeBase(transform);
        }
        ~PhysicsSphereShape() override {}

        float getRadius() const { return m_radius; }

    private:
        float m_radius {0.f};
    };
} // namespace Piccolo
