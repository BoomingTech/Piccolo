#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>
#include <string>

struct int_entry {
	int value;

	int_entry() : value(0) {
	}

	int_entry(int v) : value(v) {
	}

	std::string to_string() const {
		return "int_entry(" + std::to_string(value) + ")";
	}

	bool operator==(const int_entry& e) const {
		return value == e.value;
	}
};

int main(int, char*[]) {

	std::cout << "=== sol::lua_value/sol::array_value ==="
	          << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::io);

	sol::lua_value lv_int(lua, 56);
	sol::lua_value lv_int_table(lua, { 1, 2, 3, 4, 5 });
	sol::lua_value lv_map(lua,
	     { { "bark bark", "meow hiss!" },
	          { 3, 4 },
	          { ":D", 6 } });
	sol::lua_value lv_mixed_table(lua,
	     sol::array_value {
	          1, int_entry(2), 3, int_entry(4), 5 });
	sol::lua_value lv_mixed_nested_table(lua,
	     sol::array_value { 1,
	          int_entry(2),
	          3,
	          int_entry(4),
	          sol::array_value { 5, 6, int_entry(7), "8" } });

	const auto& code = R"(
		function real_print_recursive (e, level)
			local et = type(e)
			if et == 'table' then
				io.write("{ ")
				local iters = 0
				for k, v in pairs(e) do
					if iters ~= 0  then
						io.write(", ")
					end
					real_print_recursive(k, level + 1)
					io.write(": ")
					real_print_recursive(v, level + 1)
					iters = iters + 1
				end
				io.write(" }")
			elseif et == 'string' then
				io.write('"') 
				io.write(e) 
				io.write('"')
			else
				io.write(tostring(e))
			end
			if level == 0 then
				io.write("\n")
			end
		end

		function print_recursive (e)
			real_print_recursive(e, 0)
		end
	)";

	sol::optional<sol::error> maybe_error
	     = lua.safe_script(code, sol::script_pass_on_error);
	if (maybe_error) {
		std::cerr << maybe_error->what() << std::endl;
		return 1;
	}
	sol::function print_recursive = lua["print_recursive"];

	// show it printed out
	std::cout << "lv_int: " << std::endl;
	print_recursive(lv_int);
	std::cout << std::endl;

	std::cout << "lv_int_table: " << std::endl;
	print_recursive(lv_int_table);
	std::cout << std::endl;

	std::cout << "lv_map: " << std::endl;
	print_recursive(lv_map);
	std::cout << std::endl;

	std::cout << "lv_mixed_table: " << std::endl;
	print_recursive(lv_mixed_table);
	std::cout << std::endl;

	std::cout << "lv_mixed_nested_table: " << std::endl;
	print_recursive(lv_mixed_nested_table);
	std::cout << std::endl;

	std::cout << std::endl;

	return 0;
}