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

#include <deque>
#include <set>
#include <functional>
#include <string>

struct self_cons_0 {
	self_cons_0(sol::variadic_args args, sol::this_state thisL) {
		lua_State* L = thisL;
		self_cons_0* pself = sol::stack::get<self_cons_0*>(L);
		REQUIRE(pself == this);
		REQUIRE(args.size() == 0);
	}
};

struct self_cons_1 {
	self_cons_1(sol::variadic_args args, sol::this_state thisL) {
		lua_State* L = thisL;
		self_cons_1* pself = sol::stack::get<self_cons_1*>(L, 1);
		REQUIRE(pself == this);
		REQUIRE(args.size() == 1);
	}
};

struct self_cons_2 {
	static void init_self_cons_2(self_cons_2& mem, sol::variadic_args args, sol::this_state thisL) {
		lua_State* L = thisL;
		self_cons_2* pself = sol::stack::get<self_cons_2*>(L, 1);
		std::allocator<self_cons_2> alloc {};
		std::allocator_traits<std::allocator<self_cons_2>>::construct(alloc, &mem);
		REQUIRE(pself == &mem);
		REQUIRE(args.size() == 2);
	}
};

struct self_cons_3 {
	static void init_self_cons_3(self_cons_3* mem, sol::variadic_args args, sol::this_state thisL) {
		lua_State* L = thisL;
		self_cons_3* pself = sol::stack::get<self_cons_3*>(L, 1);
		std::allocator<self_cons_3> alloc {};
		std::allocator_traits<std::allocator<self_cons_3>>::construct(alloc, mem);
		REQUIRE(pself == mem);
		REQUIRE(args.size() == 3);
	}
};

TEST_CASE("variadics/variadic_args", "Check to see we can receive multiple arguments through a variadic") {
	struct structure {
		int x;
		bool b;
	};

	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.set_function("v", [](sol::this_state, sol::variadic_args va) -> structure {
		int r = 0;
		for (auto v : va) {
			int value = v;
			r += value;
		}
		return { r, r > 200 };
	});

	lua.safe_script("x = v(25, 25)");
	lua.safe_script("x2 = v(25, 25, 100, 50, 250, 150)");
	lua.safe_script("x3 = v(1, 2, 3, 4, 5, 6)");

	structure& lx = lua["x"];
	structure& lx2 = lua["x2"];
	structure& lx3 = lua["x3"];
	REQUIRE(lx.x == 50);
	REQUIRE(lx2.x == 600);
	REQUIRE(lx3.x == 21);
	REQUIRE_FALSE(lx.b);
	REQUIRE(lx2.b);
	REQUIRE_FALSE(lx3.b);
}

TEST_CASE("variadics/required with variadic_args", "Check if a certain number of arguments can still be required even when using variadic_args") {
	sol::state lua;
	lua.set_function("v", [](sol::this_state, sol::variadic_args, int, int) {});
	{
		auto result = lua.safe_script("v(20, 25, 30)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("v(20, 25)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("v(20)", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
}

TEST_CASE("variadics/variadic_args get type", "Make sure we can inspect types proper from variadic_args") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.set_function("f", [](sol::variadic_args va) {
		sol::type types[] = { sol::type::number, sol::type::string, sol::type::boolean };
		bool working = true;
		auto b = va.begin();
		for (std::size_t i = 0; i < va.size(); ++i, ++b) {
			sol::type t1 = va.get_type(static_cast<std::ptrdiff_t>(i));
			sol::type t2 = b->get_type();
			working &= types[i] == t1;
			working &= types[i] == t2;
		}
		REQUIRE(working);
	});

	lua.safe_script("f(1, 'bark', true)");
	lua.safe_script("f(2, 'wuf', false)");
}

TEST_CASE("variadics/variadic_results", "returning a variable amount of arguments from C++") {
	SECTION("as_returns - containers") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		lua.set_function("f", []() {
			std::set<std::string> results { "arf", "bark", "woof" };
			return sol::as_returns(std::move(results));
		});
		lua.set_function("g", []() {
			static const std::deque<int> results { 25, 82 };
			return sol::as_returns(std::ref(results));
		});

		REQUIRE_NOTHROW([&]() {
			lua.safe_script(R"(
	v1, v2, v3 = f()
	v4, v5 = g()
)");
		}());

		std::string v1 = lua["v1"];
		std::string v2 = lua["v2"];
		std::string v3 = lua["v3"];
		int v4 = lua["v4"];
		int v5 = lua["v5"];

		REQUIRE(v1 == "arf");
		REQUIRE(v2 == "bark");
		REQUIRE(v3 == "woof");
		REQUIRE(v4 == 25);
		REQUIRE(v5 == 82);
	}
	SECTION("variadic_results - variadic_args") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		lua.set_function("f", [](sol::variadic_args args) { return sol::variadic_results(args.cbegin(), args.cend()); });

		auto result1 = lua.safe_script(R"(
			v1, v2, v3 = f(1, 'bark', true)
			v4, v5 = f(25, 82)
		)",
		     sol::script_pass_on_error);
		REQUIRE(result1.valid());

		int v1 = lua["v1"];
		std::string v2 = lua["v2"];
		bool v3 = lua["v3"];
		int v4 = lua["v4"];
		int v5 = lua["v5"];

		REQUIRE(v1 == 1);
		REQUIRE(v2 == "bark");
		REQUIRE(v3);
		REQUIRE(v4 == 25);
		REQUIRE(v5 == 82);
	}
	SECTION("variadic_results") {
		sol::state lua;
		sol::stack_guard luasg(lua);

		lua.set_function("f", [](sol::this_state ts, bool maybe) {
			if (maybe) {
				sol::variadic_results vr;
				vr.push_back({ ts, sol::in_place, 1 });
				vr.push_back({ ts, sol::in_place, 2 });
				vr.insert(vr.cend(), { ts, sol::in_place, 3 });
				return vr;
			}
			else {
				sol::variadic_results vr;
				vr.push_back({ ts, sol::in_place, "bark" });
				vr.push_back({ ts, sol::in_place, "woof" });
				vr.insert(vr.cend(), { ts, sol::in_place, "arf" });
				vr.push_back({ ts, sol::in_place, "borf" });
				return vr;
			}
		});

		auto result1 = lua.safe_script(R"(
			v1, v2, v3 = f(true)
			v4, v5, v6, v7 = f(false)
		)",
		     sol::script_pass_on_error);
		REQUIRE(result1.valid());

		int v1 = lua["v1"];
		int v2 = lua["v2"];
		int v3 = lua["v3"];
		std::string v4 = lua["v4"];
		std::string v5 = lua["v5"];
		std::string v6 = lua["v6"];
		std::string v7 = lua["v7"];

		REQUIRE(v1 == 1);
		REQUIRE(v2 == 2);
		REQUIRE(v3 == 3);
		REQUIRE(v4 == "bark");
		REQUIRE(v5 == "woof");
		REQUIRE(v6 == "arf");
		REQUIRE(v7 == "borf");
	}
}

