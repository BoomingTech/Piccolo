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

#include <catch2/catch_all.hpp>

#include <iostream>

void test_free_func(std::function<void()> f) {
	f();
}

void test_free_func2(std::function<int(int)> f, int arg1) {
	int val = f(arg1);
	REQUIRE(val == arg1);
}

std::function<int()> makefn() {
	auto fx = []() -> int { return 0x1456789; };
	return fx;
}

void takefn(std::function<int()> purr) {
	if (purr() != 0x1456789)
		throw 0;
}

TEST_CASE("functions/empty std functions", "std::function is allowed to be empty, so it should be serialized to nil") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	std::function<void()> foo = nullptr;
	sol::function bar;

	lua["Foo"] = foo;
	lua["Bar"] = bar;

	sol::optional<sol::error> result = lua.safe_script(R"SCR(
	if Bar ~= nil
	then
		Bar()
	end

	if Foo ~= nil or type(Foo) == 'function'
	then
		Foo()
	end
	)SCR",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(result.has_value());

	sol::optional<sol::error> result0 = lua.safe_script(R"SCR(Foo())SCR", sol::script_pass_on_error);
	REQUIRE(result0.has_value());
	sol::optional<sol::error> result1 = lua.safe_script(R"SCR(Bar())SCR", sol::script_pass_on_error);
	REQUIRE(result1.has_value());
}

TEST_CASE("functions/sol::function to std::function", "check if conversion to std::function works properly and calls with correct arguments") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.open_libraries(sol::lib::base);

	lua.set_function("testFunc", test_free_func);
	lua.set_function("testFunc2", test_free_func2);
	auto result1 = lua.safe_script("testFunc(function() print(\"hello std::function\") end)", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	{
		auto result = lua.safe_script(
		     "function m(a)\n"
		     "     print(\"hello std::function with arg \", a)\n"
		     "     return a\n"
		     "end\n"
		     "\n"
		     "testFunc2(m, 1)",
		     sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
}

TEST_CASE("functions/returning functions from C++", "check to see if returning a functor and getting a functor from lua is possible") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("makefn", makefn);
	lua.set_function("takefn", takefn);
	{
		auto result = lua.safe_script(
		     "afx = makefn()\n"
		     "print(afx())\n"
		     "takefn(afx)\n",
		     sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
}

TEST_CASE("functions/set_function already wrapped", "setting a function returned from Lua code that is already wrapped into a sol::function or similar") {
	SECTION("test different types") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		sol::function fn = lua.safe_script("return function() return 5 end");
		sol::protected_function pfn = fn;
		std::function<int()> sfn = fn;

		lua.set_function("test", fn);
		lua.set_function("test2", pfn);
		lua.set_function("test3", sfn);

		{
			auto result = lua.safe_script("assert(type(test) == 'function')", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test() ~= nil)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test() == 5)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}

		{
			auto result = lua.safe_script("assert(type(test2) == 'function')", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test2() ~= nil)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test2() == 5)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}

		{
			auto result = lua.safe_script("assert(type(test3) == 'function')", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test3() ~= nil)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test3() == 5)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
	}

	SECTION("getting the value from C++") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		sol::function fn = lua.safe_script("return function() return 5 end");

		int result = fn();
		REQUIRE(result == 5);
	}

	SECTION("setting the function directly") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		sol::function fn = lua.safe_script("return function() return 5 end");

		lua.set_function("test", fn);

		{
			auto result = lua.safe_script("assert(type(test) == 'function')", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test() ~= nil)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test() == 5)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
	}

	SECTION("does the function actually get executed?") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		sol::function fn2 = lua.safe_script("return function() print('this was executed') end");
		lua.set_function("test", fn2);

		{
			auto result = lua.safe_script("assert(type(test) == 'function')", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("test()", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
	}

	SECTION("setting the function indirectly, with the return value cast explicitly") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		sol::function fn = lua.safe_script("return function() return 5 end");

		lua.set_function("test", [&fn]() { return fn.call<int>(); });

		{
			auto result = lua.safe_script("assert(type(test) == 'function')", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test() ~= nil)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(test() == 5)", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
	}
}
