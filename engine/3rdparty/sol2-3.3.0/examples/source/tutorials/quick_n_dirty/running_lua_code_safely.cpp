#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <fstream>
#include <iostream>

int main(int, char*[]) {
	std::cout << "=== running lua code (safely) ==="
	          << std::endl;

	{
		std::ofstream out("a_lua_script.lua");
		out << "print('hi from a lua script file')";
	}

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// load and execute from string
	auto result = lua.safe_script(
	     "a = 'test'", sol::script_pass_on_error);
	if (!result.valid()) {
		sol::error err = result;
		std::cerr << "The code has failed to run!\n"
		          << err.what() << "\nPanicking and exiting..."
		          << std::endl;
		return 1;
	}

	// load and execute from file
	auto script_from_file_result = lua.safe_script_file(
	     "a_lua_script.lua", sol::script_pass_on_error);
	if (!script_from_file_result.valid()) {
		sol::error err = script_from_file_result;
		std::cerr
		     << "The code from the file has failed to run!\n"
		     << err.what() << "\nPanicking and exiting..."
		     << std::endl;
		return 1;
	}

	// run a script, get the result
	sol::optional<int> maybe_value = lua.safe_script(
	     "return 54", sol::script_pass_on_error);
	sol_c_assert(maybe_value.has_value());
	sol_c_assert(*maybe_value == 54);

	auto bad_code_result = lua.safe_script(
	     "123 herp.derp", sol::script_pass_on_error);
	sol_c_assert(!bad_code_result.valid());

	// you can also specify a handler function, and it'll
	// properly work here
	auto bad_code_result2 = lua.script("123 herp.derp",
	     [](lua_State*, sol::protected_function_result pfr) {
		     // pfr will contain things that went wrong, for
		     // either loading or executing the script Can
		     // throw your own custom error You can also just
		     // return it, and let the call-site handle the
		     // error if necessary.
		     return pfr;
	     });
	// it did not work
	sol_c_assert(!bad_code_result2.valid());

	// the default handler panics or throws, depending on your
	// settings uncomment for explosions: auto bad_code_result_2
	// = lua.script("bad.code", &sol::script_default_on_error);

	std::cout << std::endl;

	{ std::remove("a_lua_script.lua"); }

	return 0;
}
