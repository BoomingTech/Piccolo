#define SOL_ALL_SAFTIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main() {
	const auto& co_lua_script = R"(
function loop()
	while counter ~= 30
	do
		coroutine.yield(counter);
		counter = counter + 1;
	end
	return counter
end
)";

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine);
	/*
	lua.script_file("co.lua");
	we load string directly rather than use a file
	*/
	lua.script(co_lua_script);
	sol::coroutine loop_coroutine = lua["loop"];
	// set counter variable in C++
	// (can set it to something else to
	// have loop_coroutine() yield different values)
	lua["counter"] = 20;

	// example of using and re-using coroutine
	// you do not have to use coroutines in a loop,
	// this is just the example

	// we start from 0;
	// we want 10 values, and we only want to
	// run if the coroutine "loop_coroutine" is valid
	for (int counter = 0; counter < 10 && loop_coroutine;
	     ++counter) {
		// Alternative: counter < 10 && cr.valid()

		// Call the coroutine, does the computation and then
		// suspends once it returns, we get the value back from
		// the return and then can use it we can either leave
		// the coroutine like that can come to it later, or
		// loop back around
		int value = loop_coroutine();
		std::cout << "In C++: " << value << std::endl;
	}

	return 0;
}
