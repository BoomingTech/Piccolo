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

inline namespace sol2_test_usertypes_inheritance {
	struct inh_test_A {
		int a = 5;
	};

	struct inh_test_B {
		int b() {
			return 10;
		}
	};

	struct inh_test_C : inh_test_B, inh_test_A {
		double c = 2.4;
	};

	struct inh_test_D : inh_test_C {
		bool d() const {
			return true;
		}
	};

	class A {
	public:
		void hello() {
			std::cout << "This is class A" << std::endl;
		}
		virtual void vhello() {
			std::cout << "A::vhello" << std::endl;
		}
		virtual ~A() {
		}

	public:
		int a = 1;
	};

	class B : public A {
	public:
		virtual void vhello() override {
			std::cout << "B::vhello" << std::endl;
		}
		virtual ~B() override {
		}

	public:
		int b = 2;
	};
} // namespace sol2_test_usertypes_inheritance

SOL_BASE_CLASSES(inh_test_D, inh_test_C);
SOL_BASE_CLASSES(inh_test_C, inh_test_B, inh_test_A);
SOL_DERIVED_CLASSES(inh_test_C, inh_test_D);
SOL_DERIVED_CLASSES(inh_test_B, inh_test_C);
SOL_DERIVED_CLASSES(inh_test_A, inh_test_B);

TEST_CASE("inheritance/basic", "test that metatables are properly inherited") {
	sol::state lua;
	int begintop = 0, endtop = 0;
	lua.new_usertype<inh_test_A>("A", "a", &inh_test_A::a);
	lua.new_usertype<inh_test_B>("B", "b", &inh_test_B::b);
	lua.new_usertype<inh_test_C>("C", "c", &inh_test_C::c, sol::base_classes, sol::bases<inh_test_B, inh_test_A>());
	lua.new_usertype<inh_test_D>("D", "d", &inh_test_D::d, sol::base_classes, sol::bases<inh_test_C, inh_test_B, inh_test_A>());

	test_stack_guard tsg(lua, begintop, endtop);

	auto result1 = lua.safe_script("obj = D.new()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("d = obj:d()", sol::script_pass_on_error);
	REQUIRE(result2.valid());
	bool d = lua["d"];
	auto result3 = lua.safe_script("c = obj.c", sol::script_pass_on_error);
	REQUIRE(result3.valid());
	double c = lua["c"];
	auto result4 = lua.safe_script("b = obj:b()", sol::script_pass_on_error);
	REQUIRE(result4.valid());
	int b = lua["b"];
	auto result5 = lua.safe_script("a = obj.a", sol::script_pass_on_error);
	REQUIRE(result5.valid());
	int a = lua["a"];

	REQUIRE(d);
	REQUIRE(c == 2.4);
	REQUIRE(b == 10);
	REQUIRE(a == 5);
}

TEST_CASE(
     "inheritance/usertype derived non-hiding", "usertype classes must play nice when a derived class does not overload a publically visible base function") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	sol::constructors<sol::types<int>> basector;
	sol::usertype<Base> baseusertype = lua.new_usertype<Base>("Base", basector, "get_num", &Base::get_num);

	lua.safe_script("base = Base.new(5)");
	{
		auto result = lua.safe_script("print(base:get_num())", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}

	sol::constructors<sol::types<int>> derivedctor;
	sol::usertype<Derived> derivedusertype
	     = lua.new_usertype<Derived>("Derived", derivedctor, "get_num_10", &Derived::get_num_10, "get_num", &Derived::get_num);

	lua.safe_script("derived = Derived.new(7)");
	lua.safe_script(
	     "dgn = derived:get_num()\n"
	     "print(dgn)");
	lua.safe_script(
	     "dgn10 = derived:get_num_10()\n"
	     "print(dgn10)");

	REQUIRE((lua.get<int>("dgn10") == 70));
	REQUIRE((lua.get<int>("dgn") == 7));
}

TEST_CASE("inheritance/usertype metatable interference",
     "usertypes with overriden index/new_index methods (from e.g. base classes) were intercepting calls to the named metatable") {
	sol::state lua;
	lua.open_libraries(sol::lib::base,
	     sol::lib::package,
	     sol::lib::coroutine,
	     sol::lib::string,
	     sol::lib::os,
	     sol::lib::math,
	     sol::lib::table,
	     sol::lib::io,
	     sol::lib::debug);

	sol::usertype<A> uta = lua.new_usertype<A>("A", sol::base_classes, sol::base_list<>());
	uta.set(sol::call_constructor, sol::constructors<A()>());
	uta.set("a", &A::a);
	uta.set("hello", &A::hello);
	uta.set("vhello", &A::vhello);

	sol::usertype<B> utb = lua.new_usertype<B>("B", sol::base_classes, sol::base_list<A>());
	utb.set(sol::call_constructor, sol::constructors<B()>());
	utb.set("b", &B::b);
	utb.set("vhello", &B::vhello);

	sol::optional<sol::error> maybe_error0 = lua.safe_script(R"(
	local aa = A()
	aa:hello()
	)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error0.has_value());

	sol::optional<sol::error> maybe_error1 = lua.safe_script(R"(
	local bb0 = B.new()
	bb0:hello()
	)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error1.has_value());

	sol::optional<sol::error> maybe_error2 = lua.safe_script(R"(
	local bb1 = B()
	bb1:hello()
	)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error2.has_value());
}
