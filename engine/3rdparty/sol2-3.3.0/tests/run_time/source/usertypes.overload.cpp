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

#include "sol_test.hpp"

#include "common_classes.hpp"

#include <catch2/catch_all.hpp>

inline namespace sol2_test_usertypes_overload {

	struct overloading_test {
		int print(int i) {
			INFO("Integer print: " << i);
			return 500 + i;
		}
		int print() {
			INFO("No param print.");
			return 500;
		}
	};

	class TestClass1 {
	public:
		TestClass1(void) {
		}
		~TestClass1(void) {
		}

	public:
		void* Get(const uint32_t) {
			return this;
		}
		void* Get(const std::string_view&) {
			return this;
		}
	};

	class TestClass2 {
	public:
		TestClass2(void) {
		}
		~TestClass2(void) {
		}

	public:
		void* Get(const uint32_t) {
			return this;
		}
		void* Get(const std::string_view&) {
			return this;
		}
	};

	sol::object lua_TestClass2_GetByIndex(const sol::this_state& s, TestClass2* tc, const uint32_t) {
		return sol::make_object(s, tc);
	}
	sol::object lua_TestClass2_GetByName(const sol::this_state& s, TestClass2* tc, const std::string_view&) {
		return sol::make_object(s, tc);
	}
} // namespace sol2_test_usertypes_overload

TEST_CASE("usertype/overloading", "Check if overloading works properly for usertypes") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<woof>("woof", "var", &woof::var, "func", sol::overload(&woof::func, &woof::func2, &woof::func2s));

	const std::string bark_58 = "bark 58";

	REQUIRE_NOTHROW(
	     lua.safe_script("r = woof:new()\n"
	                     "a = r:func(1)\n"
	                     "b = r:func(1, 2)\n"
	                     "c = r:func(58, 'bark')\n"));
	REQUIRE((lua["a"] == 1));
	REQUIRE((lua["b"] == 3.5));
	REQUIRE((lua["c"] == bark_58));
	auto result = lua.safe_script("r:func(1,2,'meow')", sol::script_pass_on_error);
	REQUIRE_FALSE(result.valid());
	std::cout << "----- end of 7" << std::endl;
}

TEST_CASE("usertype/overloading_values", "ensure overloads handle properly") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.new_usertype<overloading_test>("overloading_test",
	     sol::constructors<>(),
	     "print",
	     sol::overload(
	          static_cast<int (overloading_test::*)(int)>(&overloading_test::print), static_cast<int (overloading_test::*)()>(&overloading_test::print)),
	     "print2",
	     sol::overload(
	          static_cast<int (overloading_test::*)()>(&overloading_test::print), static_cast<int (overloading_test::*)(int)>(&overloading_test::print)));
	lua.set("test", overloading_test());

	sol::function f0_0 = lua.load("return test:print()");
	sol::function f0_1 = lua.load("return test:print2()");
	sol::function f1_0 = lua.load("return test:print(24)");
	sol::function f1_1 = lua.load("return test:print2(24)");
	int res = f0_0();
	int res2 = f0_1();
	int res3 = f1_0();
	int res4 = f1_1();

	REQUIRE(res == 500);
	REQUIRE(res2 == 500);

	REQUIRE(res3 == 524);
	REQUIRE(res4 == 524);
	std::cout << "----- end of 8" << std::endl;
}

TEST_CASE("usertypes/overloading with transparent arguments", "ensure transparent arguments bound with usertypes don't change arity counts in overloads") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	auto t1 = lua.new_usertype<TestClass1>(u8"TestClass1", sol::no_constructor);
	t1.set_function(
	     u8"Get", sol::overload(sol::resolve<void*(const uint32_t)>(&TestClass1::Get), sol::resolve<void*(const std::string_view&)>(&TestClass1::Get)));

	auto t2 = lua.new_usertype<TestClass2>(u8"TestClass2", sol::no_constructor);
	t2.set_function(u8"Get", sol::overload(lua_TestClass2_GetByIndex, lua_TestClass2_GetByName));

	TestClass1 test1;
	TestClass2 test2;
	lua["testObj1"] = test1;
	lua["testObj2"] = test2;

	sol::optional<sol::error> maybe_error = lua.safe_script(R"(
        local test1 = testObj1:Get(0);
        local test2 = testObj1:Get('test');
        assert(test1 ~= nil, 'test1 failed');
        assert(test2 ~= nil, 'test2 failed');
        print('test1 ok!');
        
        local test3 = testObj2:Get(0);
        local test4 = testObj2:Get('test');
        assert(test3 ~= nil, 'test3 failed');
        assert(test4 ~= nil, 'test4 failed');
        print('test2 ok!');
	)",
	     &sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error.has_value());
}
