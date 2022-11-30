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
#include <algorithm>
#include <numeric>
#include <vector>

std::string free_function() {
	INFO("free_function()");
	return "test";
}

struct object {
	std::string operator()() {
		INFO("member_test()");
		return "test";
	}
};

TEST_CASE("tables/cleanup", "make sure tables leave the stack balanced") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries();

	auto f = [] { return 5; };
	for (int i = 0; i < 30; i++) {
		std::string name = std::string("init") + std::to_string(i);
		int top = lua_gettop(lua);
		lua[name] = f;
		int aftertop = lua_gettop(lua);
		REQUIRE(aftertop == top);
		int val = lua[name]();
		REQUIRE(val == 5);
	}
}

TEST_CASE("tables/nested cleanup", "make sure tables leave the stack balanced") {
	sol::state lua;
	lua.open_libraries();

	lua.safe_script("A={}");
	auto f = [] { return 5; };
	for (int i = 0; i < 30; i++) {
		std::string name = std::string("init") + std::to_string(i);
		int top = lua_gettop(lua);
		auto A = lua["A"];
		int beforetop = lua_gettop(lua);
		REQUIRE(beforetop == top);
		A[name] = f;
		int aftertop = lua_gettop(lua);
		REQUIRE(aftertop == top);
		int val = A[name]();
		REQUIRE(val == 5);
	}
}

TEST_CASE("tables/variables", "Check if tables and variables work as intended") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::os);
	lua.get<sol::table>("os").set("name", "windows");
	{
		auto result = lua.safe_script("assert(os.name == \"windows\")", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
}

TEST_CASE("tables/create", "Check if creating a table is kosher") {
	sol::state lua;
	lua["testtable"] = sol::table::create(lua.lua_state(), 0, 0, "Woof", "Bark", 1, 2, 3, 4);
	sol::object testobj = lua["testtable"];
	REQUIRE(testobj.is<sol::table>());
	sol::table testtable = testobj.as<sol::table>();
	REQUIRE((testtable["Woof"] == std::string("Bark")));
	REQUIRE((testtable[1] == 2));
	REQUIRE((testtable[3] == 4));
}

TEST_CASE("tables/create local", "Check if creating a table is kosher") {
	sol::state lua;
	lua["testtable"] = lua.create_table(0, 0, "Woof", "Bark", 1, 2, 3, 4);
	sol::object testobj = lua["testtable"];
	REQUIRE(testobj.is<sol::table>());
	sol::table testtable = testobj.as<sol::table>();
	REQUIRE((testtable["Woof"] == std::string("Bark")));
	REQUIRE((testtable[1] == 2));
	REQUIRE((testtable[3] == 4));
}

TEST_CASE("tables/create local named", "Check if creating a table is kosher") {
	sol::state lua;
	sol::table testtable = lua.create_table("testtable", 0, 0, "Woof", "Bark", 1, 2, 3, 4);
	sol::object testobj = lua["testtable"];
	REQUIRE(testobj.is<sol::table>());
	REQUIRE((testobj == testtable));
	REQUIRE_FALSE((testobj != testtable));
	REQUIRE((testtable["Woof"] == std::string("Bark")));
	REQUIRE((testtable[1] == 2));
	REQUIRE((testtable[3] == 4));
}

TEST_CASE("tables/create-with-local", "Check if creating a table is kosher") {
	sol::state lua;
	lua["testtable"] = lua.create_table_with("Woof", "Bark", 1, 2, 3, 4);
	sol::object testobj = lua["testtable"];
	REQUIRE(testobj.is<sol::table>());
	sol::table testtable = testobj.as<sol::table>();
	REQUIRE((testtable["Woof"] == std::string("Bark")));
	REQUIRE((testtable[1] == 2));
	REQUIRE((testtable[3] == 4));
}

TEST_CASE("tables/function variables", "Check if tables and function calls work as intended") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::os);
	auto run_script = [](sol::state& lua) -> void {
		auto pf = lua.safe_script("assert(os.fun() == \"test\")");
		REQUIRE(pf.valid());
	};

	lua.get<sol::table>("os").set_function("fun", []() {
		INFO("stateless lambda()");
		return "test";
	});
	REQUIRE_NOTHROW(run_script(lua));

	lua.get<sol::table>("os").set_function("fun", &free_function);
	REQUIRE_NOTHROW(run_script(lua));

	// l-value, canNOT optimise
	// prefer value semantics unless wrapped with std::reference_wrapper
	{
		auto lval = object();
		lua.get<sol::table>("os").set_function("fun", &object::operator(), lval);
	}
	REQUIRE_NOTHROW(run_script(lua));

	auto reflval = object();
	lua.get<sol::table>("os").set_function("fun", &object::operator(), std::ref(reflval));
	REQUIRE_NOTHROW(run_script(lua));

	// stateful lambda: non-convertible, cannot be optimised
	int breakit = 50;
	lua.get<sol::table>("os").set_function("fun", [&breakit]() {
		INFO("stateful lambda() with breakit:" << breakit);
		return "test";
	});
	REQUIRE_NOTHROW(run_script(lua));

	// r-value, cannot optimise
	lua.get<sol::table>("os").set_function("fun", &object::operator(), object());
	REQUIRE_NOTHROW(run_script(lua));

	// r-value, cannot optimise
	auto rval = object();
	lua.get<sol::table>("os").set_function("fun", &object::operator(), std::move(rval));
	REQUIRE_NOTHROW(run_script(lua));
}

