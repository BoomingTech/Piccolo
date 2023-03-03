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

inline namespace sol2_tables_test {
	inline int my_custom_next(lua_State* L_) noexcept {
		sol::stack_reference table_stack_ref(L_, sol::raw_index(1));
		sol::stateless_stack_reference key_stack_ref(L_, sol::raw_index(2));
		int result = lua_next(table_stack_ref.lua_state(), table_stack_ref.stack_index());
		if (result == 0) {
			sol::stack::push(L_, sol::lua_nil);
			return 1;
		}
		return 2;
	}

	inline auto my_custom_pairs(sol::reference table_ref) noexcept {
		return std::make_tuple(&my_custom_next, std::move(table_ref), sol::lua_nil);
	}
} // namespace sol2_tables_test

TEST_CASE("tables/for_each", "Testing the use of for_each to get values from a lua table") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.safe_script(
	     "arr = {\n"
	     "[0] = \"Hi\",\n"
	     "[1] = 123.45,\n"
	     "[2] = \"String value\",\n"
	     // Does nothing
	     //"[3] = nil,\n"
	     //"[nil] = 3,\n"
	     "[\"WOOF\"] = 123,\n"
	     "}");
	sol::table tbl = lua["arr"];
	std::size_t tablesize = 4;
	std::size_t iterations = 0;
	auto fx = [&iterations](sol::object key, sol::object value) {
		++iterations;
		sol::type keytype = key.get_type();
		switch (keytype) {
		case sol::type::number:
			switch (key.as<int>()) {
			case 0:
				REQUIRE((value.as<std::string>() == "Hi"));
				break;
			case 1:
				REQUIRE((value.as<double>() == 123.45));
				break;
			case 2:
				REQUIRE((value.as<std::string>() == "String value"));
				break;
			case 3:
				REQUIRE((value.is<sol::lua_nil_t>()));
				break;
			}
			break;
		case sol::type::string:
			if (key.as<std::string>() == "WOOF") {
				REQUIRE((value.as<double>() == 123));
			}
			break;
		case sol::type::lua_nil:
			REQUIRE((value.as<double>() == 3));
			break;
		default:
			break;
		}
	};
	auto fxpair = [&fx](std::pair<sol::object, sol::object> kvp) { fx(kvp.first, kvp.second); };
	tbl.for_each(fx);
	REQUIRE(iterations == tablesize);

	iterations = 0;
	tbl.for_each(fxpair);
	REQUIRE(iterations == tablesize);
}

TEST_CASE("tables/for_each empty", "empty tables should not crash") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.safe_script("arr = {}");
	sol::table tbl = lua["arr"];
	REQUIRE(tbl.empty());
	std::size_t tablesize = 0;
	std::size_t iterations = 0;
	auto fx = [&iterations](sol::object key, sol::object value) {
		++iterations;
		sol::type keytype = key.get_type();
		switch (keytype) {
		case sol::type::number:
			switch (key.as<int>()) {
			case 0:
				REQUIRE((value.as<std::string>() == "Hi"));
				break;
			case 1:
				REQUIRE((value.as<double>() == 123.45));
				break;
			case 2:
				REQUIRE((value.as<std::string>() == "String value"));
				break;
			case 3:
				REQUIRE((value.is<sol::lua_nil_t>()));
				break;
			}
			break;
		case sol::type::string:
			if (key.as<std::string>() == "WOOF") {
				REQUIRE((value.as<double>() == 123));
			}
			break;
		case sol::type::lua_nil:
			REQUIRE((value.as<double>() == 3));
			break;
		default:
			break;
		}
	};
	auto fxpair = [&fx](std::pair<sol::object, sol::object> kvp) { fx(kvp.first, kvp.second); };
	tbl.for_each(fx);
	REQUIRE(iterations == tablesize);

	iterations = 0;
	tbl.for_each(fxpair);
	REQUIRE(iterations == tablesize);

	iterations = 0;
	for (const auto& kvp : tbl) {
		fxpair(kvp);
		++iterations;
	}
	REQUIRE(iterations == tablesize);
}

