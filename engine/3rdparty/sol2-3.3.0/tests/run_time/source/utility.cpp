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

#include <mutex>
#include <thread>
#include <string_view>
#include <variant>

struct utility_counter {
	sol::stack_count count(sol::this_state s) {
		return sol::stack::multi_push(s, 1, 2, 3, 4);
	};
};

struct optional_ref_t {
	int x = 0;
};

std::mutex basic_init_require_mutex;

void basic_initialization_and_lib_open() {
	sol::state lua;
	try {
		lua.open_libraries();
		lua["a"] = 24;
		int a = lua["a"];
		{
			std::lock_guard<std::mutex> lg(basic_init_require_mutex);
			REQUIRE(a == 24);
		}
	}
	catch (const sol::error& e) {
		std::lock_guard<std::mutex> lg(basic_init_require_mutex);
		INFO(e.what());
		REQUIRE(false);
	}
	catch (...) {
		std::lock_guard<std::mutex> lg(basic_init_require_mutex);
		REQUIRE(false);
	}
}

TEST_CASE("utility/variant", "test that variant can be round-tripped") {
	SECTION("okay") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.set_function("f", [](int v) { return v == 2; });
		lua.set_function("g", [](std::variant<int, float, std::string> vv) {
			int v = std::get<int>(vv);
			return v == 2;
		});
		lua["v"] = std::variant<int, float, std::string>(2);
		{
			auto result = lua.safe_script("assert(f(v))", sol::script_pass_on_error);
			REQUIRE(result.valid());
		};
		{
			auto result = lua.safe_script("assert(g(v))", sol::script_pass_on_error);
			REQUIRE(result.valid());
		};
	}
	SECTION("throws") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.set_function("f", [](int v) { return v == 2; });
		lua.set_function("g", [](std::variant<float, int, std::string> vv) {
			int v = std::get<int>(vv);
			return v == 2;
		});
		lua["v"] = std::variant<float, int, std::string>(std::string("bark"));
		{
			auto result = lua.safe_script("assert(f(v))", sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};
		{
			auto result = lua.safe_script("assert(g(v))", sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};
	}
}

namespace detail {
	template <typename T>
	struct optional_rebinder;

	template <template <class> class Optional, typename T>
	struct optional_rebinder<Optional<T>> {
		template <typename U>
		using type = Optional<U>;
	};

	struct dummy { };
} // namespace detail

#define Optional typename detail::optional_rebinder<TestType>::template type

using OptionalsToTest = std::tuple<sol::optional<detail::dummy>, std::optional<detail::dummy>>;

TEMPLATE_LIST_TEST_CASE("utility/optional-conversion", "test that regular optional will properly catch certain types", OptionalsToTest) {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<vars>("vars");

	lua["test"] = [](Optional<vars> x) { return static_cast<bool>(x); };

	const auto result = lua.safe_script(R"(
		assert(test(vars:new()))
		assert(not test(3))
		assert(not test(nil))
	)",
	     sol::script_pass_on_error);
	REQUIRE(result.valid());
}

TEMPLATE_LIST_TEST_CASE("utility/optional-value-or", "test that regular optional will properly handle value_or", OptionalsToTest) {
	Optional<std::string> str;
	auto x = str.value_or("!");

	Optional<unsigned int> un;
	auto y = un.value_or(64);

	optional_ref_t def_custom;
	sol::optional<optional_ref_t&> custom;
	auto z = custom.value_or(def_custom);
	const auto& z_ref = custom.value_or(def_custom);

	REQUIRE(x == "!");

	REQUIRE(y == 64);

	REQUIRE(&def_custom == &z_ref);
	REQUIRE(&z != &def_custom);
}

