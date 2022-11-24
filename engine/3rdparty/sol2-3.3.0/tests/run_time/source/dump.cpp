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

#include <list>
#include <vector>
#include <deque>

int dump_always_fail_number = -32;

int dump_always_fail(lua_State*, const void*, size_t, void*) {
	return dump_always_fail_number;
}

TEST_CASE("dump/dump transfer", "test that a function can be transferred from one place to another") {
	SECTION("safe") {
		sol::state lua;
		sol::state lua2;
		lua2.open_libraries(sol::lib::base);

		sol::load_result lr = lua.load("a = function (v) print(v) return v end");
		REQUIRE(lr.valid());
		sol::protected_function target = lr.get<sol::protected_function>();
		sol::bytecode target_bc = target.dump();

		auto result2 = lua2.safe_script(target_bc.as_string_view(), sol::script_pass_on_error);
		REQUIRE(result2.valid());
		sol::protected_function pf = lua2["a"];
		int v = pf(25557);
		REQUIRE(v == 25557);
	}
	SECTION("unsafe") {
		sol::state lua;
		sol::state lua2;
		lua2.open_libraries(sol::lib::base);

		sol::load_result lr = lua.load("a = function (v) print(v) return v end");
		REQUIRE(lr.valid());
		sol::unsafe_function target = lr;
		sol::bytecode target_bc = target.dump();

		auto result2 = lua2.safe_script(target_bc.as_string_view(), sol::script_pass_on_error);
		REQUIRE(result2.valid());
		sol::unsafe_function pf = lua2["a"];
		int v = pf(25557);
		REQUIRE(v == 25557);
	}
}

TEST_CASE("dump/failure", "test that failure is properly propagated") {
	SECTION("safe") {
		sol::state lua;
		sol::load_result lr = lua.load("a = function (v) print(v) return v end");
		REQUIRE(lr.valid());
		sol::protected_function target = lr.get<sol::protected_function>();
		int err = target.dump(&dump_always_fail, nullptr, false, sol::dump_pass_on_error);
		REQUIRE(err == dump_always_fail_number);
	}
	SECTION("unsafe") {
		sol::state lua;
		sol::load_result lr = lua.load("a = function (v) print(v) return v end");
		REQUIRE(lr.valid());
		sol::unsafe_function target = lr;
		int err = target.dump(&dump_always_fail, nullptr, false, sol::dump_pass_on_error);
		REQUIRE(err == dump_always_fail_number);
	}
}

TEST_CASE("dump/different containers", "test that dump inserter works for various kinds of containers") {
	SECTION("safe") {
		sol::state lua;

		sol::load_result lr = lua.load("a = function (v) print(v) return v end");
		REQUIRE(lr.valid());
		sol::protected_function target = lr.get<sol::protected_function>();
		sol::bytecode bytecode_dump = target.dump();
		std::list<std::byte> list_dump = target.dump<std::list<std::byte>>();
		std::vector<std::byte> vector_dump = target.dump<std::vector<std::byte>>();
		std::deque<std::byte> deque_dump = target.dump<std::deque<std::byte>>();
		REQUIRE(std::equal(bytecode_dump.cbegin(), bytecode_dump.cend(), vector_dump.cbegin(), vector_dump.cend()));
		REQUIRE(std::equal(bytecode_dump.cbegin(), bytecode_dump.cend(), list_dump.cbegin(), list_dump.cend()));
		REQUIRE(std::equal(bytecode_dump.cbegin(), bytecode_dump.cend(), deque_dump.cbegin(), deque_dump.cend()));
	}
	SECTION("unsafe") {
		sol::state lua;

		sol::load_result lr = lua.load("a = function (v) print(v) return v end");
		REQUIRE(lr.valid());
		sol::unsafe_function target = lr;
		sol::bytecode bytecode_dump = target.dump();
		std::list<std::byte> list_dump = target.dump<std::list<std::byte>>();
		std::vector<std::byte> vector_dump = target.dump<std::vector<std::byte>>();
		std::deque<std::byte> deque_dump = target.dump<std::deque<std::byte>>();
		REQUIRE(std::equal(bytecode_dump.cbegin(), bytecode_dump.cend(), vector_dump.cbegin(), vector_dump.cend()));
		REQUIRE(std::equal(bytecode_dump.cbegin(), bytecode_dump.cend(), list_dump.cbegin(), list_dump.cend()));
		REQUIRE(std::equal(bytecode_dump.cbegin(), bytecode_dump.cend(), deque_dump.cbegin(), deque_dump.cend()));
	}
}
