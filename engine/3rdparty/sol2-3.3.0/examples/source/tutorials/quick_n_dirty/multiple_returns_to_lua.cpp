#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main(int, char*[]) {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua["f"] = [](int a, int b, sol::object c) {
		// sol::object can be anything here: just pass it
		// through
		return std::make_tuple(a, b, c);
	};

	std::tuple<int, int, int> result = lua["f"](100, 200, 300);
	const std::tuple<int, int, int> expected(100, 200, 300);
	sol_c_assert(result == expected);

	std::tuple<int, int, std::string> result2;
	result2 = lua["f"](100, 200, "BARK BARK BARK!");
	const std::tuple<int, int, std::string> expected2(
	     100, 200, "BARK BARK BARK!");
	sol_c_assert(result2 == expected2);

	int a, b;
	std::string c;
	sol::tie(a, b, c) = lua["f"](100, 200, "bark");
	sol_c_assert(a == 100);
	sol_c_assert(b == 200);
	sol_c_assert(c == "bark");

	lua.script(R"(
		a, b, c = f(150, 250, "woofbark")
		assert(a == 150)
		assert(b == 250)
		assert(c == "woofbark")
	)");

	return 0;
}
