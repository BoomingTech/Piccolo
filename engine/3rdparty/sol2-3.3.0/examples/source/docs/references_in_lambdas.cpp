#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main(int, char*[]) {

	struct test {
		int blah = 0;
	};

	test t;
	sol::state lua;
	lua.set_function("f", [&t]() { return t; });
	lua.set_function("g", [&t]() -> test& { return t; });

	lua.script("t1 = f()");
	lua.script("t2 = g()");

	test& from_lua_t1 = lua["t1"];
	test& from_lua_t2 = lua["t2"];

	// not the same: 'f' lambda copied
	sol_c_assert(&from_lua_t1 != &t);
	// the same: 'g' lambda returned reference
	sol_c_assert(&from_lua_t2 == &t);

	return 0;
}
