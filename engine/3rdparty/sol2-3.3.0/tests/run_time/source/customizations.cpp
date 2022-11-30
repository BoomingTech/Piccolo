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

#include <memory>
#include <unordered_map>
#include <vector>

struct custom {
	int bweh;

	static int get_calls;
	static int check_calls;
	static int check_get_calls;
	static int push_calls;
	static int exact_push_calls;
};

int custom::get_calls = 0;
int custom::check_calls = 0;
int custom::check_get_calls = 0;
int custom::push_calls = 0;
int custom::exact_push_calls = 0;

custom sol_lua_get(sol::types<custom>, lua_State* L, int index, sol::stack::record& tracking) {
	++custom::get_calls;
	return { sol::stack::get<int>(L, index, tracking) };
}

template <typename Handler>
bool sol_lua_check(sol::types<custom>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking) {
	++custom::check_calls;
	return sol::stack::check<int>(L, index, std::forward<Handler>(handler), tracking);
}

template <typename Handler>
sol::optional<custom> sol_lua_check_get(sol::types<custom> type_tag, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking) {
	++custom::check_get_calls;
	if (sol_lua_check(type_tag, L, index, std::forward<Handler>(handler), tracking)) {
		return sol_lua_get(type_tag, L, index, tracking);
	}
	return sol::nullopt;
}

int sol_lua_push(lua_State* L, const custom& c) {
	++custom::push_calls;
	return sol::stack::push(L, c.bweh);
}

int sol_lua_push(sol::types<custom>, lua_State* L, const custom& c) {
	++custom::exact_push_calls;
	return sol::stack::push(L, c.bweh);
}

struct multi_custom {
	int bweh;
	bool bwuh;
	std::string blah;

	static int get_calls;
	static int check_calls;
	static int check_get_calls;
	static int push_calls;
	static int exact_push_calls;
};

int multi_custom::get_calls = 0;
int multi_custom::check_calls = 0;
int multi_custom::check_get_calls = 0;
int multi_custom::push_calls = 0;
int multi_custom::exact_push_calls = 0;

multi_custom sol_lua_get(sol::types<multi_custom>, lua_State* L, int index, sol::stack::record& tracking) {
	++multi_custom::get_calls;
	return {
		sol::stack::get<int>(L, index + 0, tracking), sol::stack::get<bool>(L, index + 1, tracking), sol::stack::get<std::string>(L, index + 2, tracking)
	};
}

template <typename Handler>
bool sol_lua_check(sol::types<multi_custom>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking) {
	++multi_custom::check_calls;
	bool success = sol::stack::check<int>(L, index + 0, std::forward<Handler>(handler), tracking)
	     && sol::stack::check<bool>(L, index + 1, std::forward<Handler>(handler), tracking)
	     && sol::stack::check<std::string>(L, index + 2, std::forward<Handler>(handler), tracking);
	return success;
}

int sol_lua_push(lua_State* L, const multi_custom& c) {
	++multi_custom::push_calls;
	int p = sol::stack::push(L, c.bweh);
	p += sol::stack::push(L, c.bwuh);
	p += sol::stack::push(L, c.blah);
	return p;
}

struct super_custom {
	int bweh;

	static int get_calls;
	static int check_calls;
	static int check_get_calls;
	static int push_calls;
	static int exact_push_calls;
	static int pointer_push_calls;
};

int super_custom::get_calls = 0;
int super_custom::check_calls = 0;
int super_custom::check_get_calls = 0;
int super_custom::push_calls = 0;
int super_custom::pointer_push_calls = 0;
int super_custom::exact_push_calls = 0;



int destroy_super_custom(lua_State* L) {
	void* memory = lua_touserdata(L, 1);
	super_custom* data = static_cast<super_custom*>(memory);
	std::allocator<super_custom> alloc {};
	std::allocator_traits<std::allocator<super_custom>>::destroy(alloc, data);
	return 0;
}

super_custom* sol_lua_get(sol::types<super_custom*>, lua_State* L, int index, sol::stack::record& tracking) {
	++super_custom::get_calls;
	tracking.use(1);
	void* vp = lua_touserdata(L, index);
	super_custom** pscp = static_cast<super_custom**>(vp);
	return *pscp;
}

super_custom& sol_lua_get(sol::types<super_custom>, lua_State* L, int index, sol::stack::record& tracking) {
	return *sol_lua_get(sol::types<super_custom*>(), L, index, tracking);
}

template <typename Handler>
bool sol_lua_check(sol::types<super_custom>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking) {
	++super_custom::check_calls;
	tracking.use(1);
	if (luaL_testudata(L, index, "super_custom!") == nullptr) {
		if (luaL_testudata(L, index, "super_custom!p") == nullptr) {
			handler(L, index, sol::type::userdata, sol::type_of(L, index), "not a super_custom ?!");
			return false;
		}
	}
	return true;
}

