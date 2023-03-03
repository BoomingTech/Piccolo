#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

int main() {
	sol::state lua;

	lua.script("function f (a, b, c) return a, b, c end");

	std::tuple<int, int, int> result;
	result = lua["f"](1, 2, 3);
	sol_c_assert(result == std::make_tuple(1, 2, 3));
	int a, b;
	std::string c;
	// NOTE: sol::tie, NOT std::tie
	// (ESS OH ELL prefix, not ESS TEE DEE prefix)
	sol::tie(a, b, c) = lua["f"](1, 2, "bark");
	sol_c_assert(a == 1);
	sol_c_assert(b == 2);
	sol_c_assert(c == "bark");

	return 0;
}
