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

// we have a test for weirdly aligned wrappers
// do not make lots of noise about it
#ifdef _MSC_VER
#pragma warning(disable : 4324)
#endif

#include "sol_test.hpp"

#include "common_classes.hpp"

#include <catch2/catch_all.hpp>

bool something_func_true() {
	return true;
}

struct ext_getset {

	int bark = 24;
	const int meow = 56;

	ext_getset() = default;
	ext_getset(int v) : bark(v) {
	}
	ext_getset(ext_getset&&) = default;
	ext_getset(const ext_getset&) = delete;
	ext_getset& operator=(ext_getset&&) = delete;
	ext_getset& operator=(const ext_getset&) = delete;
	~ext_getset() {
	}

	std::string x() {
		return "bark bark bark";
	}

	int x2(std::string x) {
		return static_cast<int>(x.length());
	}

	void set(sol::variadic_args, sol::this_state, int x) {
		bark = x;
	}

	int get(sol::this_state, sol::variadic_args) {
		return bark;
	}

	static void s_set(int) {
	}

	static int s_get(int x) {
		return x + 20;
	}
};

template <typename T>
void des(T& e) {
	e.~T();
}

template <typename SelfType>
struct alignas(16) weird_aligned_wrapper {
	template <typename F, std::enable_if_t<!std::is_same_v<weird_aligned_wrapper, F>, std::nullptr_t> = nullptr>
	weird_aligned_wrapper(F&& f) : lambda(std::forward<F>(f)) {
	}
	void operator()(SelfType& self, sol::object param) const {
		lambda(self, param.as<float>());
	}
	std::function<void(SelfType&, float)> lambda;
};

