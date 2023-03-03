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

#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <string_view>
#include <variant>

struct int_entry {
	int value;

	int_entry() : value(0) {
	}

	int_entry(int v) : value(v) {
	}

	bool operator==(const int_entry& e) const {
		return value == e.value;
	}
};

std::mutex lua_value_construct_require_mutex;

void lua_value_construct_race() {
	sol::state lua;
	try {
		lua.open_libraries();
		lua["a"] = sol::lua_value(lua, 24);
		int a = lua["a"];
		{
			std::lock_guard<std::mutex> lg(lua_value_construct_require_mutex);
			REQUIRE(a == 24);
		}
	}
	catch (const sol::error& e) {
		std::lock_guard<std::mutex> lg(lua_value_construct_require_mutex);
		INFO(e.what());
		REQUIRE(false);
	}
	catch (...) {
		std::lock_guard<std::mutex> lg(lua_value_construct_require_mutex);
		REQUIRE(false);
	}
}

TEST_CASE("lua_value/nested", "make nested values can be put in lua_value properly") {
#if SOL_IS_ON(SOL_STD_VARIANT)
	using mixed_table_entry = std::variant<int, int_entry, std::string>;
	using nested_entry = std::variant<int, int_entry, std::string, std::vector<mixed_table_entry>>;

	const std::vector<std::variant<int, int_entry>> mixed_table_truth = { 1, int_entry(2), 3, int_entry(4), 5 };
	const std::vector<nested_entry> mixed_nested_table_truth
	     = { 1, int_entry(2), 3, int_entry(4), std::vector<mixed_table_entry> { 5, 6, int_entry(7), "8" } };

	sol::state lua;

	sol::lua_value lv_mixed_table(lua, sol::array_value { 1, int_entry(2), 3, int_entry(4), 5 });
	sol::lua_value lv_mixed_nested_table(lua, sol::array_value { 1, int_entry(2), 3, int_entry(4), sol::array_value { 5, 6, int_entry(7), "8" } });

	REQUIRE(lv_mixed_table.is<sol::table>());
	REQUIRE(lv_mixed_nested_table.is<sol::table>());

	std::vector<std::variant<int, int_entry>> mixed_table_value_lv = lv_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
	std::vector<nested_entry> mixed_nested_table_value_lv = lv_mixed_nested_table.as<std::vector<nested_entry>>();

	REQUIRE(mixed_table_truth == mixed_table_value_lv);
	REQUIRE(mixed_nested_table_truth == mixed_nested_table_value_lv);

	SECTION("type check (object)") {
		sol::object obj_mixed_table(lv_mixed_table.value());
		sol::object obj_mixed_nested_table(lv_mixed_nested_table.value());

		REQUIRE(obj_mixed_table.is<sol::table>());
		REQUIRE(obj_mixed_nested_table.is<sol::table>());

		std::vector<std::variant<int, int_entry>> mixed_table_value = obj_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
		std::vector<nested_entry> mixed_nested_table_value = obj_mixed_nested_table.as<std::vector<nested_entry>>();

		REQUIRE(mixed_table_truth == mixed_table_value);
		REQUIRE(mixed_nested_table_truth == mixed_nested_table_value);
	}
	SECTION("pushing/popping") {
		lua["obj_mixed_table"] = lv_mixed_table;
		lua["obj_mixed_nested_table"] = lv_mixed_nested_table;

		sol::lua_value obj_mixed_table = lua["obj_mixed_table"];
		sol::lua_value obj_mixed_nested_table = lua["obj_mixed_nested_table"];

		REQUIRE(obj_mixed_table.is<sol::table>());
		REQUIRE(obj_mixed_nested_table.is<sol::table>());

		std::vector<std::variant<int, int_entry>> mixed_table_value = obj_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
		std::vector<nested_entry> mixed_nested_table_value = obj_mixed_nested_table.as<std::vector<nested_entry>>();

		REQUIRE(mixed_table_truth == mixed_table_value);
		REQUIRE(mixed_nested_table_truth == mixed_nested_table_value);
	}
	SECTION("pushing/popping (object)") {
		lua["obj_mixed_table"] = lv_mixed_table;
		lua["obj_mixed_nested_table"] = lv_mixed_nested_table;

		sol::object obj_mixed_table = lua["obj_mixed_table"];
		sol::object obj_mixed_nested_table = lua["obj_mixed_nested_table"];

		REQUIRE(obj_mixed_table.is<sol::table>());
		REQUIRE(obj_mixed_nested_table.is<sol::table>());

		std::vector<std::variant<int, int_entry>> mixed_table_value = obj_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
		std::vector<nested_entry> mixed_nested_table_value = obj_mixed_nested_table.as<std::vector<nested_entry>>();

		REQUIRE(mixed_table_truth == mixed_table_value);
		REQUIRE(mixed_nested_table_truth == mixed_nested_table_value);
	}
#else
	REQUIRE(true);
#endif // C++17
}

