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

inline namespace sol2_tests_config_function_pointers_retrieval {

	inline constexpr int magic_value = 24;

	using zero_arg_type = int (*)();
	using one_arg_type = int (*)(int);
	using callback_type = int (*)(one_arg_type);

	int free_function(int value) {
		return value + 1;
	}

	int callback(one_arg_type f) {
		return f(magic_value) + 1;
	}
} // namespace sol2_tests_config_function_pointers_retrieval

TEST_CASE("config/function_pointers/get", "retrieving a function pointer type can round trip, even if we lose safety in other contexts") {
	constexpr int expected_value = magic_value;
	constexpr int expected_free_function_value = magic_value + 1;
	constexpr int expected_callback_value = magic_value + 1 + 1;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	auto lambda = []() { return magic_value; };
	auto lambda_ptr = static_cast<zero_arg_type>(lambda);

	lua["magic_value"] = magic_value;
	lua["expected_value"] = expected_value;
	lua["expected_free_function_value"] = expected_free_function_value;
	lua["expected_callback_value"] = expected_callback_value;

	lua["lambda"] = sol::as_function_reference(lambda);
	lua["lambda_ptr"] = lambda_ptr;
	lua["free_function"] = &free_function;
	lua["callback"] = &callback;

	zero_arg_type lambda_f = lua["lambda"];
	zero_arg_type lambda_ptr_f = lua["lambda_ptr"];
	one_arg_type free_function_f = lua["free_function"];
	callback_type callback_f = lua["callback"];
	sol::function lua_callback_f = lua["callback"];

	int lambda_f_result = lambda_f();
	int lambda_ptr_f_result = lambda_ptr_f();
	int free_function_f_result = free_function_f(magic_value);
	int callback_f_result = callback_f(&free_function);
	int lua_callback_f_result = lua_callback_f(&free_function);
	REQUIRE(lambda_f_result == expected_value);
	REQUIRE(lambda_ptr_f_result == expected_value);
	REQUIRE(free_function_f_result == expected_free_function_value);
	REQUIRE(callback_f_result == expected_callback_value);
	REQUIRE(lua_callback_f_result == expected_callback_value);

	const char code[] = R"(
assert(lambda() == expected_value)
assert(lambda_ptr() == expected_value)
assert(free_function(magic_value) == expected_free_function_value)
assert(callback(free_function) == expected_callback_value)
	)";

	auto result = lua.safe_script(code, sol::script_pass_on_error);
	sol::optional<sol::error> maybe_err = result;
	REQUIRE(result.valid());
	REQUIRE(!maybe_err.has_value());
}