TEMPLATE_LIST_TEST_CASE("utility/optional", "test that shit optional can be round-tripped", OptionalsToTest) {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	SECTION("okay") {
		lua.set_function("f", [](int v) { return v == 2; });
		lua.set_function("g", [](Optional<int> vv) { return vv && *vv == 2; });
		lua.set_function("h", [](Optional<sol::object> v) { return !v; });
		lua["v"] = Optional<int>(2);
		{
			auto result = lua.safe_script("assert(f(v))", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(g(v))", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
		{
			auto result = lua.safe_script("assert(h())", sol::script_pass_on_error);
			REQUIRE(result.valid());
		}
	}
	SECTION("throws") {
		lua.set_function("f", [](int v) { return v == 2; });
		lua.set_function("g", [](Optional<int> vv) { return vv && *vv == 2; });
		lua["v"] = Optional<int> {};
		{
			auto result = lua.safe_script("assert(f(v))", sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};
		{
			auto result = lua.safe_script("assert(g(v))", sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};
	}
	SECTION("in classes") {
		struct opt_c {
			Optional<int> member;
		};

		auto uto = lua.new_usertype<opt_c>("opt_c", "value", &opt_c::member);

		opt_c obj;
		lua["obj"] = std::ref(obj);

		lua.safe_script("print(obj.value) obj.value = 20  print(obj.value)");
		REQUIRE(obj.member == 20);
		lua.safe_script("print(obj.value) obj.value = nil print(obj.value)");
		REQUIRE(!obj.member);
	}
	SECTION("accepts nil") {
		lua.set_function("f_int", [](Optional<int> v) { return !v; });
		lua.set_function("f_object", [](Optional<sol::object> v) { return !v; });
		lua.set_function("f_table", [](Optional<sol::table> v) { return !v; });
		lua.set_function("f_function", [](Optional<sol::function> v) { return !v; });

		REQUIRE(lua.safe_script("assert(f_int())").valid());
		REQUIRE(lua.safe_script("assert(f_object())").valid());
		REQUIRE(lua.safe_script("assert(f_table())").valid());
		REQUIRE(lua.safe_script("assert(f_function())").valid());
	}
	SECTION("returns nil") {
		lua.set_function("f_int", [] { return Optional<int> {}; });
		lua.set_function("f_object", [] { return Optional<sol::object> {}; });
		lua.set_function("f_table", [] { return Optional<sol::table> {}; });
		lua.set_function("f_function", [] { return Optional<sol::function> {}; });

		REQUIRE(lua.safe_script("assert(not f_int())").valid());
		REQUIRE(lua.safe_script("assert(not f_object())").valid());
		REQUIRE(lua.safe_script("assert(not f_table())").valid());
		REQUIRE(lua.safe_script("assert(not f_function())").valid());
	}
}
#undef Optional

TEST_CASE("utility/string_view", "test that string_view can be taken as an argument") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.set_function("f", [](std::string_view v) { return v == "bark!"; });
	lua["v"] = "bark!";

	REQUIRE_NOTHROW([&]() { lua.safe_script("assert(f(v))"); }());
}

TEST_CASE("utility/thread", "fire up lots of threads at the same time to make sure the initialization changes do not cause horrible crashing data races") {
	REQUIRE_NOTHROW([]() {
		std::thread thrds[16];
		for (int i = 0; i < 16; i++) {
			thrds[i] = std::thread(&basic_initialization_and_lib_open);
		}

		for (int i = 0; i < 16; i++) {
			thrds[i].join();
		}
	}());
}

TEST_CASE("utility/pointer", "check we can get pointer value from references") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.set_function("f", [](bool aorb, sol::reference a, sol::stack_reference b) {
		if (aorb) {
			return a.pointer();
		}
		return b.pointer();
	});
	auto result0 = lua.safe_script("v0 = 'hi'", sol::script_pass_on_error);
	REQUIRE(result0.valid());
	auto result1 = lua.safe_script("v1 = f(true, v0)", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("v2 = f(false, nil, v0)", sol::script_pass_on_error);
	REQUIRE(result2.valid());
	const void* ap = lua["v1"];
	const void* bp = lua["v2"];
	REQUIRE(ap == bp);
}

TEST_CASE("utility/this_state", "Ensure this_state argument can be gotten anywhere in the function.") {
	struct bark {
		int with_state(sol::this_state l, int a, int b) {
			lua_State* L = l;
			int c = lua_gettop(L);
			return a + b + (c - c);
		}

		static int with_state_2(int a, sol::this_state l, int b) {
			INFO("inside with_state_2");
			lua_State* L = l;
			INFO("L is" << (void*)L);
			int c = lua_gettop(L);
			return a * b + (c - c);
		}
	};

	sol::state lua;
	sol::stack_guard luasg(lua);

	INFO("created lua state");
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<bark>("bark", "with_state", &bark::with_state);

	INFO("setting b and with_state_2");
	bark b;
	lua.set("b", &b);
	lua.set("with_state_2", bark::with_state_2);
	INFO("finished setting");
	INFO("getting fx");
	sol::function fx = lua["with_state_2"];
	INFO("calling fx");
	int a = fx(25, 25);
	INFO("finished setting fx");
	INFO("calling a script");
	lua.safe_script("a = with_state_2(25, 25)");
	INFO("calling c script");
	lua.safe_script("c = b:with_state(25, 25)");
	INFO("getting a");
	int la = lua["a"];
	INFO("getting b");
	int lc = lua["c"];

	REQUIRE(lc == 50);
	REQUIRE(a == 625);
	REQUIRE(la == 625);
}

TEST_CASE("safety/check_stack", "check to make sure that if we overflow the stack in safety mode, we get the appropriate error thrown") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua["okay"] = []() {
		// clang-format off
		return std::make_tuple(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37);
		// clang-format on
	};
	lua["bad"] = [](lua_State* L) {
		constexpr int max_stack_guess = 1000000 * 2 * 4;
		int p = 0;
		for (std::size_t i = 0; i < max_stack_guess; ++i) {
			p += sol::stack::push(L, i);
		}
		return p;
	};
	auto result1 = lua.safe_script("okay()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("bad()", sol::script_pass_on_error);
	REQUIRE_FALSE(result2.valid());
}

TEST_CASE("stack/stack_count", "Ensure that the stck count type is treated properly and does not obliterate the stack values on retuurn.") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("count", &utility_counter::count, utility_counter {});

	sol::optional<sol::error> result0 = lua.safe_script(R"(
		local x, y, z, w = count()
		assert(x == 1)
		assert(y == 2)
		assert(z == 3)
		assert(w == 4)
	)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(result0.has_value());
}
