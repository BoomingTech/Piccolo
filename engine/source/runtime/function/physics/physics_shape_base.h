#pragma once

#include "runtime/core/math/transform.h"

namespace Pilot
{
    enum class ShapeType
    {
        AABB    = 1,
        OBB     = 2,
        SPHERE  = 3,
        INVALID = 256
    };

    class PhysicsShapeBase
    {
    public:
        PhysicsShapeBase(Transform* local_tarnsform) : m_local_transform(local_tarnsform) {}
        PhysicsShapeBase() {}
        virtual ~PhysicsShapeBase() {}

        ShapeType getType() const { return m_type; }
        void      setType(ShapeType type) { m_type = type; }

        Transform* getLocalTransform() const { return m_local_transform; }

    private:
        ShapeType  m_type {ShapeType::INVALID};
        Transform* m_local_transform {nullptr};
    };
} // namespace Pilot