TEST_CASE("tables/iterators", "Testing the use of iteratrs to get values from a lua table") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.safe_script(
	     "arr = {\n"
	     "[0] = \"Hi\",\n"
	     "[1] = 123.45,\n"
	     "[2] = \"String value\",\n"
	     // Does nothing
	     //"[3] = nil,\n"
	     //"[nil] = 3,\n"
	     "[\"WOOF\"] = 123,\n"
	     "}");
	sol::table tbl = lua["arr"];
	std::size_t tablesize = 4;
	std::size_t iterations = 0;

	int begintop = 0;
	int endtop = 0;
	{
		test_stack_guard s(lua.lua_state(), begintop, endtop);
		for (auto& kvp : tbl) {
			[&iterations](sol::object key, sol::object value) {
				++iterations;
				sol::type keytype = key.get_type();
				switch (keytype) {
				case sol::type::number:
					switch (key.as<int>()) {
					case 0:
						REQUIRE((value.as<std::string>() == "Hi"));
						break;
					case 1:
						REQUIRE((value.as<double>() == 123.45));
						break;
					case 2:
						REQUIRE((value.as<std::string>() == "String value"));
						break;
					case 3:
						REQUIRE((value.is<sol::lua_nil_t>()));
						break;
					}
					break;
				case sol::type::string:
					if (key.as<std::string>() == "WOOF") {
						REQUIRE((value.as<double>() == 123));
					}
					break;
				case sol::type::lua_nil:
					REQUIRE((value.as<double>() == 3));
					break;
				default:
					break;
				}
			}(kvp.first, kvp.second);
		}
	}
	REQUIRE(begintop == endtop);
	REQUIRE(iterations == tablesize);
}

