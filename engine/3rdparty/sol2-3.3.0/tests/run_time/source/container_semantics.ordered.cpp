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

#include <set>
#include <map>

template <typename T>
void ordered_container_check(sol::state& lua, T& items) {
	{
		auto r1 = lua.safe_script(R"(
for i=1,#c do 
	v = c[(i + 10)] 
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
		auto r1 = lua.safe_script("v1 = c:get(11)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("v2 = c:get(13)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("c:set(20)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("c:set(16)", sol::script_pass_on_error);
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
		auto r = lua.safe_script("c[18] = true", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	{
		auto r = lua.safe_script("v3 = c[20]", sol::script_pass_on_error);
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
	int i1 = lua["i1"];
	int i2 = lua["i2"];
	int io1 = lua["io1"];
	int io2 = lua["io2"];
	std::size_t s1 = lua["s1"];
	std::size_t s2 = lua["s2"];
	std::size_t s3 = lua["s3"];
	int v1 = lua["v1"];
	int v2 = lua["v2"];
	int v3 = lua["v3"];
	int values[] = { 12, 13, 15, 16, 17, 18, 20 };
	{
		std::size_t idx = 0;
		for (const auto& i : items) {
			const auto& v = values[idx];
			REQUIRE((i == v));
			++idx;
		}
	}
	REQUIRE((s1 == 7));
	REQUIRE((s2 == 6));
	REQUIRE((s3 == 5));
	REQUIRE((len == 7));
	REQUIRE((first == 12));
	REQUIRE((last == 20));
	REQUIRE((i1 == 11));
	REQUIRE((i2 == 14));
	REQUIRE((io1 == 2));
	REQUIRE((io2 == 3));
	REQUIRE((v1 == 11));
	REQUIRE((v2 == 13));
	REQUIRE((v3 == 20));
}

template <typename T>
void associative_ordered_container_check(sol::state& lua, T& items) {
	{
		auto r1 = lua.safe_script(R"(
for i=1,#c do 
	v = c[(i + 10)] 
	assert(v == (i + 20))
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
		auto r1 = lua.safe_script("v1 = c:get(11)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("v2 = c:get(13)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
	}
	{
		auto r1 = lua.safe_script("c:set(20, 30)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r2 = lua.safe_script("c:set(16, 26)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
		auto r3 = lua.safe_script("c:set(12, 31)", sol::script_pass_on_error);
		REQUIRE(r3.valid());
	}
	{
		auto r5 = lua.safe_script("s1 = #c", sol::script_pass_on_error);
		REQUIRE(r5.valid());
		auto r1 = lua.safe_script("c:erase(11)", sol::script_pass_on_error);
		REQUIRE(r1.valid());
		auto r3 = lua.safe_script("s2 = #c", sol::script_pass_on_error);
		REQUIRE(r3.valid());
		auto r2 = lua.safe_script("c:erase(14)", sol::script_pass_on_error);
		REQUIRE(r2.valid());
		auto r4 = lua.safe_script("s3 = #c", sol::script_pass_on_error);
		REQUIRE(r4.valid());
	}
	{
		auto r = lua.safe_script("c:add(17, 27)", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	{
		auto r = lua.safe_script("c[18] = 28", sol::script_pass_on_error);
		REQUIRE(r.valid());
	}
	{
		auto r = lua.safe_script("v3 = c[20]", sol::script_pass_on_error);
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
	const std::pair<const short, int>& first = *items.begin();
	const std::pair<const short, int>& last = *backit;
	int i1 = lua["i1"];
	int i2 = lua["i2"];
	int io1 = lua["io1"];
	int io2 = lua["io2"];
	std::size_t s1 = lua["s1"];
	std::size_t s2 = lua["s2"];
	std::size_t s3 = lua["s3"];
	int v1 = lua["v1"];
	int v2 = lua["v2"];
	int v3 = lua["v3"];
	std::pair<const short, int> values[]
	     = { { (short)12, 31 }, { (short)13, 23 }, { (short)15, 25 }, { (short)16, 26 }, { (short)17, 27 }, { (short)18, 28 }, { (short)20, 30 } };
	{
		std::size_t idx = 0;
		for (const auto& i : items) {
			const auto& v = values[idx];
			REQUIRE((i == v));
			++idx;
		}
	}
	REQUIRE((s1 == 7));
	REQUIRE((s2 == 6));
	REQUIRE((s3 == 5));
	REQUIRE((len == 7));
	REQUIRE((first.first == 12));
	REQUIRE((last.first == 20));
	REQUIRE((first.second == 31));
	REQUIRE((last.second == 30));
	REQUIRE((i1 == 21));
	REQUIRE((i2 == 24));
	REQUIRE((io1 == 2));
	REQUIRE((io2 == 3));
	REQUIRE((v1 == 21));
	REQUIRE((v2 == 23));
	REQUIRE((v3 == 30));
}

template <typename T>
void associative_ordered_container_key_value_check(sol::state& lua, T& data, T& reflect) {
	typedef typename T::key_type K;
	typedef typename T::mapped_type V;
	lua["collect"] = [&reflect](K k, V v) { reflect.insert({ k, v }); };

#if SOL_LUA_VERSION_I_ > 502
	lua["val"] = data;
	auto r = lua.safe_script(R"(
for k, v in pairs(val) do
	collect(k, v)
end
print()
)",
	     sol::script_pass_on_error);
	REQUIRE(r.valid());
#else
	reflect = data;
#endif
	REQUIRE((data == reflect));
}

template <typename T>
void ordered_lookup_container_check(sol::state& lua, T&) {
	auto result0 = lua.safe_script("assert(c['a'] == 'a')", sol::script_default_on_error);
	REQUIRE(result0.valid());
	auto result1 = lua.safe_script("assert(c['b'] == 'b')", sol::script_default_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("assert(c['c'] == 'c')", sol::script_default_on_error);
	REQUIRE(result2.valid());
}

TEST_CASE("containers/ordered lookup containers", "check ordered container types") {
	SECTION("set") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::set<int> items { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		ordered_container_check(lua, items);
	}
	SECTION("set string") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::set<std::string> items({ "a", "b", "c" });
		lua["c"] = &items;
		ordered_lookup_container_check(lua, items);
	}
	SECTION("multiset") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::multiset<int> items { 11, 12, 13, 14, 15 };
		lua["c"] = &items;
		ordered_container_check(lua, items);
	}
	SECTION("multiset string") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::multiset<std::string> items({ "a", "b", "c" });
		lua["c"] = &items;
		ordered_lookup_container_check(lua, items);
	}
}

TEST_CASE("containers/associative ordered containers", "check associative (map) containers that are ordered fulfill basic functionality requirements") {
	SECTION("map") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::map<short, int> items { { (short)11, 21 }, { (short)12, 22 }, { (short)13, 23 }, { (short)14, 24 }, { (short)15, 25 } };
		lua["c"] = &items;
		associative_ordered_container_check(lua, items);
	}
	SECTION("map string") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::map<std::string, std::string> items { { "a", "a" }, { "b", "b" }, { "c", "c" } };
		lua["c"] = &items;
		ordered_lookup_container_check(lua, items);
	}
	SECTION("multimap") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::multimap<short, int> items { { (short)11, 21 }, { (short)12, 22 }, { (short)13, 23 }, { (short)14, 24 }, { (short)15, 25 } };
		lua["c"] = &items;
		associative_ordered_container_check(lua, items);
	}
	SECTION("multimap string") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		std::multimap<std::string, std::string> items { { "a", "a" }, { "b", "b" }, { "c", "c" } };
		lua["c"] = &items;
		ordered_lookup_container_check(lua, items);
	}
}

TEST_CASE("containers/associative ordered pairs", "check to make sure pairs works properly for key-value types") {
	struct bar { };
	std::unique_ptr<bar> ua(new bar()), ub(new bar()), uc(new bar());
	bar* a = ua.get();
	bar* b = ub.get();
	bar* c = uc.get();

	SECTION("map") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		std::map<std::string, bar*> data({ { "a", a }, { "b", b }, { "c", c } });
		std::map<std::string, bar*> reflect;
		associative_ordered_container_key_value_check(lua, data, reflect);
	}
	SECTION("multimap") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		std::multimap<std::string, bar*> data({ { "a", a }, { "b", b }, { "c", c } });
		std::multimap<std::string, bar*> reflect;
		associative_ordered_container_key_value_check(lua, data, reflect);
	}
}
