#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

int main(int argc, char* argv[]) {
	// silence unused warnings
	(void)argc;
	(void)argv;
	sol::state lua;
	lua.open_libraries(sol::lib::base,
	     sol::lib::coroutine,
	     sol::lib::string,
	     sol::lib::io);

	lua.script("print('bark bark bark!')");

	return 0;
}