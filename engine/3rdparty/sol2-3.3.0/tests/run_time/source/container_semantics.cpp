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

#include <iterator>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <set>
#include <map>
#include <array>
#include <numeric> // std::iota

template <typename T>
void sequence_container_check(sol::state& lua, T& items) {
	{
		auto r1 = lua.safe_script(R"(
for i=1,#c do 
	v = c[i] 
	assert(v == (i + 10)) 
end
		)",
		     sol::script_pass_on_error);
		REQUIRE(r1.valid());
	}
	{
		auto r1 = lua.safe_script("i1 = c:find(11)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("i2 = c:find(14)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("io1 = c:index_of(12)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("io2 = c:index_of(13)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("v1 = c:get(1)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("v2 = c:get(3)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("c:set(2, 20)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("c:set(6, 16)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r5 = lua.safe_script("s1 = #c", sol::script_pass_on_error);
		REQUIRE(r5.valid());
		auto r1 = lua.safe_script("c:erase(i1)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r3 = lua.safe_script("s2 = #c", sol::script_pass_on_error);
		REQUIRE(r3.valid());
		auto r2 = lua.safe_script("c:erase(i2)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
		auto r4 = lua.safe_script("s3 = #c", sol::script_pass_on_error);
		REQUIRE(r4.valid());
	}
	{
		auto r = lua.safe_script("c:add(17)", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	{
		auto r = lua.safe_script("c[Issue-c + 1] = 18", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	{
		auto r = lua.safe_script("v3 = c[Issue-c]", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	auto backit = items.begin();
	std::size_t len = 0;
	{
		auto e = items.end();
		auto last = backit;
		for (; backit != e; ++backit, ++len) {
			if (backit == e) {
				break;
			}
			last = backit;
		}
		backit = last;
	}
	const int& first = *items.begin();
	const int& last = *backit;
	std::size_t i1 = lua["i1"];
	std::size_t i2 = lua["i2"];
	std::size_t io1 = lua["io1"];
	std::size_t io2 = lua["io2"];
	std::size_t s1 = lua["s1"];
	std::size_t s2 = lua["s2"];
	std::size_t s3 = lua["s3"];
	int v1 = lua["v1"];
	int v2 = lua["v2"];
	int v3 = lua["v3"];
	int values[6] = { 20, 13, 14, 16, 17, 18 };
	{
		std::size_t idx = 0;
		for (const auto& i : items) {
			const auto& v = values[idx];
			REQUIRE((i == v));
			++idx;
		}
	}
	REQUIRE((s1 == 6));
	REQUIRE((s2 == 5));
	REQUIRE((s3 == 4));
	REQUIRE((len == 6));
	REQUIRE((first == 20));
	REQUIRE((last == 18));
	REQUIRE((i1 == 1));
	REQUIRE((i2 == 4));
	REQUIRE((io1 == 2));
	REQUIRE((io2 == 3));
	REQUIRE((v1 == 11));
	REQUIRE((v2 == 13));
	REQUIRE((v3 == 18));
}

template <typename T>
void fixed_container_check(sol::state& lua, T& items) {
	{
		auto r1 = lua.safe_script(R"(
for i=1,#c do 
	v = c[i] 
	assert(v == (i + 10)) 
end
		)",
		     sol::script_pass_on_error);
		REQUIRE(r1.valid());
	}
	{
		auto r1 = lua.safe_script("i1 = c:find(11)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("i2 = c:find(14)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("io1 = c:index_of(11)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("io2 = c:index_of(14)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("v1 = c:get(2)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("v2 = c:get(5)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("c:set(2, 20)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("c:set(6, 16)", sol::script_pass_on_error);
		REQUIRE_FALSE(r2.valid());
	}
	{
		auto r5 = lua.safe_script("s1 = #c", sol::script_pass_on_error);
		REQUIRE(r5.valid());
		auto r1 = lua.safe_script("c:erase(i1)", sol::script_pass_on_error);
		REQUIRE_FALSE(r1.valid());
		auto r3 = lua.safe_script("s2 = #c", sol::script_pass_on_error);
		REQUIRE(r3.valid());
		auto r2 = lua.safe_script("c:erase(i2)", sol::script_pass_on_error);
		REQUIRE_FALSE(r2.valid());
		auto r4 = lua.safe_script("s3 = #c", sol::script_pass_on_error);
		REQUIRE(r4.valid());
	}
	{
		auto r = lua.safe_script("c:add(17)", sol::script_pass_on_error);
		REQUIRE_FALSE(r.valid());
	}
	{
		auto r = lua.safe_script("c[5] = 18", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	{
		auto r = lua.safe_script("v3 = c[4]", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	auto backit = std::begin(items);
	std::size_t len = 0;
	{
		auto e = std::end(items);
		auto last = backit;
		for (; backit != e; ++backit, ++len) {
			if (backit == e) {
				break;
			}
			last = backit;
		}
		backit = last;
	}
	const int& first = *std::begin(items);
	const int& last = *backit;
	int i1 = lua["i1"];
	int i2 = lua["i2"];
	std::size_t s1 = lua["s1"];
	std::size_t s2 = lua["s2"];
	std::size_t s3 = lua["s3"];
	int v1 = lua["v1"];
	int v2 = lua["v2"];
	int v3 = lua["v3"];
	int values[] = { 11, 20, 13, 14, 18 };
	{
		std::size_t idx = 0;
		for (const auto& i : items) {
			const auto& v = values[idx];
			REQUIRE((i == v));
			++idx;
		}
	}
	REQUIRE((first == 11));
	REQUIRE((last == 18));
	REQUIRE((s1 == 5));
	REQUIRE((s2 == 5));
	REQUIRE((s3 == 5));
	REQUIRE((len == 5));
	REQUIRE((i1 == 1));
	REQUIRE((i2 == 4));
	REQUIRE((v1 == 12));
	REQUIRE((v2 == 15));
	REQUIRE((v3 == 14));
}

TEST_CASE("containers/sequence containers", "check all of the functinos for every single container") {
	SECTION("vector") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		std::vector<int> items { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		sequence_container_check(lua, items);
	}
	SECTION("list") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		std::list<int> items { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		sequence_container_check(lua, items);
	}
	SECTION("forward_list") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		std::forward_list<int> items { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		sequence_container_check(lua, items);
	}
	SECTION("deque") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		std::deque<int> items { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		sequence_container_check(lua, items);
	}
}

TEST_CASE("containers/fixed containers", "check immutable container types") {
	SECTION("array") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		std::array<int, 5> items { { 11, 12, 13, 14, 15 } };
		lua["c"] = &items;
		fixed_container_check(lua, items);
	}
	SECTION("array ref") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		std::array<int, 5> items { { 11, 12, 13, 14, 15 } };
		lua["c"] = std::ref(items);
		fixed_container_check(lua, items);
	}
	SECTION("c array") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		int items[5] = { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		fixed_container_check(lua, items);
	}
	SECTION("c array ref") {
		sol::state lua;
		sol::stack_guard luasg(lua);
		lua.open_libraries(sol::lib::base);

		int items[5] = { 11, 12, 13, 14, 15 };
		lua["c"] = std::ref(items);
		fixed_container_check(lua, items);
	}
}

TEST_CASE("containers/auxiliary functions test", "make sure the manipulation functions are present and usable and working across various container types") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries();

	auto result1 = lua.safe_script(R"(
function g (x)
	x:add(20)
end

function h (x)
	x:add(20, 40)
end

function i (x)
	x:clear()
end

function sf (x,v)
	return x:find(v)
end

)",
	     sol::script_pass_on_error);
	REQUIRE(result1.valid());
	// Have the function we
	// just defined in Lua
	sol::function g = lua["g"];
	sol::function h = lua["h"];
	sol::function i = lua["i"];
	sol::function sf = lua["sf"];

	// Set a global variable called
	// "arr" to be a vector of 5 lements
	lua["c_arr"] = std::array<int, 5> { { 2, 4, 6, 8, 10 } };
	lua["arr"] = std::vector<int> { 2, 4, 6, 8, 10 };
	lua["map"] = std::map<int, int> { { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 8 }, { 5, 10 } };
	lua["set"] = std::set<int> { 2, 4, 6, 8, 10 };
	std::array<int, 5>& c_arr = lua["c_arr"];
	std::vector<int>& arr = lua["arr"];
	std::map<int, int>& map = lua["map"];
	std::set<int>& set = lua["set"];
	REQUIRE(c_arr.size() == 5);
	REQUIRE(arr.size() == 5);
	REQUIRE(map.size() == 5);
	REQUIRE(set.size() == 5);

	g(lua["set"]);
	g(lua["arr"]);
	h(lua["map"]);
	REQUIRE(arr.size() == 6);
	REQUIRE(map.size() == 6);
	REQUIRE(set.size() == 6);

	{
		int r = sf(set, 8);
		REQUIRE(r == 8);
		sol::object rn = sf(set, 9);
		REQUIRE(rn == sol::lua_nil);
	}

	{
		int r = sf(map, 3);
		REQUIRE(r == 6);
		sol::object rn = sf(map, 9);
		REQUIRE(rn == sol::lua_nil);
	}
	i(lua["arr"]);
	i(lua["map"]);
	i(lua["set"]);
	REQUIRE(arr.empty());
	REQUIRE(map.empty());
	REQUIRE(set.empty());

	auto result2 = lua.safe_script(R"(
c_arr[1] = 7
c_arr[2] = 7
c_arr[3] = 7
)",
	     sol::script_pass_on_error);
	REQUIRE(result2.valid());
}

TEST_CASE("containers/indices test", "test indices on fixed array types") {
#if 0
	SECTION("zero index test") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		lua["c_arr"] = std::array<int, 5>{ { 2, 4, 6, 8, 10 } };
		auto result = lua.safe_script(R"(
c_arr[0] = 7
)", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}

	SECTION("negative index test") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		lua["c_arr"] = std::array<int, 5>{ { 2, 4, 6, 8, 10 } };
		auto result = lua.safe_script(R"(
c_arr[-1] = 7
)", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
#endif // Something is wrong with g++'s lower versions: it always fails this test...
}
