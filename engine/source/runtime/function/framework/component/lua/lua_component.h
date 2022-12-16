#pragma once
#include "sol/sol.hpp"
#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/lua_script.h"

namespace Piccolo
{
    REFLECTION_TYPE(LuaComponent)
    CLASS(LuaComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(LuaComponent)

    public:
        LuaComponent() = default;

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        void tick(float delta_time) override;

        template<typename T>
        static void set(std::weak_ptr<GObject> game_object, const char* name, T value);

        template<typename T>
        static T get(std::weak_ptr<GObject> game_object, const char* name);

        static void invoke(std::weak_ptr<GObject> game_object, const char* name);
    protected:
        sol::state m_lua_state;

        META(Enable)
        LuaScriptRes m_lua_script_res;
    };
} // namespace Piccolo
