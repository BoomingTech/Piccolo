#pragma once

#include "runtime/core/math/transform.h"

namespace Pilot
{
    enum class ShapeType
    {
        aabb    = 1,
        obb     = 2,
        sphere  = 3,
        invalid = 255
    };

    class PhysicsShapeBase
    {
    public:
        PhysicsShapeBase(const Transform& local_tarnsform) : m_local_transform(local_tarnsform) {}
        PhysicsShapeBase() {}
        virtual ~PhysicsShapeBase() {}

        ShapeType getType() const { return m_type; }
        void      setType(ShapeType type) { m_type = type; }

        const Transform& getLocalTransform() const { return m_local_transform; }

    private:
        ShapeType  m_type {ShapeType::invalid};
        Transform m_local_transform ;
    };
} // namespace Pilot