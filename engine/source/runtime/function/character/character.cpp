#include "runtime/function/character/character.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/input/input_system.h"

namespace Piccolo
{
    Character::Character(std::shared_ptr<GObject> character_object) { setObject(character_object); }

    GObjectID Character::getObjectID() const
    {
        if (m_character_object)
        {
            return m_character_object->getID();
        }

        return k_invalid_gobject_id;
    }

    void Character::setObject(std::shared_ptr<GObject> gobject)
    {
        m_character_object = gobject;
        if (m_character_object)
        {
            const TransformComponent* transform_component =
                m_character_object->tryGetComponentConst(TransformComponent);
            const Transform& transform = transform_component->getTransformConst();
            m_position                 = transform.m_position;
            m_rotation                 = transform.m_rotation;
        }
        else
        {
            m_position = Vector3::ZERO;
            m_rotation = Quaternion::IDENTITY;
        }
    }

    void Character::tick(float delta_time)
    {
        if (m_character_object == nullptr)
            return;

        TransformComponent* transform_component = m_character_object->tryGetComponent(TransformComponent);

        if (m_rotation_dirty)
        {
            transform_component->setRotation(m_rotation_buffer);
            m_rotation_dirty = false;
        }

        const MotorComponent* motor_component = m_character_object->tryGetComponentConst(MotorComponent);
        if (motor_component == nullptr)
        {
            return;
        }

        if (motor_component->getIsMoving())
        {
            m_rotation_buffer = m_rotation;
            transform_component->setRotation(m_rotation_buffer);
            m_rotation_dirty = true;
        }

        const Vector3& new_position = motor_component->getTargetPosition();

        m_position = new_position;

        //float blend_ratio = std::max(1.f, motor_component->getSpeedRatio());

        //float frame_length = delta_time * blend_ratio;
        //m_position =
        //    (m_position * (k_camera_blend_time - frame_length) + new_position * frame_length) / k_camera_blend_time;
        //m_position =
        //    (m_position * (k_camera_blend_time - frame_length) + new_position * frame_length) / k_camera_blend_time;
    }
} // namespace Piccolo