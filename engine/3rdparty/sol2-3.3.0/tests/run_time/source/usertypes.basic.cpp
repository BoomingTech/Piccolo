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

#include <memory>


TEST_CASE("usertype/usertype", "Show that we can create classes from usertype and use them") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	sol::usertype<fuser> lc = lua.new_usertype<fuser>("fuser", "add", &fuser::add, "add2", &fuser::add2);

	lua.safe_script(
	     "a = fuser:new()\n"
	     "b = a:add(1)\n"
	     "c = a:add2(1)\n");

	sol::object a = lua.get<sol::object>("a");
	sol::object b = lua.get<sol::object>("b");
	sol::object c = lua.get<sol::object>("c");
	REQUIRE((a.is<sol::userdata_value>()));
	auto atype = a.get_type();
	auto btype = b.get_type();
	auto ctype = c.get_type();
	REQUIRE((atype == sol::type::userdata));
	REQUIRE((btype == sol::type::number));
	REQUIRE((ctype == sol::type::number));
	int bresult = b.as<int>();
	int cresult = c.as<int>();
	REQUIRE(bresult == 1);
	REQUIRE(cresult == 3);
}

TEST_CASE("usertype/usertype fundamentals", "Verify new_usertype registers basic member functions and a constructor") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.new_usertype<fuser>("fuser", "add", &fuser::add, "add2", &fuser::add2);

	lua.safe_script(
	     "a = fuser.new()\n"
	     "b = a:add(1)\n"
	     "c = a:add2(1)\n");

	sol::object a = lua.get<sol::object>("a");
	sol::object b = lua.get<sol::object>("b");
	sol::object c = lua.get<sol::object>("c");
	REQUIRE((a.is<sol::userdata_value>()));
	auto atype = a.get_type();
	auto btype = b.get_type();
	auto ctype = c.get_type();
	REQUIRE((atype == sol::type::userdata));
	REQUIRE((btype == sol::type::number));
	REQUIRE((ctype == sol::type::number));
	int bresult = b.as<int>();
	int cresult = c.as<int>();
	REQUIRE(bresult == 1);
	REQUIRE(cresult == 3);
}

TEST_CASE("usertype/safety", "crash with an exception -- not a segfault -- on bad userdata calls") {
	class Test {
	public:
		void sayHello() {
			std::cout << "Hey\n";
		}
	};

	sol::state lua;
	lua.new_usertype<Test>("Test", "sayHello", &Test::sayHello);
	static const std::string code = R"(
        local t = Test.new()
        t:sayHello() --Works fine
        t.sayHello() --Uh oh.
    )";
	auto result = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE_FALSE(result.valid());
}

TEST_CASE("regressions/one", "issue number 48") {
	sol::state lua;
	lua.new_usertype<vars>("vars", "boop", &vars::boop);
	auto code
	     = "beep = vars.new()\n"
	       "beep.boop = 1";
	auto result1 = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(result1.valid());
	// test for segfault
	auto my_var = lua.get<vars>("beep");
	auto& my_var_ref = lua.get<vars>("beep");
	auto* my_var_ptr = lua.get<vars*>("beep");
	REQUIRE(my_var.boop == 1);
	REQUIRE(my_var_ref.boop == 1);
	REQUIRE(my_var_ptr->boop == 1);
	REQUIRE(std::addressof(my_var_ref) == my_var_ptr);
	std::cout << "----- end of 3" << std::endl;
}

