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

inline namespace sol2_tests_environments_get {
	struct check_g_env {
		sol::state* plua;
		sol::environment* penv_g;

		check_g_env(sol::state& lua, sol::environment& env_g) : plua(&lua), penv_g(&env_g) {
		}

		void operator()(sol::function target) const {
			sol::state& lua = *plua;
			sol::environment& env_g = *penv_g;
			sol::stack_guard luasg(lua);
			sol::environment target_env = sol::get_environment(target);
			int test_env_g = env_g["test"];
			int test_target_env = target_env["test"];
			REQUIRE(test_env_g == test_target_env);
			REQUIRE(test_env_g == 5);
			REQUIRE(env_g == target_env);
		}
	};

	struct check_f_env {
		sol::state* plua;
		sol::environment* penv_f;

		check_f_env(sol::state& lua, sol::environment& env_f) : plua(&lua), penv_f(&env_f) {
		}

		void operator()(sol::function target) const {
			sol::state& lua = *plua;
			sol::environment& env_f = *penv_f;
			sol::stack_guard luasg(lua);
			{
				sol::environment target_env(sol::env_key, target);
				int test_env_f = env_f["test"];
				int test_target_env = target_env["test"];
				REQUIRE(test_env_f == test_target_env);
				REQUIRE(test_env_f == 31);
				REQUIRE(env_f == target_env);
			}
		}
	};

	struct check_h_env {
		sol::state* plua;

		check_h_env(sol::state& lua) : plua(&lua) {
		}

		void operator()(sol::function target) const {
			sol::state& lua = *plua;
			sol::stack_guard luasg(lua);
			sol::environment target_env = sol::get_environment(target);
			// cannot strictly test
			// if it's the global table, because different lua runtimes
			// give different envs when there is no env
		}
	};
} // namespace sol2_tests_environments_get

TEST_CASE("environments/get", "Envronments can be taken out of things like Lua functions properly") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.open_libraries(sol::lib::base);

	{
		auto result1 = lua.safe_script("f = function() return test end", sol::script_pass_on_error);
		REQUIRE(result1.valid());
	}
	sol::function f = lua["f"];

	sol::environment env_f(lua, sol::create);
	env_f["test"] = 31;
	{
		sol::stack_guard luasgf(lua);
		bool env_f_was_set = sol::set_environment(env_f, f);
		REQUIRE(env_f_was_set);
	}

	int result = f();
	REQUIRE(result == 31);

	{
		auto result2 = lua.safe_script("g = function() test = 5 end", sol::script_pass_on_error);
		REQUIRE(result2.valid());
	}
	sol::function g = lua["g"];
	sol::environment env_g(lua, sol::create);
	bool env_g_was_set = env_g.set_on(g);
	REQUIRE(env_g_was_set);

	g();

	int test = env_g["test"];
	REQUIRE(test == 5);

	sol::object global_test = lua["test"];
	REQUIRE(!global_test.valid());

	{
		auto result3 = lua.safe_script("h = function() end", sol::script_pass_on_error);
		REQUIRE(result3.valid());
	}

	lua.set_function("check_f_env", check_f_env(lua, env_f));
	lua.set_function("check_g_env", check_g_env(lua, env_g));
	lua.set_function("check_h_env", check_h_env(lua));

	{
		auto checkf = lua.safe_script("check_f_env(f)", sol::script_pass_on_error);
		REQUIRE(checkf.valid());
		auto checkg = lua.safe_script("check_g_env(g)", sol::script_pass_on_error);
		REQUIRE(checkg.valid());
		auto checkh = lua.safe_script("check_h_env(h)", sol::script_pass_on_error);
		REQUIRE(checkh.valid());
	}
}
