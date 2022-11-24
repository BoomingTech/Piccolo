#define SOL_ALL_SAFETIES_ON 1
#define SOL_ENABLE_INTEROP \
	1 // MUST be defined to use interop features
#include <sol/sol.hpp>

#include "Player.h"
#include <tolua++.h>
// pick or replace the include
// with whatever generated file you've created
#include "tolua_Player.h"

#include <iostream>

// tolua code lifted from some blog, if the link dies
// I don't know where else you're gonna find the reference,
// http://usefulgamedev.weebly.com/tolua-example.html


/* NOTE: there is no sol_lua_interop_get here,
 because tolua types are -- thankfully -- memory-compatible
 in most cases with sol.
 Please check other examples like kaguya or LuaBribe for an
 example of how to also write the getter for your type*/
template <typename T, typename Handler>
inline bool sol_lua_interop_check(sol::types<T>, lua_State* L,
     int relindex, sol::type index_type, Handler&& handler,
     sol::stack::record& tracking) {
	tracking.use(1);
	// just marking unused parameters for no compiler warnings
	(void)index_type;
	(void)handler;
	int index = lua_absindex(L, relindex);
	std::string name = sol::detail::short_demangle<T>();
	tolua_Error tolua_err {};
	int r = tolua_isusertype(
	     L, index, name.c_str(), 0, &tolua_err);
	if (r == 0) {
		// tolua seems to leave garbage on the stack
		// when the check fails
		// thanks, tolua
		lua_pop(L, 2);
		return false;
	}
	return true;
}

void register_sol_stuff(lua_State* L) {
	// grab raw state and put into state_view
	// state_view is cheap to construct
	sol::state_view lua(L);
	// bind and set up your things: everything is entirely
	// self-contained
	lua["f"] = sol::overload(
	     [](Player& from_tolua) {
		     std::cout << "calling 1-argument version with "
		                  "tolua-created Player { health:"
		               << from_tolua.getHealth() << " }"
		               << std::endl;
		     sol_c_assert(from_tolua.getHealth() == 4);
	     },
	     [](Player& from_tolua, int second_arg) {
		     std::cout << "calling 2-argument version with "
		                  "tolua-created Player { health: "
		               << from_tolua.getHealth()
		               << " } and integer argument of "
		               << second_arg << std::endl;
		     sol_c_assert(from_tolua.getHealth() == 4);
		     sol_c_assert(second_arg == 5);
	     });
}

void check_with_sol(lua_State* L) {
	sol::state_view lua(L);
	Player& obj = lua["obj"];
	(void)obj;
	sol_c_assert(obj.getHealth() == 4);
}

int main(int, char*[]) {

	std::cout << "=== interop example (tolua) ===" << std::endl;
	std::cout << "(code lifted from a sol2 user's use case: "
	             "https://github.com/ThePhD/sol2/issues/"
	             "511#issuecomment-331729884)"
	          << std::endl;

	lua_State* L = luaL_newstate();

	luaL_openlibs(
	     L); // initalize all lua standard library functions
	tolua_open(L); // initalize tolua
	tolua_Player_open(
	     L); // make Player class accessible from LUA


	register_sol_stuff(L);

	const auto code = R"(
obj = Player:new()
obj:setHealth(4)

f(obj) -- call 1 argument version
f(obj, 5) -- call 2 argument version
)";

	if (luaL_dostring(L, code)) {
		lua_error(L); // crash on error
	}

	check_with_sol(L);

	return 0;
}