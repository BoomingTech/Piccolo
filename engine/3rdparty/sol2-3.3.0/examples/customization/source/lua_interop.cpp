#include <lua_interop.hpp>

#include <entity.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

void register_lua(sol::state& lua) {
	lua.new_usertype<entity>("entity",
	     "position",
	     sol::property(
	          &entity::get_position, &entity::set_position));
}
