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

inline namespace sol2_tests_usertypes_unregister {
	struct unregister_me {
		double b = 5.5;
		std::string f_val = "registered";

		unregister_me() {
		}

		std::string f() {
			return f_val;
		}
	};

	struct goddamnit { };

	struct state_pointer_and_registration {
		sol::state* lua_ptr;
		bool* registered_ptr;
	};

} // namespace sol2_tests_usertypes_unregister

TEST_CASE("usertypes/unregister", "make sure that a class can still be bound but that it becomes completely unregistered") {
	const sol::string_view line1 = "assert(u:f() == 'registered')";
	const sol::string_view line2 = "assert(urm.a() == 20)";
	const sol::string_view line3 = "assert(u.a() == 20) assert(u:a() == 20)";
	const sol::string_view line4 = "assert(u.b == 5.5)";

	sol::state lua;
	lua.open_libraries();
	auto register_urm = [&lua, &line1, &line2, &line3, &line4]() {
		lua.new_usertype<unregister_me>(
		     "urm", "f", &unregister_me::f, "a", []() { return 20; }, "b", &unregister_me::b);
		{
			sol::object urm_obj = lua["urm"];
			REQUIRE(urm_obj.get_type() == sol::type::table);
			REQUIRE(urm_obj.is<sol::table>());
			REQUIRE(urm_obj.is<sol::metatable>());
			REQUIRE(urm_obj.is<sol::usertype<unregister_me>>());
		}

		lua["urm_unregister"] = [](sol::this_state ts) {
			sol::state_view current_lua = ts;
			sol::usertype<unregister_me> urm = current_lua["urm"];
			urm.unregister();
		};

		auto sresult0 = lua.safe_script("u = urm.new()", sol::script_pass_on_error);
		REQUIRE(sresult0.valid());
		auto sresult1 = lua.safe_script(line1, sol::script_pass_on_error);
		REQUIRE(sresult1.valid());
		auto sresult2 = lua.safe_script(line2, sol::script_pass_on_error);
		REQUIRE(sresult2.valid());
		auto sresult3 = lua.safe_script(line3, sol::script_pass_on_error);
		REQUIRE(sresult3.valid());
		auto sresult4 = lua.safe_script(line4, sol::script_pass_on_error);
		REQUIRE(sresult4.valid());

		unregister_me& u_orig = lua["u"];
		REQUIRE(u_orig.b == 5.5);
		REQUIRE(u_orig.f() == "registered");
	};
	SECTION("unregister C++") {
		register_urm();

		{
			sol::usertype<unregister_me> urm = lua["urm"];
			urm.unregister();
		}

		auto result0 = lua.safe_script("u_fail = urm.new()", sol::script_pass_on_error);
		REQUIRE_FALSE(result0.valid());
		auto result1 = lua.safe_script(line1, sol::script_pass_on_error);
		REQUIRE_FALSE(result1.valid());
		auto result2 = lua.safe_script(line2, sol::script_pass_on_error);
		REQUIRE_FALSE(result2.valid());
		auto result3 = lua.safe_script(line3, sol::script_pass_on_error);
		REQUIRE_FALSE(result3.valid());
		auto result4 = lua.safe_script(line4, sol::script_pass_on_error);
		REQUIRE_FALSE(result4.valid());

		unregister_me& u = lua["u"];
		REQUIRE(u.b == 5.5);
		REQUIRE(u.f() == "registered");
	}
	SECTION("re-register") {
		register_urm();

		sol::protected_function urm_unregister_func = lua["urm_unregister"];
		auto unregister_result = urm_unregister_func();
		REQUIRE(unregister_result.valid());

		auto result0 = lua.safe_script("u_fail2 = urm.new()", sol::script_pass_on_error);
		REQUIRE_FALSE(result0.valid());
		auto result1 = lua.safe_script(line1, sol::script_pass_on_error);
		REQUIRE_FALSE(result1.valid());
		auto result2 = lua.safe_script(line2, sol::script_pass_on_error);
		REQUIRE_FALSE(result2.valid());
		auto result3 = lua.safe_script(line3, sol::script_pass_on_error);
		REQUIRE_FALSE(result3.valid());
		auto result4 = lua.safe_script(line4, sol::script_pass_on_error);
		REQUIRE_FALSE(result4.valid());

		unregister_me& u = lua["u"];
		REQUIRE(u.b == 5.5);
		REQUIRE(u.f() == "registered");

		register_urm();
	}
	SECTION("unregister lua") {
		register_urm();

		auto unregister_result = lua.safe_script("urm_unregister()", sol::script_pass_on_error);
		REQUIRE(unregister_result.valid());

		auto result0 = lua.safe_script("u_fail2 = urm.new()", sol::script_pass_on_error);
		REQUIRE_FALSE(result0.valid());
		auto result1 = lua.safe_script(line1, sol::script_pass_on_error);
		REQUIRE_FALSE(result1.valid());
		auto result2 = lua.safe_script(line2, sol::script_pass_on_error);
		REQUIRE_FALSE(result2.valid());
		auto result3 = lua.safe_script(line3, sol::script_pass_on_error);
		REQUIRE_FALSE(result3.valid());
		auto result4 = lua.safe_script(line4, sol::script_pass_on_error);
		REQUIRE_FALSE(result4.valid());

		unregister_me& u = lua["u"];
		REQUIRE(u.b == 5.5);
		REQUIRE(u.f() == "registered");
	}
}

