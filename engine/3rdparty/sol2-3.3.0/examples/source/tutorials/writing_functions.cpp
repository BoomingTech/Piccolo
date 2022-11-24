#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

std::string my_function(int D_count, std::string original) {
	// Create a string with the letter 'D' "D_count" times,
	// append it to 'original'
	return original + std::string(D_count, 'D');
}

int main() {

	sol::state lua;

	lua["my_func"] = my_function;             // way 1
	lua.set("my_func", my_function);          // way 2
	lua.set_function("my_func", my_function); // way 3

	// This function is now accessible as 'my_func' in
	// lua scripts / code run on this state:
	lua.script("some_str = my_func(1, 'Da')");

	// Read out the global variable we stored in 'some_str' in
	// the quick lua code we just executed
	std::string some_str = lua["some_str"];
	sol_c_assert(some_str == "DaD");

	return 0;
}
