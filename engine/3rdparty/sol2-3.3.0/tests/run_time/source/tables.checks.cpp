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

struct not_a_table_at_all { };

TEST_CASE("tables/lua_table", "check that lua_table accepts only lua-style tables, and not other types") {
	sol::state lua;
	lua["f"] = [](sol::lua_table lt) {
		int x = lt["a"];
		REQUIRE(x == 3);
	};
	lua["ud"] = not_a_table_at_all {};

	auto result0 = lua.safe_script("t = { a = 3 }", sol::script_pass_on_error);
	REQUIRE(result0.valid());
	auto result1 = lua.safe_script("f(t)", sol::script_pass_on_error);
	REQUIRE(result1.valid());

	auto result2 = lua.safe_script("f(ud)", sol::script_pass_on_error);
	REQUIRE_FALSE(result2.valid());
	auto result3 = lua.safe_script("f(24)", sol::script_pass_on_error);
	REQUIRE_FALSE(result3.valid());
	auto result4 = lua.safe_script("f(nil)", sol::script_pass_on_error);
	REQUIRE_FALSE(result4.valid());
	auto result5 = lua.safe_script("f('bark')", sol::script_pass_on_error);
	REQUIRE_FALSE(result5.valid());
}
