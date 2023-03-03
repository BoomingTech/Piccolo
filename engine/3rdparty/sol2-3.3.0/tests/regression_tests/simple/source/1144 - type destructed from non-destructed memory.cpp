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

#include <iostream>
#include <sol/sol.hpp>

inline namespace sol2_regression_test_1144 {
	class MyClass {
	private:
		std::string m_name;

	public:
		MyClass(const std::string& name) : m_name(name) {};
	};

	void bind_class(sol::state_view state) {
		sol::usertype<MyClass> bind_my_class = state.new_usertype<MyClass>("MyClass", sol::call_constructor, sol::constructors<MyClass(std::string)>());
	}
} // namespace sol2_regression_test_1144

TEST_CASE("Test for Issue #1144 -", "[sol2][regression][Issue-1144]") {
	sol::state state;
	bind_class(state);
	sol::protected_function_result pr = state.do_string("local a = MyClass();");
	REQUIRE_FALSE(pr.valid());
	sol::error err = pr;
	std::cout << "An error occurred, as expected:\n" << err.what() << std::endl;
}
