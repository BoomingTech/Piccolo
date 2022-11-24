#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

struct ship {
	int bullets = 20;
	int life = 100;

	bool shoot() {
		if (bullets > 0) {
			--bullets;
			// successfully shot
			return true;
		}
		// cannot shoot
		return false;
	}

	bool hurt(int by) {
		life -= by;
		// have we died?
		return life < 1;
	}
};


int main() {

	std::cout << "=== usertype basics ===" << std::endl;

	static const bool way_1 = true;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	if (way_1) {
		lua.new_usertype<ship>(
		     "ship", // the name of the class, as you want it
		             // to be used in lua List the member
		             // functions you wish to bind:
		             // "name_of_item",
		             // &class_name::function_or_variable
		     "shoot",
		     &ship::shoot,
		     "hurt",
		     &ship::hurt,
		     // bind variable types, too
		     "life",
		     &ship::life,
		     // names in lua don't have to be the same as C++,
		     // but it probably helps if they're kept the same,
		     // here we change it just to show its possible
		     "bullet_count",
		     &ship::bullets);
	}
	else {
		// set usertype explicitly, with the given name
		sol::usertype<ship> usertype_table
		     = lua.new_usertype<ship>("ship");
		usertype_table["shoot"] = &ship::shoot;
		usertype_table["hurt"] = &ship::hurt;
		usertype_table["life"] = &ship::life;
		usertype_table["bullet_count"] = &ship::bullets;
	}

	const auto& code = R"(
		fwoosh = ship.new()
		-- note the ":" that is there: this is mandatory for member function calls
		-- ":" means "pass self" in Lua
		local success = fwoosh:shoot()
		local is_dead = fwoosh:hurt(20)
		-- check if it works
		print(is_dead) -- the ship is not dead at this point
		print(fwoosh.life .. "life left") -- 80 life left
		print(fwoosh.bullet_count) -- 19
	)";


	lua.script(code);

	std::cout << std::endl;

	return 0;
}
