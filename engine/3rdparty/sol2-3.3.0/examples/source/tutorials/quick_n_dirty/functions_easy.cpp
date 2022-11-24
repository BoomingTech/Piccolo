#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main(int, char*[]) {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.script("function f (a, b, c, d) return 1 end");
	lua.script("function g (a, b) return a + b end");

	// sol::function is often easier:
	// takes a variable number/types of arguments...
	sol::function fx = lua["f"];
	// fixed signature std::function<...>
	// can be used to tie a sol::function down
	std::function<int(int, double, int, std::string)> stdfx
	     = fx;

	int is_one = stdfx(1, 34.5, 3, "bark");
	sol_c_assert(is_one == 1);
	int is_also_one = fx(1, "boop", 3, "bark");
	sol_c_assert(is_also_one == 1);

	// call through operator[]
	int is_three = lua["g"](1, 2);
	sol_c_assert(is_three == 3);
	double is_4_8 = lua["g"](2.4, 2.4);
	sol_c_assert(is_4_8 == 4.8);

	return 0;
}
