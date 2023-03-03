#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main() {
	std::cout << "=== variadic_args ===" << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// another function, which doubles the argument
	lua.script("function doubler (x) return x * 2 end");
	sol::protected_function doubler = lua["doubler"];

	// Function requires 2 arguments
	// rest can be variadic, but:
	// va will include everything after "a" argument,
	// which means "b" will be part of the varaidic_args list
	// too at position 0
	lua.set_function("v",
	     [doubler](int a, sol::variadic_args va, int /*b*/) {
		     int r = 0;
		     for (auto v : va) {
			     int value = doubler(
			          v); // pass directly to Lua as well!
			     r += value;
		     }
		     // Only have to add a, b was included from
		     // variadic_args and beyond use explicit "call"
		     // syntax to return exactly an integer! this is
		     // useful for ambiguous operator overloads in C++
		     // and other shenanigans
		     return r + a;
	     });

	lua.script("x = v(25, 25)");
	lua.script("x2 = v(25, 25, 100, 50, 250, 150)");
	lua.script("x3 = v(1, 2, 3, 4, 5, 6)");
	// will error: not enough arguments!
	// lua.script("x4 = v(1)");

	lua.script("assert(x == 75)");
	lua.script("assert(x2 == 1175)");
	lua.script("assert(x3 == 41)");
	lua.script("print(x)");
	lua.script("print(x2)");
	lua.script("print(x3)");

	std::cout << std::endl;

	return 0;
}