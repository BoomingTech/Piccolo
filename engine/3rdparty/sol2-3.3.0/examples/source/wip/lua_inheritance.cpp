#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main(int, char*[]) {
	std::cout << "=== lua inheritance ===" << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	/* This example is currently under construction.
	For inheritance and classes within Lua,
	consider using kikito's middleclass
	-- https://github.com/kikito/middleclass */

	return 0;
}