TEST_CASE("usertype/issue-number-twenty-five", "Using pointers and references from C++ classes in Lua") {
	struct test {
		int x = 0;
		test& set() {
			x = 10;
			return *this;
		}

		int get() {
			return x;
		}

		test* pget() {
			return this;
		}

		test create_get() {
			return *this;
		}

		int fun(int xa) {
			return xa * 10;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<test>("test", "set", &test::set, "get", &test::get, "pointer_get", &test::pget, "fun", &test::fun, "create_get", &test::create_get);
	{
		auto result = lua.safe_script("x = test.new()", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("assert(x:set():get() == 10)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("y = x:pointer_get()", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("y:set():get()", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("y:fun(10)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("x:fun(10)", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("assert(y:fun(10) == x:fun(10), '...')", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("assert(y:fun(10) == 100, '...')", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("assert(y:set():get() == y:set():get(), '...')", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script("assert(y:set():get() == 10, '...')", sol::script_pass_on_error);
		REQUIRE(result.valid());
	}
}

TEST_CASE("usertype/issue-number-thirty-five", "using value types created from lua-called C++ code, fixing user-defined types with constructors") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	sol::constructors<sol::types<float, float, float>> ctor;
	lua.new_usertype<Vec>("Vec", ctor, "normalized", &Vec::normalized, "length", &Vec::length);

	{
		auto result = lua.safe_script(
		     "v = Vec.new(1, 2, 3)\n"
		     "print(v:length())");
		REQUIRE(result.valid());
	}
	{
		auto result = lua.safe_script(
		     "v = Vec.new(1, 2, 3)\n"
		     "print(v:normalized():length())");
		REQUIRE(result.valid());
	}
}

TEST_CASE("usertype/lua-stored-usertype", "ensure usertype values can be stored without keeping usertype object alive") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	{
		sol::constructors<sol::types<float, float, float>> ctor;
		sol::usertype<Vec> udata = lua.new_usertype<Vec>("Vec", ctor, "normalized", &Vec::normalized, "length", &Vec::length);
		// usertype dies, but still usable in lua!
	}

	{
		auto result = lua.safe_script(
		     "collectgarbage()\n"
		     "v = Vec.new(1, 2, 3)\n"
		     "print(v:length())");
		REQUIRE(result.valid());
	}

	{
		auto result = lua.safe_script(
		     "v = Vec.new(1, 2, 3)\n"
		     "print(v:normalized():length())");
		REQUIRE(result.valid());
	}
}

TEST_CASE("usertype/get-set-references", "properly get and set with std::ref semantics. Note that to get, we must not use Unqualified<T> on the type...") {
	std::cout << "----- in 4" << std::endl;
	sol::state lua;

	lua.new_usertype<vars>("vars", "boop", &vars::boop);
	vars var {};
	vars rvar {};
	std::cout << "setting beep" << std::endl;
	lua.set("beep", var);
	std::cout << "setting rbeep" << std::endl;
	lua.set("rbeep", std::ref(rvar));
	std::cout << "getting my_var" << std::endl;
	auto& my_var = lua.get<vars>("beep");
	std::cout << "setting rbeep" << std::endl;
	auto& ref_var = lua.get<std::reference_wrapper<vars>>("rbeep");
	vars& proxy_my_var = lua["beep"];
	std::reference_wrapper<vars> proxy_ref_var = lua["rbeep"];
	var.boop = 2;
	rvar.boop = 5;

	// Was return as a value: var must be diferent from "beep"
	REQUIRE_FALSE(std::addressof(var) == std::addressof(my_var));
	REQUIRE_FALSE(std::addressof(proxy_my_var) == std::addressof(var));
	REQUIRE((my_var.boop == 0));
	REQUIRE(var.boop != my_var.boop);

	REQUIRE(std::addressof(ref_var) == std::addressof(rvar));
	REQUIRE(std::addressof(proxy_ref_var.get()) == std::addressof(rvar));
	REQUIRE(rvar.boop == 5);
	REQUIRE(rvar.boop == ref_var.boop);
	std::cout << "----- end of 4" << std::endl;
}

TEST_CASE("usertype/const-pointer", "Make sure const pointers can be taken") {
	struct A_x {
		int x = 201;
	};
	struct B_foo {
		int foo(const A_x* a) {
			return a->x;
		};
	};

	sol::state lua;
	lua.new_usertype<B_foo>("B", "foo", &B_foo::foo);
	lua.set("a", A_x());
	lua.set("b", B_foo());
	lua.safe_script("x = b:foo(a)");
	int x = lua["x"];
	REQUIRE(x == 201);
	std::cout << "----- end of 6" << std::endl;
}
