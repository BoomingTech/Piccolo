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

TEST_CASE("environments/this_environment", "test various situations of pulling out an environment") {
	static std::string code = "return (f(10))";

	sol::state lua;
	sol::stack_guard luasg(lua);

	lua["f"] = [](sol::this_environment te, int x, sol::this_state ts) {
		if (te) {
			sol::environment& env = te;
			return x + static_cast<int>(env["x"]);
		}
		sol::state_view lua = ts;
		return x + static_cast<int>(lua["x"]);
	};

	sol::environment e(lua, sol::create, lua.globals());
	lua["x"] = 5;
	e["x"] = 20;
	SECTION("from Lua script") {
		int value = 0;
		{
			auto result1 = lua.safe_script(code, e, sol::script_pass_on_error);
			REQUIRE(result1.valid());
			value = result1;
		}
		REQUIRE(value == 30);
	}
	SECTION("from C++") {
		sol::function f = lua["f"];
		{
			sol::stack_guard luasg_env(lua);
			bool env_set = e.set_on(f);
			REQUIRE(env_set);
		}
		int value = f(10);
		REQUIRE(value == 30);
	}
	SECTION("from C++, with no env") {
		sol::function f = lua["f"];
		int value = f(10);
		REQUIRE(value == 15);
	}
}

TEST_CASE("environment/this_environment/nested_calls", "Test that this_environment actually finds the right environment in specific nested Lua call contexts") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("print_no_env", [](int x) { REQUIRE(x == 1); });

	lua.set_function("print_this_env", [](int x, sol::this_environment) { REQUIRE(x == 2); });

	sol::protected_function_result result = lua.safe_script(R"(
		local var = nil

		function test_1_no_env()
			var = 1
			print_no_env(1)
		end

		function test_2_this_env()
			var = 1
			print_this_env(2)
		end

		test_1_no_env() -- should be ok
		test_2_this_env() -- should be okay
	)",
	     sol::script_pass_on_error);

	REQUIRE(result.valid());
}
