#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <string>
#include <iostream>

int main() {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.script(R"(
	obj = "please don't let me die";
	)");

	sol::object keep_alive = lua["obj"];
	lua.script(R"(
	obj = nil;
	function say(msg)
		print(msg)
	end
	)");

	lua.collect_garbage();

	lua["say"](lua["obj"]);
	// still accessible here and still alive in Lua
	// even though the name was cleared
	std::string message = keep_alive.as<std::string>();
	std::cout << message << std::endl;

	// Can be pushed back into Lua as an argument
	// or set to a new name,
	// whatever you like!
	lua["say"](keep_alive);

	return 0;
}
