#pragma once
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    class GObject;
    // Component
    REFLECTION_TYPE(Component)
    CLASS(Component, WhiteListFields)
    {
        REFLECTION_BODY(Component)
    protected:
        std::weak_ptr<GObject> m_parent_object;
        bool                   m_is_dirty {false};
        bool                   m_is_scale_dirty {false};

    public:
        Component() = default;
        virtual ~Component() {}

        // Instantiating the component after definition loaded
        virtual void postLoadResource(std::weak_ptr<GObject> parent_object) { m_parent_object = parent_object; }

        virtual void tick(float delta_time) {};

        bool isDirty() const { return m_is_dirty; }

        void setDirtyFlag(bool is_dirty) { m_is_dirty = is_dirty; }

        bool m_tick_in_editor_mode {false};
    };

} // namespace Piccolo
