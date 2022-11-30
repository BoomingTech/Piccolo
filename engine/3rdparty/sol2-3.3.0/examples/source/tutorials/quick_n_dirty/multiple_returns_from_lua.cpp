#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main(int, char*[]) {
	sol::state lua;

	lua.script("function f (a, b, c) return a, b, c end");

	std::tuple<int, int, int> result;
	result = lua["f"](100, 200, 300);
	// result == { 100, 200, 300 }
	int a;
	int b;
	std::string c;
	sol::tie(a, b, c) = lua["f"](100, 200, "bark");
	sol_c_assert(a == 100);
	sol_c_assert(b == 200);
	sol_c_assert(c == "bark");

	return 0;
}
