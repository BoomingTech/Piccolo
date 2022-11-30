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
	sol::thread runner = sol::thread::create(lua.lua_state());
	sol::state_view runnerstate = runner.state();
	sol::coroutine loop_coroutine = runnerstate["loop"];
	lua["counter"] = 20;

	for (int counter = 0; counter < 10 && loop_coroutine;
	     ++counter) {
		// Call the coroutine, does the computation and then
		// suspends
		int value = loop_coroutine();
		std::cout << "value is " << value << std::endl;
	}

	return 0;
}
