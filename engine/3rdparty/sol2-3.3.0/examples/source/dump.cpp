#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>


int main() {
	std::cout << "=== dump (serialize between states) ==="
	          << std::endl;

	// 2 states, transferring function from 1 to another
	sol::state lua;
	sol::state lua2;

	// we're not going to run the code on the first
	// state, so we only actually need
	// the base lib on the second state
	// (where we will run the serialized bytecode)
	lua2.open_libraries(sol::lib::base);

	// load this code (but do not run)
	sol::load_result lr
	     = lua.load("a = function (v) print(v) return v end");
	// check if it's sucessfully loaded
	sol_c_assert(lr.valid());

	// turn it into a function, then dump the bytecode
	sol::protected_function target
	     = lr.get<sol::protected_function>();
	sol::bytecode target_bc = target.dump();

	// reload the byte code
	// in the SECOND state
	auto result2 = lua2.safe_script(
	     target_bc.as_string_view(), sol::script_pass_on_error);
	// check if it was done properly
	sol_c_assert(result2.valid());

	// check in the second state if it was valid
	sol::protected_function pf = lua2["a"];
	int v = pf(25557);
	sol_c_assert(v == 25557);

	return 0;
}