template <typename Handler>
bool sol_lua_check(sol::types<super_custom*>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking) {
	return sol_lua_check(sol::types<super_custom>(), L, index, std::forward<Handler>(handler), tracking);
}

int sol_lua_push(lua_State* L, const super_custom& c) {
	++super_custom::push_calls;
	// ensure there's enough space for 1 more thing on the stack
	lua_checkstack(L, 1);
	// tell Lua we've left something on
	// the stack: return what comes from pushing an integer
	void* ud = lua_newuserdata(L, sizeof(super_custom*) + sizeof(super_custom));
	super_custom* tud = static_cast<super_custom*>(static_cast<void*>(static_cast<char*>(ud) + sizeof(super_custom*)));
	*static_cast<super_custom**>(ud) = tud;
	new (tud) super_custom(c);
	if (luaL_newmetatable(L, "super_custom!") == 1) {
		luaL_Reg l[2] {};
		l[0] = { sol::to_string(sol::meta_function::garbage_collect).c_str(), &destroy_super_custom };
		luaL_setfuncs(L, l, 0);
	}
	lua_setmetatable(L, -2);
	return 1;
}

int sol_lua_push(lua_State* L, super_custom* c) {
	++super_custom::pointer_push_calls;
	// ensure there's enough space for 1 more thing on the stack
	lua_checkstack(L, 1);
	// tell Lua we've left something on
	// the stack: return what comes from pushing an integer
	void* ud = lua_newuserdata(L, sizeof(super_custom*));
	*static_cast<super_custom**>(ud) = c;
	luaL_newmetatable(L, "super_custom!p");
	lua_setmetatable(L, -2);
	return 1;
}

int sol_lua_push(sol::types<std::reference_wrapper<super_custom>>, lua_State* L, std::reference_wrapper<super_custom> c) {
	++super_custom::exact_push_calls;
	return sol::stack::push(L, std::addressof(c.get()));
}

TEST_CASE("customization/adl", "using the ADL customization points in various situations") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	SECTION("value-based") {
		custom::get_calls = 0;
		custom::check_calls = 0;
		custom::check_get_calls = 0;
		custom::push_calls = 0;
		custom::exact_push_calls = 0;

		lua["meow"] = custom { 25 };
		custom meow = lua["meow"];

		REQUIRE(meow.bweh == 25);
		REQUIRE(custom::get_calls > 0);
		REQUIRE(custom::check_calls > 0);
		REQUIRE(custom::check_get_calls > 0);
		REQUIRE(custom::exact_push_calls > 0);
		REQUIRE(custom::push_calls == 0);
	}
	SECTION("multi") {
		multi_custom::get_calls = 0;
		multi_custom::check_calls = 0;
		multi_custom::check_get_calls = 0;
		multi_custom::push_calls = 0;
		multi_custom::exact_push_calls = 0;

		auto result = lua.safe_script("return function (a, b, c) return a, b, c end", sol::script_pass_on_error);
		REQUIRE(result.valid());
		sol::protected_function f = result.get<sol::protected_function>();
		multi_custom pass_through = f(multi_custom { 22, false, "miao" });
		REQUIRE(pass_through.bweh == 22);
		REQUIRE_FALSE(pass_through.bwuh);
		REQUIRE(pass_through.blah == "miao");

		REQUIRE(multi_custom::get_calls > 0);
		REQUIRE(multi_custom::check_calls > 0);
		REQUIRE(multi_custom::push_calls > 0);
		REQUIRE(multi_custom::check_get_calls == 0);
		REQUIRE(multi_custom::exact_push_calls == 0);
	}
	SECTION("reference-based") {
		super_custom::get_calls = 0;
		super_custom::check_calls = 0;
		super_custom::check_get_calls = 0;
		super_custom::push_calls = 0;
		super_custom::pointer_push_calls = 0;
		super_custom::exact_push_calls = 0;

		super_custom meow_original { 50 };
		lua["meow"] = std::ref(meow_original);
		super_custom& meow = lua["meow"];
		super_custom* p_meow = lua["meow"];
		std::reference_wrapper<super_custom> ref_meow = lua["meow"];
		super_custom meow_copy = lua["meow"];

		REQUIRE(meow.bweh == 50);
		REQUIRE(&meow == &meow_original);
		REQUIRE(p_meow == &meow_original);
		REQUIRE(&ref_meow.get() == &meow_original);
		REQUIRE(meow_copy.bweh == 50);
		REQUIRE(super_custom::get_calls > 0);
		REQUIRE(super_custom::check_calls > 0);
		REQUIRE(super_custom::check_get_calls == 0);
		REQUIRE(super_custom::push_calls == 0);
		REQUIRE(super_custom::pointer_push_calls > 0);
		REQUIRE(super_custom::exact_push_calls > 0);
	}
}
