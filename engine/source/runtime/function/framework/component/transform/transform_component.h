#pragma once

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/transform.h"

#include "runtime/function/framework/component/component.h"
#include "runtime/function/framework/object/object.h"

namespace Piccolo
{
    REFLECTION_TYPE(TransformComponent)
    CLASS(TransformComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(TransformComponent)

    public:
        TransformComponent() = default;

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        Vector3    getPosition() const { return m_transform_buffer[m_current_index].m_position; }
        Vector3    getScale() const { return m_transform_buffer[m_current_index].m_scale; }
        Quaternion getRotation() const { return m_transform_buffer[m_current_index].m_rotation; }

        void setPosition(const Vector3& new_translation);

        void setScale(const Vector3& new_scale);

        void setRotation(const Quaternion& new_rotation);

        const Transform& getTransformConst() const { return m_transform_buffer[m_current_index]; }
        Transform&       getTransform() { return m_transform_buffer[m_next_index]; }

        Matrix4x4 getMatrix() const { return m_transform_buffer[m_current_index].getMatrix(); }

        void tick(float delta_time) override;

        void tryUpdateRigidBodyComponent();

    protected:
        META(Enable)
        Transform m_transform;

        Transform m_transform_buffer[2];
        size_t    m_current_index {0};
        size_t    m_next_index {1};
    };
} // namespace Piccolo
