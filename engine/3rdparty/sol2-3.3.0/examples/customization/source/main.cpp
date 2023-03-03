#include <lua_interop.hpp>
#include <entity.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main(int, char*[]) {
	std::cout << "=== customization: vec3 as table ==="
	          << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	std::cout << "registering entities into Lua ..."
	          << std::endl;
	register_lua(lua);
	std::cout << "running script ..." << std::endl;

	const auto& script = R"lua(
local e = entity.new()
local pos = e.position
print("pos type", type(pos))
print("pos", pos.x, pos.y, pos.z)
e.position = { x = 52, y = 5.5, z = 47.5 }
local new_pos = e.position
print("pos", pos.x, pos.y, pos.z)
print("new_pos", new_pos.x, new_pos.y, new_pos.z)
)lua";

	sol::optional<sol::error> result = lua.safe_script(script);
	if (result.has_value()) {
		std::cerr << "Something went horribly wrong: "
		          << result.value().what() << std::endl;
	}

	std::cout << "finishing ..." << std::endl;

	std::cout << std::endl;
	return 0;
}
