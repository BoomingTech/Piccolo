#pragma once

#include "runtime/core/math/transform.h"

#include "runtime/function/framework/object/object.h"

#include <vector>

namespace Pilot
{
    class Character
    {
        inline static const float k_camera_blend_time {0.3f};

    public:
        Character(GObject* character_object);

        GObject* getObject() const { return m_character_object; }
        void     setObject(GObject* gobject);

        void setPosition(const Vector3& position) { m_position = position; }
        void setRotation(const Quaternion& rotation) { m_rotation = rotation; }

        const Vector3&    getPosition() const { return m_position; }
        const Quaternion& getRotation() const { return m_rotation; }

        void tick();

    private:
        Vector3    m_position;
        Quaternion m_rotation;
        GObject*   m_character_object {nullptr};

        // hack for setting rotation frame buffer
        Quaternion m_rotation_buffer;
        bool       m_rotation_dirty;
    };
} // namespace Pilot