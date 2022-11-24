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

#include <iostream>
#include <list>
#include <memory>

inline namespace sol2_tests_usertypes {
	struct self_test {
		int bark;

		self_test() : bark(100) {
		}

		void g(const std::string& str) {
			std::cout << str << '\n';
			bark += 1;
		}

		void f(const self_test& t) {
			std::cout << "got test" << '\n';
			if (t.bark != bark)
				throw sol::error("bark values are not the same for self_test f function");
			if (&t != this)
				throw sol::error("call does not reference self for self_test f function");
		}
	};

	class Foobar {
	public:
		int GetValue(int index) const {
			return 1234 + index;
		}
	};

	class Doodah {
	public:
		int GetValue(int index) const {
			return 2345 + index;
		}
	};

	struct T {
		static int noexcept_function() noexcept {
			return 0x61;
		}

		int noexcept_method() noexcept {
			return 0x62;
		}
	};

	struct A {
		double f = 25.5;

		static void init(A& x, double f) {
			x.f = f;
		}
	};

	class B {
	public:
		int bvar = 24;
	};

	struct my_thing { };

} // namespace sol2_tests_usertypes

TEST_CASE("usertype/self-referential usertype", "usertype classes must play nice when C++ object types are requested for C++ code") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<self_test>("test", "g", &self_test::g, "f", &self_test::f);

	auto result = lua.safe_script(
	     "local a = test.new()\n"
	     "a:g(\"woof\")\n"
	     "a:f(a)\n",
	     sol::script_pass_on_error);
	REQUIRE(result.valid());
}

TEST_CASE("usertype/nonmember-functions", "let users set non-member functions that take unqualified T as first parameter to usertype") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<giver>(
	     "giver", "gief_stuff", giver::gief_stuff, "gief", &giver::gief, "__tostring", [](const giver& t) { return std::to_string(t.a) + ": giving value"; });
	lua.get<sol::table>("giver").set_function("stuff", giver::stuff);

	{
		auto result = lua.safe_script("assert(giver.stuff() == 97)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script(
		     "t = giver.new()\n"
		     "print(tostring(t))\n"
		     "t:gief()\n"
		     "t:gief_stuff(20)\n",
		     sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	giver& g = lua.get<giver>("t");
	REQUIRE(g.a == 20);
	std::cout << "----- end of 1" << std::endl;
}

TEST_CASE("usertype/abstract-base-class", "Ensure that abstract base classes and such can be registered") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.new_usertype<abstract_A>("A", "a", &abstract_A::a);
	lua.new_usertype<abstract_B>("B", sol::base_classes, sol::bases<abstract_A>());
	REQUIRE_NOTHROW([&]() {
		lua.safe_script(R"(
local b = B.new()
b:a()
		)");
	});
}

TEST_CASE("usertype/as_function", "Ensure that variables can be turned into functions by as_function") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries();

	lua.new_usertype<B>("B", "b", &B::bvar, "f", sol::as_function(&B::bvar));

	B b;
	lua.set("b", &b);
	lua.safe_script("x = b:f()");
	lua.safe_script("y = b.b");
	int x = lua["x"];
	int y = lua["y"];
	REQUIRE(x == 24);
	REQUIRE(y == 24);
}

TEST_CASE("usertype/call-initializers", "Ensure call constructors with initializers work well") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries();

	lua.new_usertype<A>("A", sol::call_constructor, sol::initializers(&A::init));

	lua.safe_script(R"(
a = A(24.3)
)");
	A& a = lua["a"];
	REQUIRE(a.f == 24.3);
}

