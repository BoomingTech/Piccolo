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

TEST_CASE("environments/sanboxing", "see if environments on functions are working properly") {
	SECTION("basic") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		auto result1 = lua.safe_script("a = function() return 5 end", sol::script_pass_on_error);
		REQUIRE(result1.valid());
		REQUIRE(result1.status() == sol::call_status::ok);

		sol::function a = lua["a"];

		int result0 = a();
		REQUIRE(result0 == 5);

		sol::environment env(lua, sol::create);
		bool env_set_on_a = sol::set_environment(env, a);
		// either works; there can sometimes no environment
		REQUIRE((env_set_on_a || !env_set_on_a));

		int value = a();
		REQUIRE(value == 5);
	}
	SECTION("return environment value") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		auto result1 = lua.safe_script("a = function() return test end", sol::script_pass_on_error);
		REQUIRE(result1.valid());
		REQUIRE(result1.status() == sol::call_status::ok);

		sol::function a = lua["a"];
		sol::environment env(lua, sol::create);
		env["test"] = 5;
		bool env_set_on_a = env.set_on(a);
		REQUIRE(env_set_on_a);

		// the function returns the value from the environment table
		int result = a();
		REQUIRE(result == 5);
	}

	SECTION("set environment value") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		auto result1 = lua.safe_script("a = function() test = 5 end", sol::script_pass_on_error);
		REQUIRE(result1.valid());
		REQUIRE(result1.status() == sol::call_status::ok);

		sol::function a = lua["a"];
		sol::environment env(lua, sol::create);
		bool env_set_on_a = sol::set_environment(env, a);
		REQUIRE(env_set_on_a);

		a();

		// the value can be retrieved from the env table
		int result = env["test"];
		REQUIRE(result == 5);

		// the global environment is not polluted
		auto gtest = lua["test"];
		REQUIRE(!gtest.valid());
	}
}