TEST_CASE("variadics/fallback_constructor", "ensure constructor matching behaves properly in the presence of variadic fallbacks") {
	struct vec2x {
		float x = 0, y = 0;
	};

	sol::state lua;

	lua.new_usertype<vec2x>("vec2x",
	     sol::call_constructor,
	     sol::factories([]() { return vec2x {}; },
	          [](vec2x const& v) -> vec2x { return v; },
	          [](sol::variadic_args va) {
		          vec2x res {};
		          if (va.size() == 1) {
			          res.x = va[0].get<float>();
			          res.y = va[0].get<float>();
		          }
		          else if (va.size() == 2) {
			          res.x = va[0].get<float>();
			          res.y = va[1].get<float>();
		          }
		          else {
			          throw sol::error("invalid args");
		          }
		          return res;
	          }));

	auto result1 = lua.safe_script("v0 = vec2x();", sol::script_pass_on_error);
	auto result2 = lua.safe_script("v1 = vec2x(1);", sol::script_pass_on_error);
	auto result3 = lua.safe_script("v2 = vec2x(1, 2);", sol::script_pass_on_error);
	auto result4 = lua.safe_script("v3 = vec2x(v2)", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	REQUIRE(result2.valid());
	REQUIRE(result3.valid());
	REQUIRE(result4.valid());

	vec2x& v0 = lua["v0"];
	vec2x& v1 = lua["v1"];
	vec2x& v2 = lua["v2"];
	vec2x& v3 = lua["v3"];

	REQUIRE(v0.x == 0);
	REQUIRE(v0.y == 0);
	REQUIRE(v1.x == 1);
	REQUIRE(v1.y == 1);
	REQUIRE(v2.x == 1);
	REQUIRE(v2.y == 2);
	REQUIRE(v3.x == v2.x);
	REQUIRE(v3.y == v2.y);
}

TEST_CASE("variadics/self_test", "test argument count and self object reference") {

	sol::state lua;

	lua.open_libraries();
	lua.new_usertype<self_cons_0>("foo0", sol::constructors<self_cons_0(sol::variadic_args, sol::this_state)>());
	lua.new_usertype<self_cons_1>("foo1", sol::constructors<self_cons_1(sol::variadic_args, sol::this_state)>());
	lua.new_usertype<self_cons_2>("foo2", "new", sol::initializers(&self_cons_2::init_self_cons_2));
	lua.new_usertype<self_cons_3>("foo3", "new", sol::initializers(&self_cons_3::init_self_cons_3));

	sol::optional<sol::error> maybe_err = lua.safe_script(R"(
local obj0 = foo0.new()
local obj1 = foo1.new(0)
local obj2 = foo2.new(0, 1)
local obj3 = foo3.new(0, 1, 2)
)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_err.has_value());
}

TEST_CASE("variadics/overloads with fallbacks",
     "Test that 'fuzzy' types like optional and variadic_args can coexist and have optional numbers of arguments passed to them") {

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	auto test = lua[u8"test"].get_or_create<sol::table>();
	auto derp = test[u8"derp"].get_or_create<sol::table>();

	derp[u8"herp"]
	     = sol::overload([](sol::this_state, const std::string_view&, const std::string_view&, const std::string_view&, sol::optional<uint32_t>) { return 1; },
	          [](const std::string_view&, const std::string_view&, sol::function, sol::optional<uint32_t>) { return 2; },
	          [](sol::this_state, sol::variadic_args) { return 3; });

	const std::string_view code = R"(
assert(test.derp.herp('str1', 'str2', 'str3') == 1);
assert(test.derp.herp('str1', 'str2', 'str3', 42) == 1);
assert(test.derp.herp('str1', 'str2', 'str3', nil) == 1);

assert(test.derp.herp('str1', 'str2', function(r) end) == 2);
assert(test.derp.herp('str1', 'str2', function(r) end, 42) == 2);
assert(test.derp.herp('str1', 'str2', function(r) end, nil) == 2);

assert(test.derp.herp('str1', 'str2', 'str3', {}) == 3);
assert(test.derp.herp('str1', 'str2', function(r) end, {}) == 3);
assert(test.derp.herp(1, 2, 3, 4, 5, 6, 7) == 3);
assert(test.derp.herp('str1', 'str2', 'str3', 'str4') == 3);
)";
	sol::optional<sol::error> maybe_error = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error.has_value());
}