TEST_CASE("usertypes/unregister multiple states", "guarantee unregistration can happen from multiple states without interfering with the originals") {
	sol::state lua0;
	bool lua0_registered = false;
	lua0.open_libraries(sol::lib::base);
	lua0.new_usertype<goddamnit>("goddamnit", "a", sol::property([i = 0]() { return i; }));

	sol::state lua1;
	bool lua1_registered = false;
	lua1.open_libraries(sol::lib::base);
	lua1.new_usertype<goddamnit>("goddamnit", "a", sol::property([i = 1]() { return i; }));

	sol::state lua2;
	bool lua2_registered = false;
	lua2.open_libraries(sol::lib::base);
	lua2.new_usertype<goddamnit>("goddamnit", "a", sol::property([i = 2]() { return i; }));

	state_pointer_and_registration states[] = { { &lua0, &lua0_registered }, { &lua1, &lua1_registered }, { &lua2, &lua2_registered } };
	const std::size_t number_of_states = std::size(states);

	auto prove = [](sol::state& lua, const bool& is_registered, std::size_t state_index) {
		std::string code = "val = gdi.a\nprint(val)";
		if (is_registered) {
			sol::optional<sol::error> maybe_error = lua.safe_script(code, sol::script_pass_on_error);
			REQUIRE_FALSE(maybe_error.has_value());
			sol::optional<std::size_t> maybe_val = lua["val"];
			REQUIRE(maybe_val.has_value());
			const std::size_t& val = *maybe_val;
			REQUIRE(val == state_index);
		}
		else {
			sol::optional<sol::error> maybe_error = lua.safe_script(code, sol::script_pass_on_error);
			REQUIRE(maybe_error.has_value());
			const sol::error& error = *maybe_error;
			std::string err = error.what();
			bool has_a_vague_error_that_makes_some_amount_of_sense
			     = (err.find("attempt to index") != std::string::npos) || (err.find("gdi") != std::string::npos);
			REQUIRE(has_a_vague_error_that_makes_some_amount_of_sense);
		}
	};

	for (std::size_t i = 0; i < number_of_states; ++i) {
		state_pointer_and_registration& repr = states[i];
		sol::state& lua = *repr.lua_ptr;
		bool& is_registered = *repr.registered_ptr;

		lua["gdi"] = goddamnit {};
		is_registered = true;

		prove(lua, is_registered, i);
	}
	{
		sol::metatable goddamnit_usertype0 = lua0["goddamnit"];
		goddamnit_usertype0.unregister();
		lua0_registered = false;
	}
	for (std::size_t i = 0; i < number_of_states; ++i) {
		state_pointer_and_registration& repr = states[i];
		sol::state& lua = *repr.lua_ptr;
		bool& is_registered = *repr.registered_ptr;

		prove(lua, is_registered, i);
	}
	{
		sol::usertype<goddamnit> goddamnit_usertype1 = lua1["goddamnit"];
		goddamnit_usertype1.unregister();
		lua1_registered = false;
	}
	for (std::size_t i = 0; i < number_of_states; ++i) {
		state_pointer_and_registration& repr = states[i];
		sol::state& lua = *repr.lua_ptr;
		bool& is_registered = *repr.registered_ptr;

		prove(lua, is_registered, i);
	}
	{
		lua0.new_usertype<goddamnit>("goddamnit", "a", sol::property([i = 0]() { return i; }));
		lua0_registered = true;
	}
	for (std::size_t i = 0; i < number_of_states; ++i) {
		state_pointer_and_registration& repr = states[i];
		sol::state& lua = *repr.lua_ptr;
		bool& is_registered = *repr.registered_ptr;

		prove(lua, is_registered, i);
	}
}
