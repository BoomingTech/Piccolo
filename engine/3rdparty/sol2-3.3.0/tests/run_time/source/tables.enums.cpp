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

#include "sol_test.hpp"

#include <catch2/catch_all.hpp>

enum weak_direction { up, down, left, right };
enum class strong_direction { up, down, left, right };

TEST_CASE("tables/as enums", "Making sure enums can be put in and gotten out as values") {

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua["strong_direction"]
	     = lua.create_table_with("up", weak_direction::up, "down", weak_direction::down, "left", weak_direction::left, "right", weak_direction::right);

	sol::object obj = lua["strong_direction"]["up"];
	bool isdir = obj.is<weak_direction>();
	REQUIRE(isdir);
	auto dir = obj.as<weak_direction>();
	REQUIRE(dir == weak_direction::up);
}

TEST_CASE("tables/as enum classes", "Making sure enums can be put in and gotten out as values") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua["strong_direction"]
	     = lua.create_table_with("up", strong_direction::up, "down", strong_direction::down, "left", strong_direction::left, "right", strong_direction::right);

	sol::object obj = lua["strong_direction"]["up"];
	bool isdir = obj.is<strong_direction>();
	REQUIRE(isdir);
	auto dir = obj.as<strong_direction>();
	REQUIRE(dir == strong_direction::up);
}

TEST_CASE("tables/new_enum", "Making sure enums can be put in and gotten out as values") {
	enum class strong_direction { up, down, left, right };

	SECTION("variadics") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_enum("strong_direction",
		     "up",
		     strong_direction::up,
		     "down",
		     strong_direction::down,
		     "left",
		     strong_direction::left,
		     "right",
		     strong_direction::right);

		strong_direction d = lua["strong_direction"]["left"];
		REQUIRE(d == strong_direction::left);
		auto result = lua.safe_script("strong_direction.left = 50", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
		d = lua["strong_direction"]["left"];
		REQUIRE(d == strong_direction::left);
	}
	SECTION("initializer_list") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_enum<strong_direction>("strong_direction",
		     { { "up", strong_direction::up },
		          { "down", strong_direction::down },
		          { "left", strong_direction::left },
		          { "right", strong_direction::right } });

		strong_direction d = lua["strong_direction"]["left"];
		REQUIRE(d == strong_direction::left);
		auto result = lua.safe_script("strong_direction.left = 50", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
		d = lua["strong_direction"]["left"];
		REQUIRE(d == strong_direction::left);
	}
}
