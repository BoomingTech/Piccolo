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

#include <iterator>
#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <deque>
#include <array>
#include <unordered_map>
#include <set>
#include <unordered_set>

TEST_CASE("containers/vector roundtrip", "make sure vectors can be round-tripped") {
	sol::state lua;
	std::vector<int> v { 1, 2, 3 };
	lua.set_function("f", [&]() -> std::vector<int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::vector<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/deque roundtrip", "make sure deques can be round-tripped") {
	sol::state lua;
	std::deque<int> v { 1, 2, 3 };
	lua.set_function("f", [&]() -> std::deque<int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::deque<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/array roundtrip", "make sure arrays can be round-tripped") {
	sol::state lua;
	std::array<int, 3> v { { 1, 2, 3 } };
	lua.set_function("f", [&]() -> std::array<int, 3>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::array<int, 3> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/list roundtrip", "make sure lists can be round-tripped") {
	sol::state lua;
	std::list<int> v { 1, 2, 3 };
	lua.set_function("f", [&]() -> std::list<int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::list<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/forward_list roundtrip", "make sure forward_lists can be round-tripped") {
	sol::state lua;
	std::forward_list<int> v { 1, 2, 3 };
	lua.set_function("f", [&]() -> std::forward_list<int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::forward_list<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/map roundtrip", "make sure maps can be round-tripped") {
	sol::state lua;
	std::map<std::string, int> v { { "a", 1 }, { "b", 2 }, { "c", 3 } };
	lua.set_function("f", [&]() -> std::map<std::string, int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::map<std::string, int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/unordered_map roundtrip", "make sure unordered_maps can be round-tripped") {
	sol::state lua;
	std::unordered_map<std::string, int> v { { "a", 1 }, { "b", 2 }, { "c", 3 } };
	lua.set_function("f", [&]() -> std::unordered_map<std::string, int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::unordered_map<std::string, int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/unordered_set roundtrip", "make sure unordered_sets can be round-tripped") {
	sol::state lua;
	std::unordered_set<int> v { 1, 2, 3 };
	lua.set_function("f", [&]() -> std::unordered_set<int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::unordered_set<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/set roundtrip", "make sure sets can be round-tripped") {
	sol::state lua;
	std::set<int> v { 1, 2, 3 };
	lua.set_function("f", [&]() -> std::set<int>& { return v; });
	auto result1 = lua.safe_script("x = f()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	std::set<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/ipairs test", "ensure that abstractions roundtrip properly") {
	struct thing {
		int x = 20;
	};
	thing t {};
	sol::state lua;
	lua.open_libraries();

	lua.set_function("f", [&t]() { return std::vector<thing*>(5, &t); });

	auto result1 = lua.safe_script(R"(
c = f()
)",
	     sol::script_pass_on_error);
	REQUIRE(result1.valid());

	auto result2 = lua.safe_script(R"(
check = {}
local i = 1
while c[i] do
	check[i] = c[i]
	i = i + 1
end
)",
	     sol::script_pass_on_error);
	REQUIRE(result2.valid());

	sol::table c = lua["check"];
	for (std::size_t i = 1; i < 6; ++i) {
		thing& ct = c[i];
		REQUIRE(&t == &ct);
		REQUIRE(ct.x == 20);
	}
}
