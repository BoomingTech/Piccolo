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
#include <unordered_map>
#include <string_view>

struct static_special_property_object {
	static int named_set_calls;
	static int named_get_calls;

	struct obj_hash {
		std::size_t operator()(const sol::object& obj) const noexcept {
			return std::hash<const void*>()(obj.pointer());
		}
	};

	std::unordered_map<sol::object, sol::object, obj_hash> props;

	sol::object get_property_lua(sol::stack_object key) const {
		if (auto it = props.find(key); it != props.cend()) {
			return it->second;
		}
		return sol::lua_nil;
	}

	void set_property_lua(sol::stack_object key, sol::stack_object value) {
		props.insert_or_assign(key, sol::object(value));
	}

	static void named_get_property_lua(sol::this_state L) {
		++named_get_calls;
		luaL_error(L, "absolutely not");
	}

	static void named_set_property_lua(sol::this_state L) {
		++named_set_calls;
		luaL_error(L, "absolutely not");
	}
};

int static_special_property_object::named_get_calls = 0;
int static_special_property_object::named_set_calls = 0;

struct special_property_object {
	struct obj_hash {
		std::size_t operator()(const sol::object& obj) const noexcept {
			return std::hash<const void*>()(obj.pointer());
		}
	};

	std::unordered_map<sol::object, sol::object, obj_hash> props;

	sol::object get_property_lua(sol::stack_object key) const {
		if (auto it = props.find(key); it != props.cend()) {
			return it->second;
		}
		return sol::lua_nil;
	}

	void set_property_lua(sol::stack_object key, sol::stack_object value) {
		props.insert_or_assign(key, sol::object(value));
	}
};

struct new_index_test_object {
	bool new_indexed = false;
	bool borf_new_indexed = false;

	float GetLevel(int x) const {
		return static_cast<float>(x);
	}

	static void nidx(new_index_test_object& self, std::string_view key, sol::stack_object value) {
		if (key == "borf" && value.is<double>() && value.as<double>() == 2) {
			self.borf_new_indexed = true;
			return;
		}
		self.new_indexed = true;
	}

	static sol::object idx(sol::stack_object self, std::string_view key) {
		if (key == "bark") {
			return sol::object(self.lua_state(), sol::in_place, &new_index_test_object::GetLevel);
		}
		return sol::lua_nil;
	}
};

TEST_CASE("usertype/runtime-extensibility", "Check if usertypes are runtime extensible") {
	struct thing {
		int v = 20;
		int func(int a) {
			return a;
		}
	};
	int val = 0;

	class base_a {
	public:
		int x;
	};

	class derived_b : public base_a { };

	SECTION("just functions") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<thing>("thing", "func", &thing::func);

		lua.safe_script(R"(
t = thing.new()
		)");

		{
			auto result = lua.safe_script(R"(
t.runtime_func = function (a)
	return a + 50
end
		)",
			     sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};

		{
			auto result = lua.safe_script(R"(
function t:runtime_func(a)
	return a + 52
end
		)",
			     sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};

		lua.safe_script("val = t:func(2)");
		val = lua["val"];
		REQUIRE(val == 2);

		REQUIRE_NOTHROW([&lua]() {
			lua.safe_script(R"(
function thing:runtime_func(a)
	return a + 1
end
		)");
		}());

		lua.safe_script("val = t:runtime_func(2)");
		val = lua["val"];
		REQUIRE(val == 3);
	}
	SECTION("with variable") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<thing>("thing", "func", &thing::func, "v", &thing::v);

		lua.safe_script(R"(
t = thing.new()
		)");

		{
			auto result = lua.safe_script(R"(
t.runtime_func = function (a)
	return a + 50
end
		)",
			     sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};

		{
			auto result = lua.safe_script(R"(
function t:runtime_func(a)
	return a + 52
end
		)",
			     sol::script_pass_on_error);
			REQUIRE_FALSE(result.valid());
		};

		lua.safe_script("val = t:func(2)");
		val = lua["val"];
		REQUIRE(val == 2);

		REQUIRE_NOTHROW([&lua]() {
			lua.safe_script(R"(
function thing:runtime_func(a)
	return a + 1
end
		)");
		}());

		lua.safe_script("val = t:runtime_func(2)");
		val = lua["val"];
		REQUIRE(val == 3);
	}
	SECTION("with bases") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<base_a>("A", "x", &base_a::x // no crash without this
		);

		lua.new_usertype<derived_b>("B", sol::base_classes, sol::bases<base_a>());

		auto pfr0 = lua.safe_script("function A:c() print('A') return 1 end", sol::script_pass_on_error);
		REQUIRE(pfr0.valid());
		auto pfr1 = lua.safe_script("function B:c() print('B') return 2 end", sol::script_pass_on_error);
		REQUIRE(pfr1.valid());
		auto pfr2 = lua.safe_script("obja = A.new() objb = B.new()", sol::script_default_on_error);
		REQUIRE(pfr2.valid());
		auto pfr3 = lua.safe_script("assert(obja:c() == 1)", sol::script_default_on_error);
		REQUIRE(pfr3.valid());
		auto pfr4 = lua.safe_script("assert(objb:c() == 2)", sol::script_default_on_error);
		REQUIRE(pfr4.valid());
	}
}

