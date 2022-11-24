// sol2

// The MIT License (MIT)

// Copyright (c) 2013-2022 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <catch2/catch_all.hpp>

#include <sol/sol.hpp>

#include <string>

TEST_CASE("Test for Issue #1266 - add method in Lua 5.4 exposes freelist in table, unfortunately", "[sol2][regression][Issue-1266]") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	const auto& code = R"(
for k, v in pairs(table) do
	assert(k == 1) assert(v == 'item')
end)";

	SECTION("sol2 API") {
		// Create and add using sol.
		sol::table table = lua.create_table();
		table.add("item");
		lua["table"] = table;

		auto result = lua.safe_script(code, sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	SECTION("sol2 + C API") {
		// Create using C API and add using sol.
		lua_createtable(lua.lua_state(), 0, 0);
		lua_setglobal(lua.lua_state(), "table");
		sol::table table = lua["table"];
		table.add("item");

		auto result = lua.safe_script(code, sol::script_pass_on_error);
		REQUIRE(result.valid());
	}

	sol::table table = lua["table"];
	REQUIRE(table.size() == 1);
	std::string table_value = table[1];
	std::string table_value_at_size = table[table.size()];
	REQUIRE(table_value == "item");
	REQUIRE(table_value_at_size == "item");
}
