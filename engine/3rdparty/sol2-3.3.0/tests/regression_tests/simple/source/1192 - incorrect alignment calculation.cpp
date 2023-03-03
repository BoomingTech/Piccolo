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

#if SOL_TESTS_SIZEOF_VOID_P == 4

inline namespace sol2_regression_test_1192 {
	struct Test {
		std::uint64_t dummy;
	};

	struct alignas(1024) Test2 {
		char dummy[1024];
	};
} // namespace sol2_regression_test_1192

TEST_CASE("Test for Issue #1192 - alignment test should not fail for strangely-aligned / over-aligned objects", "[sol2][regression][Issue-1192]") {
	sol::state lua;

	static_assert(sizeof(Test) == 8);
	static_assert(alignof(Test) == 8);
	static_assert(sizeof(Test*) == 4);
	static_assert(alignof(Test*) == 4);

	/// [sol2] An error occurred and panic has been invoked: aligned allocation of userdata block (data section) for 'sol2_regression_test_1192::Test'
	/// failed Note: may not panic depending on alignment of local variable `alignment_shim` in sol::detail::aligned_space_for
	lua["test"] = Test {};

	// Test also unique and over-aligned userdata
	lua["test"] = std::make_unique<Test>();
	lua["test"] = Test2 {};

	return 0;
}

#endif
