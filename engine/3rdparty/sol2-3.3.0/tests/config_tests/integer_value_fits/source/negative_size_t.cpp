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

#include <cstdint>
#include <limits>

inline namespace sol2_tests_negative_size_t {
	static inline size_t npos_like_api() {
		return size_t(-1);
	}
} // namespace sol2_tests_negative_size_t

TEST_CASE("numeric/negative size_t", "handle negative integers casted to size_t values") {
#if SOL_LUA_VERSION_I_ > 502
	// For 5.3 and above, this can fit using the negative-cast method
	sol::state lua;
	lua.set_function("f", &npos_like_api);

	auto result = lua.safe_script("v = f()", sol::script_pass_on_error);
	sol::optional<sol::error> maybe_error = result;
	REQUIRE(result.valid());
	REQUIRE(result.status() == sol::call_status::ok);
	REQUIRE_FALSE(maybe_error.has_value());
	size_t should_be_like_npos = lua["v"];
	REQUIRE(should_be_like_npos == size_t(-1));
#elif SOL_LUA_VERSION_I_ <= 502
	// For 5.2 and below, this will trigger an error if checking is on
	sol::state lua;
	lua.set_function("f", &npos_like_api);

	auto result = lua.safe_script("v = f()", sol::script_pass_on_error);
	sol::optional<sol::error> maybe_error = result;
	REQUIRE_FALSE(result.valid());
	REQUIRE(result.status() != sol::call_status::ok);
	REQUIRE(result.status() == sol::call_status::runtime);
	REQUIRE(maybe_error.has_value());
#endif
}
