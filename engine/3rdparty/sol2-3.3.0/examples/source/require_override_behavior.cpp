// Thanks to OrfeasZ for their answer to
// an issue for this example!
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


#include <iostream>
#include <exception>

// Use raw function of form "int(lua_State*)"
// -- this is called a "raw C function",
// and matches the type for lua_CFunction
int LoadFileRequire(lua_State* L) {
	// use sol2 stack API to pull
	// "first argument"
	std::string path = sol::stack::get<std::string>(L, 1);

	if (path == "a") {
		std::string script = R"(
			print("Hello from module land!")
			test = 123
			return "bananas"
		)";
		// load "module", but don't run it
		luaL_loadbuffer(
		     L, script.data(), script.size(), path.c_str());
		// returning 1 object left on Lua stack:
		// a function that, when called, executes the script
		// (this is what lua_loadX/luaL_loadX functions return
		return 1;
	}

	sol::stack::push(
	     L, "This is not the module you're looking for!");
	return 1;
}

int main() {
	std::cout << "=== require override behavior ==="
	          << std::endl;

	sol::state lua;
	// need base for print,
	// need package for package/searchers/require
	lua.open_libraries(sol::lib::base, sol::lib::package);

	lua.clear_package_loaders();
	lua.add_package_loader(LoadFileRequire);

	// this will call our function for
	// the searcher and it will succeed
	auto a_result = lua.safe_script(R"(
		local a = require("a")
		print(a)
		print(test)
	)",
	     sol::script_pass_on_error);
	sol_c_assert(a_result.valid());
	try {
		// this will always fail
		auto b_result = lua.safe_script(R"(
			local b = require("b")
			print(b)
		)",
		     sol::script_throw_on_error);
		// this will not be executed because of the throw,
		// but it better be true regardless!
		sol_c_assert(!b_result.valid());
	}
	catch (const std::exception& ex) {
		// Whenever sol2 throws an exception from panic,
		// catch
		std::cout << "Something went wrong, as expected:\n"
		          << ex.what() << std::endl;
		// and CRASH / exit the application
		return 0;
	}

	// If we get here something went wrong...!
	return -1;
}
