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

#include <iostream>

TEST_CASE("tables/table_proxy override_value", "allow override_value by way of key") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base, sol::lib::io);

	sol::optional<int> not_there = lua["a"]["b"]["c"];
	REQUIRE_FALSE(static_cast<bool>(not_there));
	lua["a"].force()["b"].force()["c"] = 357;
	sol::optional<int> totally_there = lua["a"]["b"]["c"];
	REQUIRE(static_cast<bool>(totally_there));
	REQUIRE(*totally_there == 357);
}

TEST_CASE("tables/insertion override", "allow override all non-table values plus final value") {
	SECTION("traverse") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<int> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		lua.traverse_set(sol::override_value, "a", "b", "c", 357);
		sol::optional<int> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there));
		REQUIRE(*totally_there == 357);

		lua.traverse_set(sol::override_value, "a", "b", 500);
		sol::optional<int> b_totally_there = lua["a"]["b"];
		sol::optional<int> totally_not_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(b_totally_there));
		REQUIRE(*b_totally_there == 500);
		REQUIRE_FALSE(static_cast<bool>(totally_not_there));
	}
	SECTION("table_proxy") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<int> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		lua[sol::override_value]["a"]["b"]["c"] = 357;
		sol::optional<int> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there));
		REQUIRE(*totally_there == 357);

		lua[sol::override_value]["a"]["b"] = 500;
		sol::optional<int> b_totally_there = lua["a"]["b"];
		sol::optional<int> totally_not_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(b_totally_there));
		REQUIRE(*b_totally_there == 500);
		REQUIRE_FALSE(static_cast<bool>(totally_not_there));
	}
	SECTION("complex table_proxy") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<int> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		int definitely_there = (lua[sol::override_value]["a"]["b"]["c"] = 357);
		sol::optional<int> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there));
		REQUIRE(*totally_there == definitely_there);
		REQUIRE(*totally_there == 357);
		REQUIRE(definitely_there == 357);
	}
}

TEST_CASE("tables/insertion update_if_empty", "allow updating a value only if it's missing") {
	SECTION("traverse") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<int> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		lua.traverse_set(sol::update_if_empty, "a", "b", "c", 357);
		sol::optional<int> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there));
		REQUIRE(*totally_there == 357);
	}
	SECTION("table_proxy") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<int> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		lua[sol::update_if_empty]["a"]["b"]["c"] = 357;
		sol::optional<int> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there));
		REQUIRE(*totally_there == 357);

		lua[sol::update_if_empty]["a"]["b"]["c"] = 500;
		sol::optional<int> totally_there_still = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there_still));
		REQUIRE(*totally_there_still == 357);
	}
	SECTION("table_proxy invoker") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<int> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		lua[sol::update_if_empty]["a"]["b"]["c"] = sol::push_invoke([]() { return 357; });
		sol::optional<int> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there));
		REQUIRE(*totally_there == 357);

		lua[sol::update_if_empty]["a"]["b"]["c"] = sol::push_invoke([]() { return 1000; });
		sol::optional<int> totally_there_still = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_there_still));
		REQUIRE(*totally_there_still == 357);
	}
}

TEST_CASE("tables/get create_if_nil", "create tables all the way down") {
	SECTION("traverse non-optional") {
		sol::state lua;
		sol::stack_guard luasg_outer(lua);

		{
			sol::stack_guard luasg_inner_1(lua);
			sol::optional<sol::table> not_there = lua["a"]["b"]["c"];
			REQUIRE_FALSE(static_cast<bool>(not_there));
		}
		{
			sol::stack_guard luasg_inner_2(lua);
			sol::table totally_created = lua.traverse_get<sol::table>(sol::create_if_nil, "a", "b", "c");
			REQUIRE(totally_created.size() == 0);
		}
	}
	SECTION("traverse") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<sol::table> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		sol::optional<sol::table> totally_created = lua.traverse_get<sol::optional<sol::table>>(sol::create_if_nil, "a", "b", "c");
		sol::optional<sol::table> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_created));
		REQUIRE(static_cast<bool>(totally_there));
	}
	SECTION("table_proxy non-optional") {
		sol::state lua;
		sol::stack_guard luasg_outer(lua);

		{
			sol::stack_guard luasg_inner_1(lua);
			sol::optional<sol::table> not_there = lua["a"]["b"]["c"];
			REQUIRE_FALSE(static_cast<bool>(not_there));
		}
		{
			sol::stack_guard luasg_inner_2(lua);
			sol::table totally_created = lua[sol::create_if_nil]["a"]["b"]["c"];
			REQUIRE(totally_created.size() == 0);
		}
	}

	SECTION("table_proxy") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		sol::optional<sol::table> not_there = lua["a"]["b"]["c"];
		REQUIRE_FALSE(static_cast<bool>(not_there));
		sol::optional<sol::table> totally_created = lua[sol::create_if_nil]["a"]["b"]["c"];
		sol::optional<sol::table> totally_there = lua["a"]["b"]["c"];
		REQUIRE(static_cast<bool>(totally_created));
		REQUIRE(static_cast<bool>(totally_there));
	}
}