TEST_CASE("usertype/properties", "Check if member properties/variables work") {
	struct bark {
		int var = 50;
		int var2 = 25;

		int get_var2() const {
			return var2;
		}

		int get_var3() {
			return var2;
		}

		void set_var2(int x) {
			var2 = x;
		}
	};

	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<bark>("bark",
	     "var",
	     &bark::var,
	     "var2",
	     sol::readonly(&bark::var2),
	     "a",
	     sol::property(&bark::get_var2, &bark::set_var2),
	     "b",
	     sol::property(&bark::get_var2),
	     "c",
	     sol::property(&bark::get_var3),
	     "d",
	     sol::property(&bark::set_var2));

	bark b;
	lua.set("b", &b);

	lua.safe_script("b.a = 59");
	lua.safe_script("var2_0 = b.a");
	lua.safe_script("var2_1 = b.b");
	lua.safe_script("b.d = 1568");
	lua.safe_script("var2_2 = b.c");

	int var2_0 = lua["var2_0"];
	int var2_1 = lua["var2_1"];
	int var2_2 = lua["var2_2"];
	REQUIRE(var2_0 == 59);
	REQUIRE(var2_1 == 59);
	REQUIRE(var2_2 == 1568);

	{
		auto result = lua.safe_script("b.var2 = 24", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
	{
		auto result = lua.safe_script("r = b.d", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
	{
		auto result = lua.safe_script("r = b.d", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
	{
		auto result = lua.safe_script("b.b = 25", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
	{
		auto result = lua.safe_script("b.c = 11", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
}

TEST_CASE("usertype/copyability", "make sure user can write to a class variable even if the class itself isn't copy-safe") {
	struct NoCopy {
		int get() const {
			return _you_can_copy_me;
		}
		void set(int val) {
			_you_can_copy_me = val;
		}

		int _you_can_copy_me;
		non_copyable _haha_you_cant_copy_me;
	};

	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.new_usertype<NoCopy>("NoCopy", "val", sol::property(&NoCopy::get, &NoCopy::set));

	REQUIRE_NOTHROW(lua.safe_script(R"__(
nocopy = NoCopy.new()
nocopy.val = 5
               )__"));
}

TEST_CASE("usertype/protect", "users should be allowed to manually protect a function") {
	struct protect_me {
		int gen(int x) {
			return x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<protect_me>("protect_me", "gen", sol::protect(&protect_me::gen));

	REQUIRE_NOTHROW(lua.safe_script(R"__(
pm = protect_me.new()
value = pcall(pm.gen,pm)
)__"));
	bool value = lua["value"];
	REQUIRE_FALSE(value);
}

TEST_CASE("usertype/static-properties", "allow for static functions to get and set things as a property") {
	static int b = 50;
	struct test_t {
		static double s_func() {
			return b + 0.5;
		}

		static void g_func(int v) {
			b = v;
		}

		std::size_t func() {
			return 24;
		}
	};
	test_t manager;

	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.new_usertype<test_t>(
	     "test", "f", std::function<std::size_t()>(std::bind(std::mem_fn(&test_t::func), &manager)), "g", sol::property(&test_t::s_func, &test_t::g_func));

	lua.safe_script("v1 = test.f()");
	lua.safe_script("v2 = test.g");
	lua.safe_script("test.g = 60");
	lua.safe_script("v2a = test.g");
	int v1 = lua["v1"];
	REQUIRE(v1 == 24);
	double v2 = lua["v2"];
	REQUIRE(v2 == 50.5);
	double v2a = lua["v2a"];
	REQUIRE(v2a == 60.5);
}

TEST_CASE("usertype/var with string literals", "String literals are the bane of my existence and one day C++ will make them not be fucking arrays") {
	struct blah { };

	sol::state lua;
	sol::usertype<blah> x = lua.new_usertype<blah>("blah");
	x["__className"] = sol::var("Entity");

	std::string cxx_name = x["__className"];
	std::string lua_name = lua.script("return blah.__className");
	REQUIRE(cxx_name == lua_name);
	REQUIRE(cxx_name == "Entity");
	REQUIRE(lua_name == "Entity");
}

TEST_CASE("usertype/var-and-property", "make sure const vars are readonly and properties can handle lambdas") {
	const static int arf = 20;

	struct test {
		int value = 10;
	};

	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries();

	lua.new_usertype<test>(
	     "test", "prop", sol::property([](test& t) { return t.value; }, [](test& t, int x) { t.value = x; }), "global", sol::var(std::ref(arf)));

	lua.safe_script(R"(
t = test.new()
print(t.prop)
t.prop = 50
print(t.prop)
	)");

	test& t = lua["t"];
	REQUIRE(t.value == 50);

	lua.safe_script(R"(
t = test.new()
print(t.global)
	)");
	{
		auto result = lua.safe_script("t.global = 20", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
	lua.safe_script("print(t.global)");
}

TEST_CASE("usertype/coverage", "try all the things") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<ext_getset>("ext_getset",
	     sol::call_constructor,
	     sol::constructors<sol::types<>, sol::types<int>>(),
	     sol::meta_function::garbage_collect,
	     sol::destructor(des<ext_getset>),
	     "x",
	     sol::overload(&ext_getset::x, &ext_getset::x2, [](ext_getset& m, std::string x, int y) { return m.meow + 50 + y + static_cast<int>(x.length()); }),
	     "bark",
	     &ext_getset::bark,
	     "meow",
	     &ext_getset::meow,
	     "readonlybark",
	     sol::readonly(&ext_getset::bark),
	     "set",
	     &ext_getset::set,
	     "get",
	     &ext_getset::get,
	     "sset",
	     &ext_getset::s_set,
	     "sget",
	     &ext_getset::s_get,
	     "propbark",
	     sol::property(&ext_getset::set, &ext_getset::get),
	     "readonlypropbark",
	     sol::property(&ext_getset::get),
	     "writeonlypropbark",
	     sol::property(&ext_getset::set));

	INFO("usertype created");

	lua.safe_script(R"(
e = ext_getset()
w = e:x(e:x(), e:x(e:x()))
print(w)
)");

	int w = lua["w"];
	REQUIRE(w == (56 + 50 + 14 + 14));

	INFO("REQUIRE(w) successful");

	lua.safe_script(R"(
e:set(500)
e.sset(24)
x = e:get()
y = e.sget(20)
)");

	int x = lua["x"];
	int y = lua["y"];
	REQUIRE(x == 500);
	REQUIRE(y == 40);

	INFO("REQUIRE(x, y) successful");

	lua.safe_script(R"(
e.bark = 5001
a = e:get()
print(e.bark)
print(a)

e.propbark = 9700
b = e:get()
print(e.propbark)
print(b)
)");
	int a = lua["a"];
	int b = lua["b"];

	REQUIRE(a == 5001);
	REQUIRE(b == 9700);

	INFO("REQUIRE(a, b) successful");

	lua.safe_script(R"(
c = e.readonlybark
d = e.meow
print(e.readonlybark)
print(c)
print(e.meow)
print(d)
)");

	int c = lua["c"];
	int d = lua["d"];
	REQUIRE(c == 9700);
	REQUIRE(d == 56);

	INFO("REQUIRE(c, d) successful");

	lua.safe_script(R"(
e.writeonlypropbark = 500
z = e.readonlypropbark
print(e.readonlybark)
print(e.bark)
)");

	int z = lua["z"];
	REQUIRE(z == 500);

	INFO("REQUIRE(z) successful");
	{
		auto result = lua.safe_script("e.readonlybark = 24", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
		INFO("REQUIRE_FALSE 1 successful");
	}
	{
		auto result = lua.safe_script("e.readonlypropbark = 500", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
		INFO("REQUIRE_FALSE 2 successful");
	}
	{
		auto result = lua.safe_script("y = e.writeonlypropbark", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
		INFO("REQUIRE_FALSE 3 successful");
	}
}

TEST_CASE("usertype/alignment", "ensure that alignment does not trigger weird aliasing issues") {
	struct aligned_base { };
	struct aligned_derived : aligned_base { };

	sol::state lua;
	sol::stack_guard luasg(lua);

	auto f = [](aligned_base&, float d) { REQUIRE(d == 5.0f); };
	lua.new_usertype<aligned_base>("Base", "x", sol::writeonly_property(weird_aligned_wrapper<aligned_base>(std::ref(f))));
	lua.new_usertype<aligned_derived>("Derived", sol::base_classes, sol::bases<aligned_base>());

	aligned_derived d;
	lua["d"] = d;

	auto result = lua.safe_script("d.x = 5");
	REQUIRE(result.valid());
}

TEST_CASE("usertype/readonly-and-static-functions", "Check if static functions can be called on userdata and from their originating (meta)tables") {
	struct bark {
		int var = 50;

		void func() {
		}

		static void oh_boy() {
		}

		static int oh_boy(std::string name) {
			return static_cast<int>(name.length());
		}

		int operator()(int x) {
			return x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<bark>(
	     "bark",
	     "var",
	     &bark::var,
	     "var2",
	     sol::readonly(&bark::var),
	     "something",
	     something_func_true,
	     "something2",
	     [](int x, int y) { return x + y; },
	     "func",
	     &bark::func,
	     "oh_boy",
	     sol::overload(sol::resolve<void()>(&bark::oh_boy), sol::resolve<int(std::string)>(&bark::oh_boy)),
	     sol::meta_function::call_function,
	     &bark::operator());

	{
		auto result = lua.safe_script("assert(bark.oh_boy('woo') == 3)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("bark.oh_boy()", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}

	bark b;
	lua.set("b", &b);

	sol::table b_table = lua["b"];
	sol::function member_func = b_table["func"];
	sol::function s = b_table["something"];
	sol::function s2 = b_table["something2"];

	sol::table b_metatable = b_table[sol::metatable_key];
	bool isvalidmt = b_metatable.valid();
	REQUIRE(isvalidmt);
	sol::function b_call = b_metatable["__call"];
	sol::function b_as_function = lua["b"];

	int x = b_as_function(1);
	int y = b_call(b, 1);
	bool z = s();
	int w = s2(2, 3);
	REQUIRE(x == 1);
	REQUIRE(y == 1);
	REQUIRE(z);
	REQUIRE(w == 5);

	lua.safe_script(R"(
lx = b(1)
ly = getmetatable(b).__call(b, 1)
lz = b.something()
lz2 = bark.something()
lw = b.something2(2, 3)
lw2 = bark.something2(2, 3)
    )");

	int lx = lua["lx"];
	int ly = lua["ly"];
	bool lz = lua["lz"];
	int lw = lua["lw"];
	bool lz2 = lua["lz2"];
	int lw2 = lua["lw2"];
	REQUIRE(lx == 1);
	REQUIRE(ly == 1);
	REQUIRE(lz);
	REQUIRE(lz2);
	REQUIRE(lw == 5);
	REQUIRE(lw2 == 5);
	REQUIRE(lx == ly);
	REQUIRE(lz == lz2);
	REQUIRE(lw == lw2);

	auto result = lua.safe_script("b.var2 = 2", sol::script_pass_on_error);
	REQUIRE_FALSE(result.valid());
}

TEST_CASE("usertype/vars", "usertype vars can bind various class items") {
	static int muh_variable = 25;
	static int through_variable = 10;

	sol::state lua;
	lua.open_libraries();
	struct test { };
	lua.new_usertype<test>("test",
	     "straight",
	     sol::var(2),
	     "global",
	     sol::var(muh_variable),
	     "ref_global",
	     sol::var(std::ref(muh_variable)),
	     "global2",
	     sol::var(through_variable),
	     "ref_global2",
	     sol::var(std::ref(through_variable)));

	int prets = lua["test"]["straight"];
	int pretg = lua["test"]["global"];
	int pretrg = lua["test"]["ref_global"];
	int pretg2 = lua["test"]["global2"];
	int pretrg2 = lua["test"]["ref_global2"];

	REQUIRE(prets == 2);
	REQUIRE(pretg == 25);
	REQUIRE(pretrg == 25);
	REQUIRE(pretg2 == 10);
	REQUIRE(pretrg2 == 10);

	lua.safe_script(R"(
print(test.straight)
test.straight = 50
print(test.straight)
)");
	int s = lua["test"]["straight"];
	REQUIRE(s == 50);

	lua.safe_script(R"(
t = test.new()
print(t.global)
t.global = 50
print(t.global)
)");
	int mv = lua["test"]["global"];
	REQUIRE(mv == 50);
	REQUIRE(muh_variable == 25);

	lua.safe_script(R"(
print(t.ref_global)
t.ref_global = 50
print(t.ref_global)
)");
	int rmv = lua["test"]["ref_global"];
	REQUIRE(rmv == 50);
	REQUIRE(muh_variable == 50);

	REQUIRE(through_variable == 10);
	lua.safe_script(R"(
print(test.global2)
test.global2 = 35
print(test.global2)
)");
	int tv = lua["test"]["global2"];
	REQUIRE(through_variable == 10);
	REQUIRE(tv == 35);

	lua.safe_script(R"(
print(test.ref_global2)
test.ref_global2 = 35
print(test.ref_global2)
)");
	int rtv = lua["test"]["ref_global2"];
	REQUIRE(rtv == 35);
	REQUIRE(through_variable == 35);
}
