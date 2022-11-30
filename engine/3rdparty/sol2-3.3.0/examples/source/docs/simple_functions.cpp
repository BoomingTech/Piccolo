#define SOL_ALL_SAFETIES_ON 1

#include <sol/sol.hpp>

int main() {
	sol::state lua;
	int x = 0;
	lua.set_function("beep", [&x] { ++x; });
	lua.script("beep()");
	sol_c_assert(x == 1);

	sol::function beep = lua["beep"];
	beep();
	sol_c_assert(x == 2);

	return 0;
}
