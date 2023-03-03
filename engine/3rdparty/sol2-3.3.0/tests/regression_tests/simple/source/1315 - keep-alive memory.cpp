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

#include <catch2/catch_all.hpp>

#include <sol/sol.hpp>

inline namespace sol2_regression_test_1315 {
	struct A {
	public:
		A() {
		}

		std::vector<int> children;
	};

	static std::vector<int>* getChildrenPtr(A& a) {
		return &a.children;
	}

	static std::vector<int>& getChildrenRef(A& a) {
		return a.children;
	}

	static std::vector<int> getChildrenValue(A& a) {
		return a.children;
	}

} // namespace sol2_regression_test_1315


TEST_CASE("Test for Issue #1315 - memory keep-alive with iteration functions, using a pointer", "[sol2][regression][Issue-1315][pointer]") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine);

	constexpr const auto& coroutine_iteration_code = R"(
for i=1, 100 do
co = coroutine.create( function()
	for child_index, child in ipairs(A.children) do
		collectgarbage()
		assert(child == child_index)
	end
	collectgarbage()
end)
coroutine.resume(co)
end)";

	A a {};
	for (int i = 0; i < 100; i++) {
		a.children.push_back(i);
	}
	
	auto perform_action = [&lua]() {
		// call lua code directly
		auto result = lua.safe_script(coroutine_iteration_code, sol::script_pass_on_error);
		sol::optional<sol::error> maybe_err = result.get<sol::optional<sol::error>>();
		REQUIRE(result.status() == sol::call_status::ok);
		REQUIRE_FALSE(maybe_err.has_value());
	};

	SECTION("A as a pointer") {
		lua["A"] = &a;
		SECTION("using getChildrenPtr as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenPtr));
			perform_action();
		}
		SECTION("using getChildrenRef as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenRef));
			perform_action();
		}
		SECTION("using getChildrenValue as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenValue));
			perform_action();
		}
	}
	SECTION("A as a std::ref") {
		lua["A"] = std::ref(a);
		SECTION("using getChildrenPtr as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenPtr));
			perform_action();
		}
		SECTION("using getChildrenRef as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenRef));
			perform_action();
		}
		SECTION("using getChildrenValue as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenValue));
			perform_action();
		}
	}
	SECTION("A usertype object as a value (copied)") {
		lua["A"] = a;
		SECTION("using getChildrenPtr as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenPtr));
			perform_action();
		}
		SECTION("using getChildrenRef as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenRef));
			perform_action();
		}
		SECTION("using getChildrenValue as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenValue));
			perform_action();
		}
	}
	SECTION("A usertype object as a value (moved)") {
		lua["A"] = std::move(a);
		SECTION("using getChildrenPtr as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenPtr));
			perform_action();
		}
		SECTION("using getChildrenRef as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenRef));
			perform_action();
		}
		SECTION("using getChildrenValue as the sol::property function") {
			lua.new_usertype<A>("A", "children", sol::property(getChildrenValue));
			perform_action();
		}
	}
}
