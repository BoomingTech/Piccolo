#pragma once

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/transform.h"
#include "runtime/engine.h"

#include "runtime/function/framework/component/component.h"

namespace Pilot
{
    REFLECTION_TYPE(TransformComponent)
    CLASS(TransformComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(TransformComponent)
    protected:
        META(Enable)
        Transform m_transform;

        Transform m_transform_buffer[2];
        size_t    m_current_index {0};
        size_t    m_next_index {1};

    public:
        TransformComponent() {}
        TransformComponent(const Transform& transform, GObject* /*unused*/)
        {
            m_transform_buffer[0] = transform;
            m_transform_buffer[1] = transform;
        }

        ~TransformComponent() override {}

        Vector3    getPosition() const { return m_transform_buffer[m_current_index].m_position; }
        Vector3    getScale() const { return m_transform_buffer[m_current_index].m_scale; }
        Quaternion getRotation() const { return m_transform_buffer[m_current_index].m_rotation; }

        void setPosition(const Vector3& new_transition)
        {
            m_transform_buffer[m_next_index].m_position = new_transition;
        }
        void setScale(const Vector3& new_scale) { m_transform_buffer[m_next_index].m_scale = new_scale; }
        void setRotation(const Quaternion& new_rotation) { m_transform_buffer[m_next_index].m_rotation = new_rotation; }

        const Transform& getTransformConst() const { return m_transform_buffer[m_current_index]; }
        Transform&       getTransform() { return m_transform_buffer[m_next_index]; }

        Matrix4x4 getMatrix() const { return m_transform_buffer[m_current_index].getMatrix(); }

        void tick(float delta_time) override
        {
            std::swap(m_current_index, m_next_index);
            m_transform = m_transform_buffer[m_current_index];

            if (g_is_editor_mode)
            {
                m_transform_buffer[m_next_index] = m_transform;
            }
        }
        void destroy() override {}
    };
} // namespace Pilot
