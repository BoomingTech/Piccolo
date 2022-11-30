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

#include <iostream>
#include <string>

inline namespace sol2_regression_test_1096 {
	const double constant_float_v = 3;
	double test_value() {
		return constant_float_v;
	}
} // namespace sol2_regression_test_1096

TEST_CASE("Test for Issue #1096 - checking different functions/lambdas/structures bind as intendedcorrectly", "[sol2][regression][Issue-1096]") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua["test0"] = lua.create_table_with("value", [] { return constant_float_v; });
	lua["test1"] = lua.create_table_with("value", [] { return constant_float_v; });
	lua["test2"] = lua.create_table_with("value", [&] { return constant_float_v; });
	lua["test3"] = lua.create_table_with("value", test_value);
	lua["test4"] = lua.create_table_with("value", &test_value);
	lua["test5"] = sol::create;
	lua["test6"] = sol::create;
	lua["test7"] = sol::create;
	lua["test5"]["value"] = [] { return constant_float_v; };
	lua["test6"]["value"] = [] { return constant_float_v; };
	lua["test7"]["value"] = [&] { return constant_float_v; };

	unsigned int accumulated_errors = 0;
	for (std::size_t i = 0; i < 8; ++i) {
		std::string num = std::to_string(i);
		std::string val = std::to_string(constant_float_v);
		std::string code = "local val = test" + num + ".value()\nassert(val == " + val + ")";
		sol::optional<sol::error> maybe_error = lua.safe_script(code, sol::script_pass_on_error);
		if (maybe_error.has_value()) {
			const sol::error& err = *maybe_error;
			std::cerr << "regression 1096: nested function call using test" << i << " failed:\n" << err.what() << std::endl;
			++accumulated_errors;
		}
	}
	REQUIRE(accumulated_errors == 0);
}
