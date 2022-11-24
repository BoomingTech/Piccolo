#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main(int, char*[]) {
	std::cout << "=== environment state 2 ===" << std::endl;

	sol::state lua;
	lua.open_libraries();

	sol::environment env(lua, sol::create, lua.globals());
	env["func"] = []() { return 42; };

	sol::environment env2(lua, sol::create, lua.globals());
	env2["func"] = []() { return 24; };

	lua.script("function foo() print(func()) end", env);
	lua.script("function foo() print(func()) end", env2);

	env["foo"]();  // prints 42
	env2["foo"](); // prints 24

	std::cout << std::endl;

	return 0;
}
