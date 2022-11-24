#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main(int, char*[]) {
	std::cout << "=== passing arguments to scripts ==="
	          << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	const auto& my_script = R"(
local a,b,c = ...
print(a,b,c)
	)";

	sol::load_result fx = lua.load(my_script);
	if (!fx.valid()) {
		sol::error err = fx;
		std::cerr << "failed to load string-based script into "
		             "the program"
		          << err.what() << std::endl;
	}

	// prints "your arguments here"
	fx("your", "arguments", "here");

	return 0;
}
