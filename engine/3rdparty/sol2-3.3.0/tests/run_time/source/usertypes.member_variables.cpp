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

#include "common_classes.hpp"

#include <catch2/catch_all.hpp>

#include <memory>


TEST_CASE("usertype/member-variables", "allow table-like accessors to behave as member variables for usertype") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);
	sol::constructors<sol::types<float, float, float>> ctor;
	sol::usertype<Vec> udata
	     = lua.new_usertype<Vec>("Vec", ctor, "x", &Vec::x, "y", &Vec::y, "z", &Vec::z, "normalized", &Vec::normalized, "length", &Vec::length);

	REQUIRE_NOTHROW(
	     lua.safe_script("v = Vec.new(1, 2, 3)\n"
	                     "v2 = Vec.new(0, 1, 0)\n"
	                     "print(v:length())\n"));
	REQUIRE_NOTHROW(
	     lua.safe_script("v.x = 2\n"
	                     "v2.y = 2\n"
	                     "print(v.x, v.y, v.z)\n"
	                     "print(v2.x, v2.y, v2.z)\n"));
	REQUIRE_NOTHROW(
	     lua.safe_script("assert(v.x == 2)\n"
	                     "assert(v2.x == 0)\n"
	                     "assert(v2.y == 2)\n"));
	REQUIRE_NOTHROW(
	     lua.safe_script("v.x = 3\n"
	                     "local x = v.x\n"
	                     "assert(x == 3)\n"));

	struct breaks {
		sol::function f;
	};

	lua.set("b", breaks());
	lua.new_usertype<breaks>("breaks", "f", &breaks::f);

	breaks& b = lua["b"];
	{
		auto result = lua.safe_script("b.f = function () print('BARK!') end", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("b.f()", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	REQUIRE_NOTHROW(b.f());
}

TEST_CASE("usertype/reference-and-constness", "Make sure constness compiles properly and errors out at runtime") {
	struct bark {
		int var = 50;
	};
	struct woof {
		bark b;
	};

	struct nested {
		const int f = 25;
	};

	struct outer {
		nested n;
	};

	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.new_usertype<woof>("woof", "b", &woof::b);
	lua.new_usertype<bark>("bark", "var", &bark::var);
	lua.new_usertype<outer>("outer", "n", &outer::n);
	lua.set("w", woof());
	lua.set("n", nested());
	lua.set("o", outer());
	lua.set("f", sol::c_call<decltype(&nested::f), &nested::f>);
	lua.safe_script(R"(
    x = w.b
    x.var = 20
    val = w.b.var == x.var
    v = f(n);
    )");

	woof& w = lua["w"];
	bark& x = lua["x"];
	nested& n = lua["n"];
	int v = lua["v"];
	bool val = lua["val"];
	// enforce reference semantics
	REQUIRE(std::addressof(w.b) == std::addressof(x));
	REQUIRE(n.f == 25);
	REQUIRE(v == 25);
	REQUIRE(val);

	{
		auto result = lua.safe_script("f(n, 50)", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
	{
		auto result = lua.safe_script("o.n = 25", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
}
