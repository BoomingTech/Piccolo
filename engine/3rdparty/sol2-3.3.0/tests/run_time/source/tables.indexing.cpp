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

int plop_xyz(int x, int y, std::string z) {
	INFO(x << " " << y << " " << z);
	return 11;
}

TEST_CASE("tables/operator[]", "Check if operator[] retrieval and setting works properly") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.safe_script("foo = 20\nbar = \"hello world\"");
	// basic retrieval
	std::string bar = lua["bar"];
	int foo = lua["foo"];
	REQUIRE(bar == "hello world");
	REQUIRE(foo == 20);
	// test operator= for stringification
	// errors due to ambiguous operators
	bar = lua["bar"];

	// basic setting
	lua["bar"] = 20.4;
	lua["foo"] = "goodbye";

	// doesn't modify the actual values obviously.
	REQUIRE(bar == "hello world");
	REQUIRE(foo == 20);

	// function setting
	lua["test"] = plop_xyz;
	{
		auto result = lua.safe_script("assert(test(10, 11, \"hello\") == 11)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}

	// function retrieval
	sol::function test = lua["test"];
	REQUIRE(test.call<int>(10, 11, "hello") == 11);

	// setting a lambda
	lua["lamb"] = [](int x) { return x * 2; };

	{
		auto result = lua.safe_script("assert(lamb(220) == 440)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}

	// function retrieval of a lambda
	sol::function lamb = lua["lamb"];
	REQUIRE(lamb.call<int>(220) == 440);

	// test const table retrieval
	auto assert1 = [](const sol::table& t) {
		std::string a = t["foo"];
		double b = t["bar"];
		REQUIRE(a == "goodbye");
		REQUIRE(b == 20.4);
	};

	REQUIRE_NOTHROW(assert1(lua.globals()));
}

TEST_CASE("tables/operator[] valid", "Test if proxies on tables can lazily evaluate validity") {
	sol::state lua;
	bool isFullScreen = false;
	auto fullscreennopers = lua["fullscreen"]["nopers"];
	auto fullscreen = lua["fullscreen"];
	REQUIRE_FALSE(fullscreennopers.valid());
	REQUIRE_FALSE(fullscreen.valid());

	lua["fullscreen"] = true;

	REQUIRE_FALSE(fullscreennopers.valid());
	REQUIRE(fullscreen.valid());
	isFullScreen = lua["fullscreen"];
	REQUIRE(isFullScreen);

	lua["fullscreen"] = false;
	REQUIRE_FALSE(fullscreennopers.valid());
	REQUIRE(fullscreen.valid());
	isFullScreen = lua["fullscreen"];
	REQUIRE_FALSE(isFullScreen);
}

TEST_CASE("tables/operator[] optional", "Test if proxies on tables can lazily evaluate validity") {
	sol::state lua;

	sol::optional<int> test1 = lua["no_exist_yet"];
	bool present = (bool)test1;
	REQUIRE_FALSE(present);

	lua["no_exist_yet"] = 262;
	sol::optional<int> test2 = lua["no_exist_yet"];
	present = (bool)test2;
	REQUIRE(present);
	REQUIRE(test2.value() == 262);

	sol::optional<int> nope = lua["nope"]["kek"]["hah"];
	auto nope2 = lua.get<sol::optional<int>>(std::make_tuple("nope", "kek", "hah"));
	present = (bool)nope;
	REQUIRE_FALSE(present);
	present = (bool)nope2;
	REQUIRE_FALSE(present);
	lua.create_named_table("nope", "kek", lua.create_table_with("hah", 1));
	sol::optional<int> non_nope = lua["nope"]["kek"]["hah"].get<sol::optional<int>>();
	sol::optional<int> non_nope2 = lua.get<sol::optional<int>>(std::make_tuple("nope", "kek", "hah"));
	present = (bool)non_nope;
	REQUIRE(present);
	present = (bool)non_nope2;
	REQUIRE(present);
	REQUIRE(non_nope.value() == 1);
	REQUIRE(non_nope2.value() == 1);

	INFO("Keys: nope, kek, hah");
	lua.set(std::make_tuple("nope", "kek", "hah"), 35);
	sol::optional<int> non_nope3 = lua["nope"]["kek"]["hah"].get<sol::optional<int>>();
	sol::optional<int> non_nope4 = lua.get<sol::optional<int>>(std::make_tuple("nope", "kek", "hah"));
	present = (bool)non_nope3;
	REQUIRE(present);
	present = (bool)non_nope4;
	REQUIRE(present);
	REQUIRE(non_nope3.value() == 35);
	REQUIRE(non_nope4.value() == 35);
}
