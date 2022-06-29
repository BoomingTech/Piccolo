#pragma once
#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/quaternion.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(Transform)
    CLASS(Transform, Fields)
    {
        REFLECTION_BODY(Transform);

    public:
        Vector3    m_position {Vector3::ZERO};
        Vector3    m_scale {Vector3::UNIT_SCALE};
        Quaternion m_rotation {Quaternion::IDENTITY};

        Transform() = default;
        Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale) :
            m_position {position}, m_scale {scale}, m_rotation {rotation}
        {}

        Matrix4x4 getMatrix() const
        {
            Matrix4x4 temp;
            temp.makeTransform(m_position, m_scale, m_rotation);
            return temp;
        }
    };
} // namespace Piccolo
