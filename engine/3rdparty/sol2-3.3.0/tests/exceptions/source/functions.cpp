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

#include <stdexcept>
#include <string_view>

inline namespace sol2_tests_exceptions_functions {
	inline constexpr const std::string_view special_string = "Whoopsie [expected]";

	static void func_throw() {
		throw std::runtime_error(special_string.data());
	}

	struct some_class {
		void mem_func_throw() {
			throw std::runtime_error(special_string.data());
		}
	};
} // namespace sol2_tests_exceptions_functions

TEST_CASE("exceptions/functions", "exercise the ability to throw exceptions many different function binding code paths") {
	sol::state lua;
	lua.open_libraries(sol::lib::base,
	     sol::lib::package,
	     sol::lib::coroutine,
	     sol::lib::string,
	     sol::lib::os,
	     sol::lib::math,
	     sol::lib::table,
	     sol::lib::io,
	     sol::lib::debug);

	[[maybe_unused]] int a = 10;
	some_class sc {};
	auto throw_action = [](sol::state& lua, bool use_sc) {
		if (use_sc) {
			auto res = lua.safe_script("func_throw(sc)", sol::script_pass_on_error);
			REQUIRE_FALSE(res.valid());
			REQUIRE(res.status() == sol::call_status::runtime);
			sol::error err = res.get<sol::error>();
			std::string_view err_what(err.what());
			REQUIRE(err_what.find(special_string) != std::string::npos);
		}
		else {
			auto res = lua.safe_script("func_throw()", sol::script_pass_on_error);
			REQUIRE_FALSE(res.valid());
			REQUIRE(res.status() == sol::call_status::runtime);
			sol::error err = res.get<sol::error>();
			std::string_view err_what(err.what());
			REQUIRE(err_what.find(special_string) != std::string::npos);
		}
	};

	lua["sc"] = &sc;

	SECTION("proxy") {
		lua["func_throw"] = sol::property(&func_throw, &some_class::mem_func_throw);
		REQUIRE_NOTHROW(throw_action(lua, false));
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua["func_throw"] = sol::property(&some_class::mem_func_throw);
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua["func_throw"] = sol::readonly_property(func_throw);
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = sol::writeonly_property(&some_class::mem_func_throw);
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua["func_throw"] = [f = &func_throw] { return f(); };
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = &func_throw;
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = [] { return func_throw(); };
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = [a]() { (void)a; return func_throw(); };
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = [&sc]() { return sc.mem_func_throw(); };
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = &some_class::mem_func_throw;
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua["func_throw"] = std::function<void()>([f = &func_throw] { return f(); });
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = sol::overload(std::function<void()>([f = &func_throw] { return f(); }));
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = std::function<void()>([] { return func_throw(); });
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = std::function<void()>(&func_throw);
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua["func_throw"] = std::function<void(some_class&)>(&some_class::mem_func_throw);
		REQUIRE_NOTHROW(throw_action(lua, true));
	}

	SECTION("set_function") {
		lua.set_function("func_throw", sol::property(&func_throw, &some_class::mem_func_throw));
		REQUIRE_NOTHROW(throw_action(lua, false));
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua.set_function("func_throw", sol::property(&some_class::mem_func_throw));
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua.set_function("func_throw", sol::readonly_property(func_throw));
		REQUIRE_NOTHROW(throw_action(lua, false));

		// TODO: better handling of no_prop in this case here
		// lua.set_function("func_throw", sol::writeonly_property(&some_class::mem_func_throw));
		// REQUIRE_NOTHROW(throw_action(lua, true));

		lua.set_function("func_throw", [f = &func_throw] { return f(); });
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", &func_throw);
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", [] { return func_throw(); });
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", [a]() { (void)a; return func_throw(); });
		REQUIRE_NOTHROW(throw_action(lua, false));

		// TODO: this should work at some point, yeah?
		/*
		lua.set_function(
		     "func_throw", [](some_class& sc) { return sc.mem_func_throw(); }, sc);
		REQUIRE_NOTHROW(throw_action(lua, false));
		*/

		lua.set_function("func_throw", [&sc]() { return sc.mem_func_throw(); });
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", &some_class::mem_func_throw);
		REQUIRE_NOTHROW(throw_action(lua, true));

		lua.set_function("func_throw", std::function<void()>([f = &func_throw] { return f(); }));
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", sol::overload(std::function<void()>([f = &func_throw] { return f(); })));
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", std::function<void()>([] { return func_throw(); }));
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", std::function<void()>(&func_throw));
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", &some_class::mem_func_throw, &sc);
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", &some_class::mem_func_throw, sc);
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", &some_class::mem_func_throw, std::ref(sc));
		REQUIRE_NOTHROW(throw_action(lua, false));

		lua.set_function("func_throw", std::function<void(some_class&)>(&some_class::mem_func_throw));
		REQUIRE_NOTHROW(throw_action(lua, true));
	}
}
