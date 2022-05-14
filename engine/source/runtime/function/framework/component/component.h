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

        virtual void postLoadResource(GObject* parent_object)
        {
            m_parent_object = parent_object;
        }

        void setParentObject(GObject * object) { m_parent_object = object; }

        virtual void tick(float delta_time) {};

    };

} // namespace Pilot
