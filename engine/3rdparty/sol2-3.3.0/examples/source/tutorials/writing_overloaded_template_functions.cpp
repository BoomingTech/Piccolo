#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

template <typename A, typename B>
auto my_add(A a, B b) {
	return a + b;
}

int main() {

	sol::state lua;

	auto int_function_pointer = &my_add<int, int>;
	auto string_function_pointer
	     = &my_add<std::string, std::string>;
	// adds 2 integers, or "adds" (concatenates) two strings
	lua["my_combine"] = sol::overload(
	     int_function_pointer, string_function_pointer);

	lua.script("my_num = my_combine(1, 2)");
	lua.script(
	     "my_str = my_combine('bark bark', ' woof woof')");
	int my_num = lua["my_num"];
	std::string my_str = lua["my_str"];
	sol_c_assert(my_num == 3);
	sol_c_assert(my_str == "bark bark woof woof");

	return 0;
}