TEST_CASE("tables/pairs_iterators", "check that pairs()-style iteration works for sol::table types") {
	void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(0x02));
	auto verify_key_value = [&ptr](sol::object& key, sol::object& value) {
		switch (key.get_type()) {
		case sol::type::lightuserdata:
			REQUIRE(key.as<void*>() == ptr);
			REQUIRE(value.as<std::string>() == "bleat");
			break;
		case sol::type::number:
			REQUIRE(key.as<int>() == 1);
			REQUIRE(value.as<std::string>() == "bark");
			break;
		case sol::type::string:
			REQUIRE(key.as<std::string>() == "hi");
			REQUIRE(value.as<std::string>() == "meow");
			break;
		default:
			REQUIRE(false);
			break;
		}
	};
	SECTION("manual iterator") {
		SECTION("normal table") {
			int begintop = 0, endtop = 0;
			sol::state lua;
			test_stack_guard g(lua.lua_state(), begintop, endtop);
			lua.open_libraries(sol::lib::base);

			lua["t"] = sol::lua_value(lua, { { "hi", "meow" }, { 1, "bark" }, { ptr, "bleat" } });
			sol::table t = lua["t"];
			{
				int begintop2 = 0, endtop2 = 0;
				test_stack_guard g2(lua.lua_state(), begintop2, endtop2);
				auto first = sol::pairs_iterator(t);
				auto last = sol::pairs_sentinel();
				int index = 0;
				for (; first != last; ++first, ++index) {
					std::pair<sol::object, sol::object>& key_value_pair = *first;
					verify_key_value(key_value_pair.first, key_value_pair.second);
					REQUIRE(first.index() == index);
				}
			}
		}
		SECTION("with pairs metamethod") {
			sol::state lua;
			int begintop = 0, endtop = 0;
			test_stack_guard g(lua.lua_state(), begintop, endtop);
			lua.open_libraries(sol::lib::base);

			lua["t"] = sol::lua_value(lua, { { "hi", "meow" }, { 1, "bark" }, { ptr, "bleat" } });
			sol::table t = lua["t"];
			sol::table mt = lua.create_table();
			mt[sol::meta_function::pairs] = my_custom_pairs;
			t[sol::metatable_key] = mt;
			{
				auto first = sol::pairs_iterator(t);
				auto last = sol::pairs_sentinel();
				int index = 0;
				int begintop2 = 0, endtop2 = 0;
				test_stack_guard g2(lua.lua_state(), begintop2, endtop2);
				for (; first != last; ++first, ++index) {
					std::pair<sol::object, sol::object>& key_value_pair = *first;
					verify_key_value(key_value_pair.first, key_value_pair.second);
					REQUIRE(first.index() == index);
				}
			}
		}
		SECTION("with no global next function available") {
			int begintop = 0, endtop = 0;
			sol::state lua;
			test_stack_guard g(lua.lua_state(), begintop, endtop);

			lua["t"] = sol::lua_value(lua, { { "hi", "meow" }, { 1, "bark" }, { ptr, "bleat" } });
			sol::table t = lua["t"];
			{
				int begintop2 = 0, endtop2 = 0;
				test_stack_guard g2(lua.lua_state(), begintop2, endtop2);
				auto first = sol::pairs_iterator(t);
				auto last = sol::pairs_sentinel();
				int index = 0;
				lua.open_libraries(sol::lib::base);
				for (; first != last; ++first, ++index) {
					std::pair<sol::object, sol::object>& key_value_pair = *first;
					verify_key_value(key_value_pair.first, key_value_pair.second);
					REQUIRE(first.index() == index);
				}
			}
		}
	}
	SECTION("ranged for") {
		SECTION("normal table") {
			int begintop = 0, endtop = 0;
			sol::state lua;
			test_stack_guard g(lua.lua_state(), begintop, endtop);
			lua.open_libraries(sol::lib::base);

			lua["t"] = sol::lua_value(lua, { { "hi", "meow" }, { 1, "bark" }, { ptr, "bleat" } });
			sol::table t = lua["t"];
			{
				int begintop2 = 0, endtop2 = 0;
				test_stack_guard g2(lua.lua_state(), begintop2, endtop2);
				for (auto& key_value_pair : t.pairs()) {
					verify_key_value(key_value_pair.first, key_value_pair.second);
				}
			}
		}
		SECTION("with pairs metamethod") {
			int begintop = 0, endtop = 0;
			sol::state lua;
			test_stack_guard g(lua.lua_state(), begintop, endtop);
			lua.open_libraries(sol::lib::base);

			lua["t"] = sol::lua_value(lua, { { "hi", "meow" }, { 1, "bark" }, { ptr, "bleat" } });
			sol::table t = lua["t"];
			sol::table mt = lua.create_table();
			mt[sol::meta_function::pairs] = my_custom_pairs;
			t[sol::metatable_key] = mt;
			{
				int begintop2 = 0, endtop2 = 0;
				test_stack_guard g2(lua.lua_state(), begintop2, endtop2);
				for (auto& key_value_pair : t.pairs()) {
					verify_key_value(key_value_pair.first, key_value_pair.second);
				}
			}
		}
		SECTION("with no global next function available") {
			int begintop = 0, endtop = 0;
			sol::state lua;
			test_stack_guard g(lua.lua_state(), begintop, endtop);

			lua["t"] = sol::lua_value(lua, { { "hi", "meow" }, { 1, "bark" }, { ptr, "bleat" } });
			sol::table t = lua["t"];
			{
				int begintop2 = 0, endtop2 = 0;
				test_stack_guard g2(lua.lua_state(), begintop2, endtop2);
				int index = 0;
				for (auto& key_value_pair : t.pairs()) {
					if (index == 0) {
						lua.open_libraries(sol::lib::base);
					}
					verify_key_value(key_value_pair.first, key_value_pair.second);
					++index;
				}
			}
		}
	}
}
