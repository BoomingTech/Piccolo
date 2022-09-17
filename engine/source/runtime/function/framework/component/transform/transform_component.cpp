#include "runtime/function/framework/component/transform/transform_component.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/rigidbody/rigidbody_component.h"

namespace Piccolo
{
    void TransformComponent::postLoadResource(std::weak_ptr<GObject> parent_gobject)
    {
        m_parent_object       = parent_gobject;
        m_transform_buffer[0] = m_transform;
        m_transform_buffer[1] = m_transform;
        m_is_dirty            = true;
    }

    void TransformComponent::setPosition(const Vector3& new_translation)
    {
        m_transform_buffer[m_next_index].m_position = new_translation;
        m_transform.m_position                      = new_translation;
        m_is_dirty                                  = true;
    }

    void TransformComponent::setScale(const Vector3& new_scale)
    {
        m_transform_buffer[m_next_index].m_scale = new_scale;
        m_transform.m_scale                      = new_scale;
        m_is_dirty                               = true;
        m_is_scale_dirty                         = true;
    }

    void TransformComponent::setRotation(const Quaternion& new_rotation)
    {
        m_transform_buffer[m_next_index].m_rotation = new_rotation;
        m_transform.m_rotation                      = new_rotation;
        m_is_dirty                                  = true;
    }

    void TransformComponent::tick(float delta_time)
    {
        std::swap(m_current_index, m_next_index);

        if (m_is_dirty)
        {
            // update transform component, dirty flag will be reset in mesh component
            tryUpdateRigidBodyComponent();
        }

        if (g_is_editor_mode)
        {
            m_transform_buffer[m_next_index] = m_transform;
        }
    }

    void TransformComponent::tryUpdateRigidBodyComponent()
    {
        if (!m_parent_object.lock())
            return;

        RigidBodyComponent* rigid_body_component = m_parent_object.lock()->tryGetComponent(RigidBodyComponent);
        if (rigid_body_component)
        {
            rigid_body_component->updateGlobalTransform(m_transform_buffer[m_current_index], m_is_scale_dirty);
            m_is_scale_dirty = false;
        }
    }

} // namespace Piccolo