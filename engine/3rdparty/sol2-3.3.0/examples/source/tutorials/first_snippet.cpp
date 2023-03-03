#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> // or #include "sol.hpp", whichever suits your needs

int main(int argc, char* argv[]) {
	// silence unused warnings
	(void)argc;
	(void)argv;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.script("print('bark bark bark!')");

	return 0;
}
