#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main(int, char*[]) {

	struct bark {
		int operator()(int x) {
			return x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<bark>("bark",
	     sol::meta_function::call_function,
	     &bark::operator());

	bark b;
	lua.set("b", &b);

	sol::table b_as_table = lua["b"];
	sol::table b_metatable = b_as_table[sol::metatable_key];
	sol::function b_call = b_metatable["__call"];
	sol::function b_as_function = lua["b"];

	int result1 = b_as_function(1);
	// pass 'self' directly to argument
	int result2 = b_call(b, 1);
	sol_c_assert(result1 == result2);
	sol_c_assert(result1 == 1);
	sol_c_assert(result2 == 1);
}