TEST_CASE("usertype/missing-key", "make sure a missing key returns nil") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<thing>("thing");
	{
		auto result = lua.safe_script("v = thing.missingKey\nprint(v)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	sol::object o = lua["v"];
	bool isnil = o.is<sol::lua_nil_t>();
	REQUIRE(isnil);
}

TEST_CASE("usertype/basic type information", "check that we can query some basic type information") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<my_thing>("my_thing");

	lua.safe_script("obj = my_thing.new()");

	lua.safe_script("assert(my_thing.__type.is(obj))");
	lua.safe_script("assert(not my_thing.__type.is(1))");
	lua.safe_script("assert(not my_thing.__type.is(\"not a thing\"))");
	lua.safe_script("print(my_thing.__type.name)");

	lua.safe_script("assert(obj.__type.is(obj))");
	lua.safe_script("assert(not obj.__type.is(1))");
	lua.safe_script("assert(not obj.__type.is(\"not a thing\"))");
	lua.safe_script("print(obj.__type.name)");

	lua.safe_script("assert(getmetatable(my_thing).__type.is(obj))");
	lua.safe_script("assert(not getmetatable(my_thing).__type.is(1))");
	lua.safe_script("assert(not getmetatable(my_thing).__type.is(\"not a thing\"))");
	lua.safe_script("print(getmetatable(my_thing).__type.name)");

	lua.safe_script("assert(getmetatable(obj).__type.is(obj))");
	lua.safe_script("assert(not getmetatable(obj).__type.is(1))");
	lua.safe_script("assert(not getmetatable(obj).__type.is(\"not a thing\"))");
	lua.safe_script("print(getmetatable(obj).__type.name)");
}

#if !defined(_MSC_VER) || !(defined(_WIN32) && !defined(_WIN64))

TEST_CASE("usertype/noexcept-methods", "make sure noexcept functions and methods can be bound to usertypes without issues") {
	sol::state lua;
	sol::stack_guard luasg(lua);
	lua.new_usertype<T>("T", "nf", &T::noexcept_function, "nm", &T::noexcept_method);

	lua.safe_script("t = T.new()");
	lua.safe_script("v1 = t.nf()");
	lua.safe_script("v2 = t:nm()");
	int v1 = lua["v1"];
	int v2 = lua["v2"];
	REQUIRE(v1 == 0x61);
	REQUIRE(v2 == 0x62);
}

#endif // VC++ or my path/compiler settings doing strange bullshit (but it happens on Appveyor too???)

TEST_CASE("usertype/set_function and set parity", "Make sure set and set_function on metatable / usertype<T> have feature parity and do the right thing") {
	SECTION("usertype<T>") {
		sol::state L;
		L.open_libraries(sol::lib::base);

		// the index only works when set in the constructor?
		auto foo = L.new_usertype<Foobar>("foo", sol::meta_function::index, &Foobar::GetValue);
		foo.set_function("f", &Foobar::GetValue).set_function("g", &Foobar::GetValue);
		sol::usertype<Doodah> bar = L.new_usertype<Doodah>("bar");
		bar.set_function(sol::meta_function::index, &Doodah::GetValue).set_function("f", &Doodah::GetValue).set_function("g", &Doodah::GetValue);

		sol::optional<sol::error> maybe_error = L.safe_script(
		     "f = foo.new()\n"
		     "b = bar.new()\n"
		     "assert(f[3] == (f:f(3)))\n"
		     "assert(f[3] == (f:g(3)))\n"
		     "assert(b[6] == (b:f(6)))\n"
		     "assert(b[6] == (b:g(6)))\n",
		     &sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error.has_value());
	}
	SECTION("metatable") {
		sol::state L;
		L.open_libraries(sol::lib::base);

		// the index only works when set in the constructor?
		L.new_usertype<Foobar>("foo");
		L.new_usertype<Doodah>("bar");
		sol::metatable foo = L["foo"];
		sol::metatable bar = L["bar"];
		foo.set_function(sol::meta_function::index, &Foobar::GetValue).set_function("f", &Foobar::GetValue).set_function("g", &Foobar::GetValue);
		bar.set_function(sol::meta_function::index, &Doodah::GetValue).set_function("f", &Doodah::GetValue).set_function("g", &Doodah::GetValue);

		sol::optional<sol::error> maybe_error = L.safe_script(
		     "f = foo.new()\n"
		     "b = bar.new()\n"
		     "assert(f[3] == (f:f(3)))\n"
		     "assert(f[3] == (f:g(3)))\n"
		     "assert(b[6] == (b:f(6)))\n"
		     "assert(b[6] == (b:g(6)))\n",
		     &sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error.has_value());
	}
}
