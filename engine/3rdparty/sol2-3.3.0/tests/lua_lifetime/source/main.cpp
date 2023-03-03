#include <sol/sol.hpp>

sol::function fn;

int main(int, char*[]) {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	fn = lua["print"];
	fn.abandon();
	return 0;
}
