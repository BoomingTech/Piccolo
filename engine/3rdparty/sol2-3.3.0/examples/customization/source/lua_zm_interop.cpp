#include <lua_zm_interop.hpp>

#include <zm/vec3.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

bool sol_lua_check(sol::types<zm::vec3>, lua_State* L, int index,
     std::function<sol::check_handler_type> handler,
     sol::stack::record& tracking) {
	// use sol's method for checking
	// specifically for a table
	return sol::stack::check<sol::lua_table>(
	     L, index, handler, tracking);
}

zm::vec3 sol_lua_get(sol::types<zm::vec3>, lua_State* L,
     int index, sol::stack::record& tracking) {
	sol::lua_table vec3table
	     = sol::stack::get<sol::lua_table>(L, index, tracking);
	float x = vec3table["x"];
	float y = vec3table["y"];
	float z = vec3table["z"];
	return zm::vec3 { x, y, z };
}

int sol_lua_push(
     sol::types<zm::vec3>, lua_State* L, const zm::vec3& v) {
	// create table
	sol::state_view lua(L);
	sol::table vec3table = sol::table::create_with(
	     L, "x", v.x, "y", v.y, "z", v.z);
	// use base sol method to
	// push the table
	int amount = sol::stack::push(L, vec3table);
	// return # of things pushed onto stack
	return amount;
}
