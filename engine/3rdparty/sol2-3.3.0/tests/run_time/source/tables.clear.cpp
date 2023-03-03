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

TEST_CASE("tables/clear", "clear method works and does not clobber stack") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua["t"] = sol::lua_value({ { "a", "b" }, { "c", "d" } });
	sol::table t = lua["t"];
	{
		sol::stack_guard clearsg(lua);
		t.clear();
	}
	REQUIRE(!t["a"].valid());
	REQUIRE(!t["c"].valid());
}

TEST_CASE("tables/stack clear", "stack-based clear method works and does not clobber stack") {
	sol::state lua;
	SECTION("reference based") {
		sol::stack_guard luasg(lua);
		lua["t"] = sol::lua_value({ { "a", "b" }, { "c", "d" } });
		sol::table t = lua["t"];
		REQUIRE((t["a"] == std::string("b")));
		REQUIRE((t["c"] == std::string("d")));
		{
			sol::stack_guard clearsg(lua);
			sol::stack::clear(t);
		}
		REQUIRE(!t["a"].valid());
		REQUIRE(!t["c"].valid());
	}
	SECTION("with index") {
		sol::stack_guard luasg(lua);
		lua["t"] = sol::lua_value({ { "a", "b" }, { "c", "d" } });
		sol::table t = lua["t"];
		REQUIRE((t["a"] == std::string("b")));
		REQUIRE((t["c"] == std::string("d")));
		{
			sol::stack_guard ppclearsg(lua);
			auto pp = sol::stack::push_pop(t);
			int table_index = pp.index_of(t);
			{
				sol::stack_guard clearsg(lua);
				sol::stack::clear(lua, table_index);
			}
		}
		REQUIRE(!t["a"].valid());
		REQUIRE(!t["c"].valid());
	}
}