TEST_CASE("lua_value/nested key value", "make nested values (key value) can be put in lua_value properly") {
#if SOL_IS_ON(SOL_STD_VARIANT)
	using mixed_table_entry = std::variant<int, int_entry, std::string>;
	using nested_entry = std::variant<int, int_entry, std::string, std::vector<mixed_table_entry>>;

	const std::vector<std::variant<int, int_entry>> mixed_table_truth = { 1, int_entry(2), 3, int_entry(4), 5 };
	const std::vector<nested_entry> mixed_nested_table_truth
	     = { 1, int_entry(2), 3, int_entry(4), std::vector<mixed_table_entry> { 5, 6, int_entry(7), "8" } };

	sol::state lua;

	sol::lua_value lv_mixed_table(lua, sol::array_value { 1, int_entry(2), 3, int_entry(4), 5 });
	sol::lua_value lv_mixed_nested_table(lua, sol::array_value { 1, int_entry(2), 3, int_entry(4), sol::array_value { 5, 6, int_entry(7), "8" } });

	REQUIRE(lv_mixed_table.is<sol::table>());
	REQUIRE(lv_mixed_nested_table.is<sol::table>());

	std::vector<std::variant<int, int_entry>> mixed_table_value_lv = lv_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
	std::vector<nested_entry> mixed_nested_table_value_lv = lv_mixed_nested_table.as<std::vector<nested_entry>>();

	REQUIRE(mixed_table_truth == mixed_table_value_lv);
	REQUIRE(mixed_nested_table_truth == mixed_nested_table_value_lv);

	SECTION("type check (object)") {
		sol::object obj_mixed_table(lv_mixed_table.value());
		sol::object obj_mixed_nested_table(lv_mixed_nested_table.value());

		REQUIRE(obj_mixed_table.is<sol::table>());
		REQUIRE(obj_mixed_nested_table.is<sol::table>());

		std::vector<std::variant<int, int_entry>> mixed_table_value = obj_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
		std::vector<nested_entry> mixed_nested_table_value = obj_mixed_nested_table.as<std::vector<nested_entry>>();

		REQUIRE(mixed_table_truth == mixed_table_value);
		REQUIRE(mixed_nested_table_truth == mixed_nested_table_value);
	}
	SECTION("pushing/popping") {
		lua["obj_mixed_table"] = lv_mixed_table;
		lua["obj_mixed_nested_table"] = lv_mixed_nested_table;

		sol::lua_value obj_mixed_table = lua["obj_mixed_table"];
		sol::lua_value obj_mixed_nested_table = lua["obj_mixed_nested_table"];

		REQUIRE(obj_mixed_table.is<sol::table>());
		REQUIRE(obj_mixed_nested_table.is<sol::table>());

		std::vector<std::variant<int, int_entry>> mixed_table_value = obj_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
		std::vector<nested_entry> mixed_nested_table_value = obj_mixed_nested_table.as<std::vector<nested_entry>>();

		REQUIRE(mixed_table_truth == mixed_table_value);
		REQUIRE(mixed_nested_table_truth == mixed_nested_table_value);
	}
	SECTION("pushing/popping (object)") {
		lua["obj_mixed_table"] = lv_mixed_table;
		lua["obj_mixed_nested_table"] = lv_mixed_nested_table;

		sol::object obj_mixed_table = lua["obj_mixed_table"];
		sol::object obj_mixed_nested_table = lua["obj_mixed_nested_table"];

		REQUIRE(obj_mixed_table.is<sol::table>());
		REQUIRE(obj_mixed_nested_table.is<sol::table>());

		std::vector<std::variant<int, int_entry>> mixed_table_value = obj_mixed_table.as<std::vector<std::variant<int, int_entry>>>();
		std::vector<nested_entry> mixed_nested_table_value = obj_mixed_nested_table.as<std::vector<nested_entry>>();

		REQUIRE(mixed_table_truth == mixed_table_value);
		REQUIRE(mixed_nested_table_truth == mixed_nested_table_value);
	}
#else
	REQUIRE(true);
#endif // C++17
}