TEST_CASE("tables/add", "Basic test to make sure the 'add' feature works") {
	static const int sz = 120;

	sol::state lua;
	sol::table t = lua.create_table(sz, 0);

	std::vector<int> bigvec(sz);
	std::iota(bigvec.begin(), bigvec.end(), 1);

	for (std::size_t i = 0; i < bigvec.size(); ++i) {
		t.add(bigvec[i]);
	}
	for (std::size_t i = 0; i < bigvec.size(); ++i) {
		int val = t[i + 1];
		REQUIRE(val == bigvec[i]);
	}
}

TEST_CASE("tables/raw set and raw get", "ensure raw setting and getting works through metatables") {
	sol::state lua;
	sol::table t = lua.create_table();
	t[sol::metatable_key] = lua.create_table_with(
	     sol::meta_function::new_index,
	     [](lua_State* L) { return luaL_error(L, "nay"); },
	     sol::meta_function::index,
	     [](lua_State* L) { return luaL_error(L, "nay"); });
	t.raw_set("a", 2.5);
	double la = t.raw_get<double>("a");
	REQUIRE(la == 2.5);
}

TEST_CASE("tables/boolean keys", "make sure boolean keys don't get caught up in `is_integral` specializations") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.safe_script(R"(
tbl = {}
tbl[true] = 10
tbl[1] = 20

print(tbl[true])
print(tbl[1])
)");
	sol::table tbl = lua["tbl"];
	int v1 = tbl[true];
	int v2 = tbl[1];
	REQUIRE(v1 == 10);
	REQUIRE(v2 == 20);

	tbl[true] = 30;
	tbl[1] = 40;
	v1 = tbl[true];
	v2 = tbl[1];
	REQUIRE(v1 == 30);
	REQUIRE(v2 == 40);
}

TEST_CASE("tables/optional move", "ensure pushing a sol::optional<T> rvalue correctly moves the contained object into tables") {
	sol::state sol_state;
	struct move_only {
		int secret_code;
		move_only(int sc) : secret_code(sc) {
		}

		move_only(const move_only&) = delete;
		move_only(move_only&&) = default;
		move_only& operator=(const move_only&) = delete;
		move_only& operator=(move_only&&) = default;
	};
	sol_state["requires_move"] = sol::optional<move_only>(move_only(0x4D));
	REQUIRE(sol_state["requires_move"].get<move_only>().secret_code == 0x4D);
}

TEST_CASE("table/stack table size", "make sure size() works correctly on stack tables") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	sol::stack_table t(lua, sol::create);
	t[1] = 42;
	auto sz1 = t.size();
	REQUIRE(sz1 == 1);
	sol::stack::push(lua, "some string");
	auto sz2 = t.size();
	REQUIRE(sz2 == 1);
	std::string s = sol::stack::pop<std::string>(lua);
	REQUIRE(s == "some string");
	sol::table t2 = sol::stack::pop<sol::table>(lua);
	auto sz3 = t2.size();
	REQUIRE(sz1 == sz3);
	REQUIRE(sz1 == sz2);
}

TEST_CASE("table/proxy call", "test proxy calls put the variable in the right place") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	{
		sol::stack_guard tsg(lua);
		lua["t"] = std::initializer_list<std::pair<int, std::string_view>> { { 1, "borf" }, { 2, "bjork" }, { 3, "waf" } };
	}
	{
		sol::stack_guard fsg(lua);
		lua["f"] = [](std::string bjork) { REQUIRE(bjork == std::string("borf")); };
	}
	auto prox = lua["t"][1];
	lua["f"](prox);
}
