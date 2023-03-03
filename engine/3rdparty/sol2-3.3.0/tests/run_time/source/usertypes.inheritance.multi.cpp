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

class TestClass00 {
public:
	TestClass00() {
	}

	int Thing() const {
		return 123;
	}
};

class TestClass01 : public TestClass00 {
public:
	TestClass01() : a(1) {
	}
	TestClass01(const TestClass00& other) : a(other.Thing()) {
	}

	int a;
};

class TestClass02 : public TestClass01 {
public:
	TestClass02() : b(2) {
	}
	TestClass02(const TestClass01& other) : b(other.a) {
	}
	TestClass02(const TestClass00& other) : b(other.Thing()) {
	}

	int b;
};

class TestClass03 : public TestClass02 {
public:
	TestClass03() : c(2) {
	}
	TestClass03(const TestClass02& other) : c(other.b) {
	}
	TestClass03(const TestClass01& other) : c(other.a) {
	}
	TestClass03(const TestClass00& other) : c(other.Thing()) {
	}

	int c;
};

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

SOL_BASE_CLASSES(TestClass03, TestClass02);
SOL_BASE_CLASSES(TestClass02, TestClass01);
SOL_BASE_CLASSES(TestClass01, TestClass00);
SOL_DERIVED_CLASSES(TestClass02, TestClass03);
SOL_DERIVED_CLASSES(TestClass01, TestClass02);
SOL_DERIVED_CLASSES(TestClass00, TestClass01);

TEST_CASE("inheritance/multi base", "test that multiple bases all work and overloading for constructors works with them") {

	sol::state lua;

	sol::usertype<TestClass00> s_TestUsertype00
	     = lua.new_usertype<TestClass00>("TestClass00", sol::call_constructor, sol::constructors<TestClass00()>(), "Thing", &TestClass00::Thing);

	sol::usertype<TestClass01> s_TestUsertype01 = lua.new_usertype<TestClass01>("TestClass01",
	     sol::call_constructor,
	     sol::constructors<sol::types<>, sol::types<const TestClass00&>>(),
	     sol::base_classes,
	     sol::bases<TestClass00>(),
	     "a",
	     &TestClass01::a);

	sol::usertype<TestClass02> s_TestUsertype02 = lua.new_usertype<TestClass02>("TestClass02",
	     sol::call_constructor,
	     sol::constructors<sol::types<>, sol::types<const TestClass01&>, sol::types<const TestClass00&>>(),
	     sol::base_classes,
	     sol::bases<TestClass01, TestClass00>(),
	     "b",
	     &TestClass02::b);

	sol::usertype<TestClass03> s_TestUsertype03 = lua.new_usertype<TestClass03>("TestClass03",
	     sol::call_constructor,
	     sol::constructors<sol::types<>, sol::types<const TestClass02&>, sol::types<const TestClass01&>, sol::types<const TestClass00&>>(),
	     sol::base_classes,
	     sol::bases<TestClass02, TestClass01, TestClass00>(),
	     "c",
	     &TestClass03::c);

	auto result1 = lua.safe_script(R"(
tc0 = TestClass00()
tc2 = TestClass02(tc0)
tc1 = TestClass01()
tc3 = TestClass03(tc1)
)",
	     sol::script_pass_on_error);
	REQUIRE(result1.valid());

	TestClass00& tc0 = lua["tc0"];
	TestClass01& tc1 = lua["tc1"];
	TestClass02& tc2 = lua["tc2"];
	TestClass03& tc3 = lua["tc3"];
	REQUIRE(tc0.Thing() == 123);
	REQUIRE(tc1.a == 1);
	REQUIRE(tc2.a == 1);
	REQUIRE(tc2.b == 123);
	REQUIRE(tc3.a == 1);
	REQUIRE(tc3.b == 2);
	REQUIRE(tc3.c == 1);
}