TEST_CASE("lua_value/basic types", "make sure we can stick values and nested values in a lua_value and retrieve them") {
	sol::state lua;

	const int_entry userdata_truth = int_entry(3);
	const std::vector<int> int_table_truth = { 1, 2, 3, 4, 5 };
	const std::map<int, int> int_map_truth = { { 1, 2 }, { 3, 4 }, { 5, 6 } };

	sol::lua_value lv_int(lua, 1);
	sol::lua_value lv_double(lua, 2.0);
	sol::lua_value lv_string(lua, "heyo");
	sol::lua_value lv_lstring(lua, L"hiyo");
	sol::lua_value lv_bool(lua, true);
	sol::lua_value lv_nil(lua, sol::lua_nil);
	sol::lua_value lv_userdata(lua, int_entry(3));
	sol::lua_value lv_int_table(lua, { 1, 2, 3, 4, 5 });
	sol::lua_value lv_int_map(lua, { { 1, 2 }, { 3, 4 }, { 5, 6 } });
	REQUIRE(lv_int.is<int>());
	REQUIRE(lv_double.is<double>());
	REQUIRE(lv_string.is<std::string>());
	REQUIRE(lv_lstring.is<std::string>());
	REQUIRE(lv_bool.is<bool>());
	REQUIRE(lv_nil.is<sol::lua_nil_t>());
	REQUIRE(lv_userdata.is<sol::userdata>());
	REQUIRE(lv_userdata.is<int_entry>());
	REQUIRE(lv_int_table.is<sol::table>());
	REQUIRE(lv_int_map.is<sol::table>());

	REQUIRE(lv_int.as<int>() == 1);
	REQUIRE(lv_double.as<double>() == 2.0);
	REQUIRE(lv_string.as<std::string>() == "heyo");
	REQUIRE(lv_lstring.as<std::string>() == "hiyo");
	REQUIRE(lv_lstring.as<std::wstring>() == L"hiyo");
	REQUIRE(lv_bool.as<bool>());
	REQUIRE(lv_nil.as<sol::lua_nil_t>() == sol::lua_nil);
	REQUIRE(lv_userdata.as<int_entry>() == userdata_truth);

	std::vector<int> int_table_value_lv = lv_int_table.as<std::vector<int>>();
	REQUIRE(int_table_truth == int_table_value_lv);
	std::map<int, int> int_map_value_lv = lv_int_map.as<std::map<int, int>>();
	REQUIRE(int_map_truth == int_map_value_lv);

	SECTION("type check (object)") {
		sol::object obj_int(lv_int.value());
		sol::object obj_double(lv_double.value());
		sol::object obj_string(lv_string.value());
		sol::object obj_lstring(lv_lstring.value());
		sol::object obj_bool(lv_bool.value());
		sol::object obj_nil(lv_nil.value());
		sol::object obj_userdata(lv_userdata.value());
		sol::object obj_int_table(lv_int_table.value());
		sol::object obj_int_map(lv_int_map.value());

		REQUIRE(obj_int.is<int>());
		REQUIRE(obj_double.is<double>());
		REQUIRE(obj_string.is<std::string>());
		REQUIRE(obj_lstring.is<std::string>());
		REQUIRE(obj_bool.is<bool>());
		REQUIRE(obj_nil.is<sol::lua_nil_t>());
		REQUIRE(obj_userdata.is<sol::userdata>());
		REQUIRE(obj_userdata.is<int_entry>());
		REQUIRE(obj_int_table.is<sol::table>());
		REQUIRE(obj_int_map.is<sol::table>());

		REQUIRE(obj_int.as<int>() == 1);
		REQUIRE(obj_double.as<double>() == 2.0);
		REQUIRE(obj_string.as<std::string>() == "heyo");
		REQUIRE(obj_lstring.as<std::string>() == "hiyo");
		REQUIRE(obj_lstring.as<std::wstring>() == L"hiyo");
		REQUIRE(obj_bool.as<bool>());
		REQUIRE(obj_userdata.as<int_entry>() == userdata_truth);
		REQUIRE(obj_nil.as<sol::lua_nil_t>() == sol::lua_nil);

		std::vector<int> int_table_value = obj_int_table.as<std::vector<int>>();
		REQUIRE(int_table_truth == int_table_value);
		std::map<int, int> int_map_value = obj_int_map.as<std::map<int, int>>();
		REQUIRE(int_map_truth == int_map_value);
	}
	SECTION("push/popping") {
		lua["obj_int"] = lv_int;
		lua["obj_double"] = lv_double;
		lua["obj_string"] = lv_string;
		lua["obj_lstring"] = lv_lstring;
		lua["obj_bool"] = lv_bool;
		lua["obj_nil"] = lv_nil;
		lua["obj_userdata"] = lv_userdata;
		lua["obj_int_table"] = lv_int_table;
		lua["obj_int_map"] = lv_int_map;

		// these all actually invoke the constructor
		// so do one .get<> explicitly to ensure it's
		// working correctl for a few cases...
		// but it's nice to make sure it's all there now
		sol::lua_value obj_int = lua["obj_int"].get<sol::lua_value>();
		sol::lua_value obj_double = lua["obj_double"].get<sol::lua_value>();
		sol::lua_value obj_string = lua["obj_string"].get<sol::lua_value>();
		sol::lua_value obj_lstring = lua["obj_lstring"].get<sol::lua_value>();
		sol::lua_value obj_bool = lua["obj_bool"].get<sol::lua_value>();
		sol::lua_value obj_nil = lua["obj_nil"];
		sol::lua_value obj_userdata = lua["obj_userdata"];
		sol::lua_value obj_int_table = lua["obj_int_table"];
		sol::lua_value obj_int_map = lua["obj_int_map"];

		REQUIRE(obj_int.is<int>());
		REQUIRE(obj_double.is<double>());
		REQUIRE(obj_string.is<std::string>());
		REQUIRE(obj_lstring.is<std::string>());
		REQUIRE(obj_bool.is<bool>());
		REQUIRE(obj_nil.is<sol::lua_nil_t>());
		REQUIRE(obj_int_table.is<sol::table>());
		REQUIRE(obj_int_map.is<sol::table>());

		REQUIRE(obj_int.as<int>() == 1);
		REQUIRE(obj_double.as<double>() == 2.0);
		REQUIRE(obj_string.as<std::string>() == "heyo");
		REQUIRE(obj_lstring.as<std::string>() == "hiyo");
		REQUIRE(obj_lstring.as<std::wstring>() == L"hiyo");
		REQUIRE(obj_bool.as<bool>());
		REQUIRE(obj_nil.as<sol::lua_nil_t>() == sol::lua_nil);

		std::vector<int> int_table_value = obj_int_table.as<std::vector<int>>();
		REQUIRE(int_table_truth == int_table_value);
		std::map<int, int> int_map_value = obj_int_map.as<std::map<int, int>>();
		REQUIRE(int_map_truth == int_map_value);
	}
	SECTION("push/popping (object)") {
		lua["obj_int"] = lv_int;
		lua["obj_double"] = lv_double;
		lua["obj_string"] = lv_string;
		lua["obj_lstring"] = lv_lstring;
		lua["obj_bool"] = lv_bool;
		lua["obj_nil"] = lv_nil;
		lua["obj_userdata"] = lv_userdata;
		lua["obj_int_table"] = lv_int_table;
		lua["obj_int_map"] = lv_int_map;

		sol::object obj_int = lua["obj_int"];
		sol::object obj_double = lua["obj_double"];
		sol::object obj_string = lua["obj_string"];
		sol::object obj_lstring = lua["obj_lstring"];
		sol::object obj_bool = lua["obj_bool"];
		sol::object obj_nil = lua["obj_nil"];
		sol::object obj_userdata = lua["obj_userdata"];
		sol::object obj_int_table = lua["obj_int_table"];
		sol::object obj_int_map = lua["obj_int_map"];

		REQUIRE(obj_int.is<int>());
		REQUIRE(obj_double.is<double>());
		REQUIRE(obj_string.is<std::string>());
		REQUIRE(obj_lstring.is<std::string>());
		REQUIRE(obj_bool.is<bool>());
		REQUIRE(obj_nil.is<sol::lua_nil_t>());
		REQUIRE(obj_userdata.is<sol::userdata>());
		REQUIRE(obj_userdata.is<int_entry>());
		REQUIRE(obj_int_table.is<sol::table>());

		REQUIRE(obj_int.as<int>() == 1);
		REQUIRE(obj_double.as<double>() == 2.0);
		REQUIRE(obj_string.as<std::string>() == "heyo");
		REQUIRE(obj_lstring.as<std::string>() == "hiyo");
		REQUIRE(obj_lstring.as<std::wstring>() == L"hiyo");
		REQUIRE(obj_bool.as<bool>());
		REQUIRE(obj_nil.as<sol::lua_nil_t>() == sol::lua_nil);
		REQUIRE(obj_userdata.as<int_entry>() == userdata_truth);

		std::vector<int> int_table_value = obj_int_table.as<std::vector<int>>();
		REQUIRE(int_table_truth == int_table_value);
		std::map<int, int> int_map_value = obj_int_map.as<std::map<int, int>>();
		REQUIRE(int_map_truth == int_map_value);
	}
}

TEST_CASE("lua_value/threading", "test that thread_local in lua_value constructors do not race or clobber") {
	REQUIRE_NOTHROW([]() {
		std::thread thrds[24];
		for (int i = 0; i < 24; i++) {
			thrds[i] = std::thread(&lua_value_construct_race);
		}

		for (int i = 0; i < 24; i++) {
			thrds[i].join();
		}
	}());
}
