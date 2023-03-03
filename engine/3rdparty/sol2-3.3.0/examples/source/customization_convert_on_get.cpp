#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>
#include <iomanip>

struct number_shim {
	double num = 0;
};

template <typename Handler>
bool sol_lua_check(sol::types<number_shim>, lua_State* L,
     int index, Handler&& handler,
     sol::stack::record& tracking) {
	// check_usertype is a backdoor for directly checking sol2
	// usertypes
	if (!sol::stack::check_usertype<number_shim>(L, index)
	     && !sol::stack::check<double>(L, index)) {
		handler(L,
		     index,
		     sol::type_of(L, index),
		     sol::type::userdata,
		     "expected a number_shim or a number");
		return false;
	}
	tracking.use(1);
	return true;
}

number_shim sol_lua_get(sol::types<number_shim>, lua_State* L,
     int index, sol::stack::record& tracking) {
	if (sol::stack::check_usertype<number_shim>(L, index)) {
		number_shim& ns
		     = sol::stack::get_usertype<number_shim>(
		          L, index, tracking);
		return ns;
	}
	number_shim ns {};
	ns.num = sol::stack::get<double>(L, index, tracking);
	return ns;
}

int main() {
	sol::state lua;

	// Create a pass-through style of function
	lua.safe_script("function f ( a ) return a end");
	lua.set_function("g", [](double a) {
		number_shim ns;
		ns.num = a;
		return ns;
	});

	lua.script("vf = f(25) vg = g(35)");

	number_shim thingsf = lua["vf"];
	number_shim thingsg = lua["vg"];

	sol_c_assert(thingsf.num == 25);
	sol_c_assert(thingsg.num == 35);

	return 0;
}