TEST_CASE("usertype/runtime-replacement", "ensure that functions can be properly replaced at runtime for non-indexed things") {
	struct heart_base_t { };
	struct heart_t : heart_base_t {
		int x = 0;
		void func() {
		}
	};

	SECTION("plain") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<heart_t>("a");
		{
			auto result1 = lua.safe_script("obj = a.new()", sol::script_pass_on_error);
			REQUIRE(result1.valid());
			auto result2 = lua.safe_script("function a:heartbeat () print('arf') return 1 end", sol::script_pass_on_error);
			REQUIRE(result2.valid());
			auto result3 = lua.safe_script("v1 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result3.valid());
			auto result4 = lua.safe_script("function a:heartbeat () print('bark') return 2 end", sol::script_pass_on_error);
			REQUIRE(result4.valid());
			auto result5 = lua.safe_script("v2 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result5.valid());
			auto result6 = lua.safe_script("a.heartbeat = function(self) print('woof') return 3 end", sol::script_pass_on_error);
			REQUIRE(result6.valid());
			auto result7 = lua.safe_script("v3 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result7.valid());
		}
		int v1 = lua["v1"];
		int v2 = lua["v2"];
		int v3 = lua["v3"];
		REQUIRE(v1 == 1);
		REQUIRE(v2 == 2);
		REQUIRE(v3 == 3);
	}
	SECTION("variables") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<heart_t>("a", "x", &heart_t::x, sol::base_classes, sol::bases<heart_base_t>());

		{
			auto result1 = lua.safe_script("obj = a.new()", sol::script_pass_on_error);
			REQUIRE(result1.valid());
			auto result2 = lua.safe_script("function a:heartbeat () print('arf') return 1 end", sol::script_pass_on_error);
			REQUIRE(result2.valid());
			auto result3 = lua.safe_script("v1 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result3.valid());
			auto result4 = lua.safe_script("function a:heartbeat () print('bark') return 2 end", sol::script_pass_on_error);
			REQUIRE(result4.valid());
			auto result5 = lua.safe_script("v2 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result5.valid());
			auto result6 = lua.safe_script("a.heartbeat = function(self) print('woof') return 3 end", sol::script_pass_on_error);
			REQUIRE(result6.valid());
			auto result7 = lua.safe_script("v3 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result7.valid());
		}
		int v1 = lua["v1"];
		int v2 = lua["v2"];
		int v3 = lua["v3"];
		REQUIRE(v1 == 1);
		REQUIRE(v2 == 2);
		REQUIRE(v3 == 3);
	}
	SECTION("methods") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<heart_t>("a", "func", &heart_t::func, sol::base_classes, sol::bases<heart_base_t>());

		{
			auto result1 = lua.safe_script("obj = a.new()", sol::script_pass_on_error);
			REQUIRE(result1.valid());
			auto result2 = lua.safe_script("function a:heartbeat () print('arf') return 1 end", sol::script_pass_on_error);
			REQUIRE(result2.valid());
			auto result3 = lua.safe_script("v1 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result3.valid());
			auto result4 = lua.safe_script("function a:heartbeat () print('bark') return 2 end", sol::script_pass_on_error);
			REQUIRE(result4.valid());
			auto result5 = lua.safe_script("v2 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result5.valid());
			auto result6 = lua.safe_script("a.heartbeat = function(self) print('woof') return 3 end", sol::script_pass_on_error);
			REQUIRE(result6.valid());
			auto result7 = lua.safe_script("v3 = obj:heartbeat()", sol::script_pass_on_error);
			REQUIRE(result7.valid());
		}
		int v1 = lua["v1"];
		int v2 = lua["v2"];
		int v3 = lua["v3"];
		REQUIRE(v1 == 1);
		REQUIRE(v2 == 2);
		REQUIRE(v3 == 3);
	}
}

TEST_CASE("usertype/runtime additions with newindex", "ensure that additions when new_index is overriden don't hit the specified new_index function") {
	class newindex_object { };
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<newindex_object>("object", sol::meta_function::new_index, [](newindex_object&, sol::object, sol::object) { return; });

	lua["object"]["test"] = [](newindex_object&) {
		std::cout << "test" << std::endl;
		return 446;
	};

	auto result1 = lua.safe_script("o = object.new()", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("assert(o:test() == 446)", sol::script_pass_on_error);
	REQUIRE(result2.valid());
}

TEST_CASE("usertype/meta-key-retrievals", "allow for special meta keys (__index, __newindex) to trigger methods even if overwritten directly") {
	SECTION("dynamically") {
		static int writes = 0;
		static std::string keys[4] = {};
		static int values[4] = {};
		struct d_sample {
			void foo(std::string k, int v) {
				keys[writes] = k;
				values[writes] = v;
				++writes;
			}
		};

		sol::state lua;
		lua.new_usertype<d_sample>("sample");
		sol::table s = lua["sample"]["new"]();
		s[sol::metatable_key][sol::meta_function::new_index] = &d_sample::foo;
		lua["var"] = s;

		sol::optional<sol::error> maybe_error0 = lua.safe_script("var = sample.new()", sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error0.has_value());
		sol::optional<sol::error> maybe_error1 = lua.safe_script("var.key = 2", sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error1.has_value());
		sol::optional<sol::error> maybe_error2 = lua.safe_script("var.__newindex = 4", sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error2.has_value());
		sol::optional<sol::error> maybe_error3 = lua.safe_script("var.__index = 3", sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error3.has_value());
		sol::optional<sol::error> maybe_error4 = lua.safe_script("var.__call = 1", sol::script_pass_on_error);
		REQUIRE_FALSE(maybe_error4.has_value());
		REQUIRE(values[0] == 2);
		REQUIRE(values[1] == 4);
		REQUIRE(values[2] == 3);
		REQUIRE(values[3] == 1);
		REQUIRE(keys[0] == "key");
		REQUIRE(keys[1] == "__newindex");
		REQUIRE(keys[2] == "__index");
		REQUIRE(keys[3] == "__call");
	}

	SECTION("statically") {
		static int writes = 0;
		static std::string keys[4] = {};
		static int values[4] = {};
		struct sample {
			void foo(std::string k, int v) {
				keys[writes] = k;
				values[writes] = v;
				++writes;
			}
		};

		sol::state lua;
		lua.new_usertype<sample>("sample", sol::meta_function::new_index, &sample::foo);

		lua.safe_script("var = sample.new()");
		lua.safe_script("var.key = 2");
		lua.safe_script("var.__newindex = 4");
		lua.safe_script("var.__index = 3");
		lua.safe_script("var.__call = 1");
		REQUIRE(values[0] == 2);
		REQUIRE(values[1] == 4);
		REQUIRE(values[2] == 3);
		REQUIRE(values[3] == 1);
		REQUIRE(keys[0] == "key");
		REQUIRE(keys[1] == "__newindex");
		REQUIRE(keys[2] == "__index");
		REQUIRE(keys[3] == "__call");
	}
}

TEST_CASE("usertype/new_index and index",
     "a custom new_index and index only kicks in after the values pre-ordained on the index and new_index tables are assigned") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<new_index_test_object>("new_index_test_object",
	     sol::meta_function::index,
	     &new_index_test_object::idx,
	     sol::meta_function::new_index,
	     &new_index_test_object::nidx,
	     "Level",
	     &new_index_test_object::GetLevel);

	const auto& code = R"(a = new_index_test_object.new()
	                   print(a:Level(1))
				    print(a:Level(2))
				    print(a:Level(3)))";

	auto result0 = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(result0.valid());
	auto result1 = lua.safe_script("print(a:bark(1))", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("assert(a.Level2 == nil)", sol::script_pass_on_error);
	REQUIRE(result2.valid());
	auto result3 = lua.safe_script("a:Level3()", sol::script_pass_on_error);
	REQUIRE_FALSE(result3.valid());

	new_index_test_object& a = lua["a"];
	REQUIRE_FALSE(a.borf_new_indexed);
	REQUIRE_FALSE(a.new_indexed);

	auto resultnormal = lua.safe_script("a.normal = 'foo'", sol::script_pass_on_error);
	REQUIRE(resultnormal.valid());
	REQUIRE_FALSE(a.borf_new_indexed);
	REQUIRE(a.new_indexed);

	auto resultborf = lua.safe_script("a.borf = 2", sol::script_pass_on_error);
	REQUIRE(resultborf.valid());
	REQUIRE(a.borf_new_indexed);
	REQUIRE(a.new_indexed);
}

TEST_CASE("usertype/object and class extensible", "make sure that a class which override new_index and friends can still work properly") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<special_property_object>("special_property_object",
	     sol::meta_function::new_index,
	     &special_property_object::set_property_lua,
	     sol::meta_function::index,
	     &special_property_object::get_property_lua);

	lua["add_class_func"] = [](sol::this_state L, special_property_object&) {
		sol::stack_userdata self = sol::stack::get<sol::stack_userdata>(L, 1);
		sol::usertype<special_property_object> mt = self[sol::metatable_key];
		mt["additional_function"] = []() { return 24; };
	};

	lua["add_object_func"] = [](sol::this_state L, special_property_object&) {
		sol::stack_userdata self = sol::stack::get<sol::stack_userdata>(L, 1);
		self["specific_function"] = []() { return 23; };
	};

	sol::optional<sol::error> result0 = lua.safe_script(R"(
		s = special_property_object.new()
		s2 = special_property_object.new()
		add_class_func(s)
		add_object_func(s)
		value = s:additional_function()
		assert(value == 24)
		value2 = s:specific_function()
		assert(value2 == 23) 
	)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(result0.has_value());
	int value = lua["value"];
	REQUIRE(value == 24);
	int value2 = lua["value2"];
	REQUIRE(value2 == 23);

	sol::optional<sol::error> result1 = lua.safe_script(R"(
		value3 = s2:specific_function()
		assert(value3 == 23)
	)",
	     sol::script_pass_on_error);
	REQUIRE(result1.has_value());
}

TEST_CASE("usertypes/static new index and static index", "ensure static index and static new index provide the proper interface") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<static_special_property_object>("static_special_property_object",
	     sol::meta_function::index,
	     &static_special_property_object::get_property_lua,
	     sol::meta_function::new_index,
	     &static_special_property_object::set_property_lua,
	     sol::meta_function::static_index,
	     &static_special_property_object::named_get_property_lua,
	     sol::meta_function::static_new_index,
	     &static_special_property_object::named_set_property_lua);

	lua["add_object_func"] = [](sol::this_state L, static_special_property_object&) {
		sol::stack_userdata self = sol::stack::get<sol::stack_userdata>(L, 1);
		self["specific_function"] = []() { return 23; };
	};

	sol::optional<sol::error> result0 = lua.safe_script(R"(
		s = static_special_property_object.new()
		s2 = static_special_property_object.new()		
		add_object_func(s)
		value2 = s:specific_function()
		assert(value2 == 23)
	)",
	     sol::script_pass_on_error);
	REQUIRE_FALSE(result0.has_value());
	int value2 = lua["value2"];
	REQUIRE(value2 == 23);

	sol::optional<sol::error> result1 = lua.safe_script(R"(
		function static_special_property_object:additional_function ()
			return 24
		end
		value = s:additional_function()
		assert(value == 24)
	)",
	     sol::script_pass_on_error);
	REQUIRE(result1.has_value());
	bool is_value_valid = lua["value"].valid();
	REQUIRE_FALSE(is_value_valid);

	sol::optional<sol::error> result2 = lua.safe_script(R"(
		value3 = s2:specific_function()
		assert(value3 == 23)
	)",
	     sol::script_pass_on_error);
	REQUIRE(result2.has_value());

	sol::optional<sol::error> result3 = lua.safe_script(R"(
		assert(static_special_property_object.non_existent == nil)
	)",
	     sol::script_pass_on_error);
	REQUIRE(result3.has_value());

	REQUIRE(static_special_property_object::named_get_calls == 1);
	REQUIRE(static_special_property_object::named_set_calls == 1);
}
