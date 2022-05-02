#pragma once
#include "runtime/core/meta/reflection/reflection.h"

namespace Pilot
{
    class GObject;
    // Component
    REFLECTION_TYPE(Component)
    CLASS(Component, WhiteListFields)
    {
        REFLECTION_BODY(Component)
    protected:
        GObject* m_parent_object;
        bool     m_is_dirty {false};

    public:
        Component(GObject * object) : m_parent_object {object} {}
        Component() {}
        virtual ~Component() { m_parent_object = nullptr; }

        void setParentObject(GObject * object) { m_parent_object = object; }

        virtual void tick(float delta_time) {};

        bool m_tick_in_editor_mode {false};
    };

} // namespace Pilot
