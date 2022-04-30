#include "runtime/function/character/character.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/input/input_system.h"

namespace Pilot
{
    Character::Character(GObject* character_object) { setObject(character_object); }

    void Character::setObject(GObject* gobject)
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

    void Character::tick()
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

        const int fps = PilotEngine::getInstance().getFPS();
        if (fps == 0)
            return;

        float blend_ratio = std::max(1.f, motor_component->getSpeedRatio());

        float frame_length = 1.f / static_cast<float>(fps) * blend_ratio;
        m_position =
            (m_position * (k_camera_blend_time - frame_length) + new_position * frame_length) / k_camera_blend_time;
        m_position =
            (m_position * (k_camera_blend_time - frame_length) + new_position * frame_length) / k_camera_blend_time;
    }
} // namespace Pilot