TEST_CASE("inheritance/runtime multi base", "test that multiple bases all work and overloading for constructors works with them when just using sol::bases") {
	struct runtime_A {
		int a = 5;
	};

	struct runtime_B {
		int b2 = 46;

		int b() {
			return 10;
		}
	};

	struct runtime_C : runtime_B, runtime_A {
		double c = 2.4;
	};

	struct runtime_D : runtime_C {
		bool d() const {
			return true;
		}
	};

	sol::state lua;
	lua.new_usertype<runtime_A>("A", "a", &runtime_A::a);
	lua.new_usertype<runtime_B>("B", "b", &runtime_B::b);
	lua.new_usertype<runtime_C>("C", "c", &runtime_C::c, sol::base_classes, sol::bases<runtime_B, runtime_A>());
	lua.new_usertype<runtime_D>("D", "d", &runtime_D::d, sol::base_classes, sol::bases<runtime_C, runtime_B, runtime_A>());

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

	runtime_D& d_obj = lua["obj"];
	REQUIRE(d_obj.d());
	REQUIRE(d_obj.c == 2.4);
	REQUIRE(d_obj.b() == 10);
	REQUIRE(d_obj.b2 == 46);
	REQUIRE(d_obj.a == 5);
	runtime_C& c_obj = lua["obj"];
	REQUIRE(c_obj.c == 2.4);
	REQUIRE(c_obj.b() == 10);
	REQUIRE(c_obj.b2 == 46);
	REQUIRE(c_obj.a == 5);
	runtime_B& b_obj = lua["obj"];
	REQUIRE(b_obj.b() == 10);
	REQUIRE(b_obj.b2 == 46);
	runtime_A& a_obj = lua["obj"];
	REQUIRE(a_obj.a == 5);
}

TEST_CASE("inheritance/bad_base-class", "check to make sure bad/unregistered base classes do not blow up usertypes") {
	struct a {
		a(sol::this_state ts, sol::this_environment) {
			lua_State* L = ts;
			ud = sol::userdata(L, 1);
		}


		sol::object get_property_lua(const char* name, sol::this_state) {
			return props[name];
		}

		void set_property_lua(const char* name, sol::stack_object object) {
			props[name] = object.as<sol::object>();
		}

		std::unordered_map<std::string, sol::object> props;
		sol::userdata ud;
	};

	struct nofun {
		nofun() {
		}
	};

	struct b : public a, public nofun {
		b(sol::this_state ts, sol::this_environment te, int) : a(ts, te) {
			sol::state_view lua = ts;
			lua.script("function break_crap(b_obj) b_obj.test3 = {} end");

			sol::protected_function pf = lua["break_crap"];
			sol::optional<sol::error> result = pf(this);
			REQUIRE_FALSE(result.has_value());
		}

		b(sol::this_state ts, sol::this_environment te, int, int) : a(ts, te) {
		}
		~b() {
		}
	};

	struct c : public b {
		c(sol::this_state ts, sol::this_environment te, int ab) : b(ts, te, ab) {
		}

		c(sol::this_state ts, sol::this_environment te, int ab, int bc) : b(ts, te, ab, bc) {
		}
		~c() {
		}
	};

	sol::state lua;

	lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::package, sol::lib::debug);

	lua.new_usertype<a>("a", sol::meta_function::new_index, &a::set_property_lua, sol::meta_function::index, &a::get_property_lua);

	lua.new_usertype<b>("b",
	     sol::constructors<b(sol::this_state, sol::this_environment, int), b(sol::this_state, sol::this_environment, int, int)>(),
	     sol::meta_function::new_index,
	     &b::set_property_lua,
	     sol::meta_function::index,
	     &b::get_property_lua,
	     sol::base_classes,
	     sol::bases<a, nofun>());


	lua.new_usertype<c>("c",
	     sol::constructors<c(sol::this_state, sol::this_environment, int), c(sol::this_state, sol::this_environment, int, int)>(),
	     sol::meta_function::new_index,
	     &c::set_property_lua,
	     sol::meta_function::index,
	     &c::get_property_lua,
	     sol::base_classes,
	     sol::bases<b>());


	lua.script(R"(
		function init_entity(e)
	   init_entity_properties(e)
	   return true
	   end

	   function init_entity_properties(e)
	   e._internal_entity_properties_ = {}

	   function e : GetName()
	   return self._internal_entity_properties_['name']
	   end

	   function e : SetName(s)
	   self._internal_entity_properties_['name'] = s
	   end
	   --return e
	   end

		)");

	sol::optional<sol::error> result = lua.safe_script("b_tmp = b.new(1)", sol::script_pass_on_error);
	REQUIRE_FALSE(result.has_value());

	a* b_base = lua["b_tmp"]; // get the base...
	sol::protected_function pf = lua["init_entity"];
	sol::optional<sol::error> result1 = pf(b_base);
	REQUIRE_FALSE(result1.has_value());

	sol::optional<sol::error> result2 = lua.script("c_tmp = c.new(1)", sol::script_pass_on_error);
	REQUIRE_FALSE(result2.has_value());
}
