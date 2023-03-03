#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main() {

	const auto& code = R"(
	bark = { 
		woof = {
			[2] = "arf!" 
		} 
	}
	)";

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.script(code);

	// produces table_proxy, implicitly converts to std::string,
	// quietly destroys table_proxy
	std::string arf_string = lua["bark"]["woof"][2];

	// lazy-evaluation of tables
	auto x = lua["bark"];
	auto y = x["woof"];
	auto z = y[2];

	// retrivies value inside of lua table above
	std::string value = z;
	sol_c_assert(value == "arf!");

	// Can change the value later...
	z = 20;

	// Yay, lazy-evaluation!
	int changed_value = z; // now it's 20!
	sol_c_assert(changed_value == 20);
	lua.script("assert(bark.woof[2] == 20)");

	lua["a_new_value"] = 24;
	lua["chase_tail"] = [](int chasing) {
		int r = 2;
		for (int i = 0; i < chasing; ++i) {
			r *= r;
		}
		return r;
	};

	lua.script("assert(a_new_value == 24)");
	lua.script("assert(chase_tail(2) == 16)");

	return 0;
}
