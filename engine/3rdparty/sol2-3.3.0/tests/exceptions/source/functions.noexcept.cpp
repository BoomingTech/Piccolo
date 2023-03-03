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

inline namespace sol2_tests_exceptions_functions_noexcept {

	struct T {
		static int noexcept_function() noexcept {
			return 0x61;
		}

		int noexcept_method() noexcept {
			return 0x62;
		}
	};

	static int raw_noexcept_function(lua_State* L) noexcept {
		return sol::stack::push(L, 0x63);
	}

} // namespace sol2_tests_exceptions_functions_noexcept

TEST_CASE("exceptions/functions/noexcept", "allow noexcept functions to be serialized properly into Lua using sol2") {
	T t {};

	lua_CFunction ccall = sol::c_call<decltype(&raw_noexcept_function), &raw_noexcept_function>;

	sol::state lua;

	auto check_script_call = [&lua](std::string script) {
		auto result = lua.safe_script(script, &sol::script_pass_on_error);
		sol::optional<sol::error> maybe_err = result;
		REQUIRE(result.valid());
		REQUIRE(result.status() == sol::call_status::ok);
		REQUIRE_FALSE(maybe_err.has_value());
	};

	lua.set_function("f", &T::noexcept_function);
	lua.set_function("g", &T::noexcept_method);
	lua.set_function("h", &T::noexcept_method, T());
	lua.set_function("i", &T::noexcept_method, std::ref(t));
	lua.set_function("j", &T::noexcept_method, &t);
	lua.set_function("k", &T::noexcept_method, t);
	lua.set_function("l", &raw_noexcept_function);
	lua.set_function("m", ccall);

	lua["t"] = T();
	check_script_call("v1 = f()");
	check_script_call("v2 = g(t)");
	check_script_call("v3 = h()");
	check_script_call("v4 = i()");
	check_script_call("v5 = j()");
	check_script_call("v6 = k()");
	check_script_call("v7 = l()");
	check_script_call("v8 = m()");
	int v1 = lua["v1"];
	int v2 = lua["v2"];
	int v3 = lua["v3"];
	int v4 = lua["v4"];
	int v5 = lua["v5"];
	int v6 = lua["v6"];
	int v7 = lua["v7"];
	int v8 = lua["v8"];
	REQUIRE(v1 == 0x61);
	REQUIRE(v2 == 0x62);
	REQUIRE(v3 == 0x62);
	REQUIRE(v4 == 0x62);
	REQUIRE(v5 == 0x62);
	REQUIRE(v6 == 0x62);
	REQUIRE(v7 == 0x63);
	REQUIRE(v8 == 0x63);
}
