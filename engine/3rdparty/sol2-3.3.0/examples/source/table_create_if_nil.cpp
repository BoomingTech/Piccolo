#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

void create_namespace_sf(sol::state& lua) {
	// this would explode
	// lua["sf"]["value"] = 256;
	lua[sol::create_if_nil]["sf"]["value"] = 256;
}

int main(int, char*[]) {

	std::cout << "=== sol::lua_value/sol::array_value ==="
	          << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	const auto& code = R"(
		print(sf)
		print(sf.value)
		assert(sf.value == 256)
	)";

	auto result
	     = lua.safe_script(code, sol::script_pass_on_error);
	// did not work
	sol_c_assert(!result.valid());

	// create values
	create_namespace_sf(lua);

	auto result2
	     = lua.safe_script(code, sol::script_pass_on_error);
	// it worked properly
	sol_c_assert(result2.valid());

	std::cout << std::endl;

	return 0;
}
