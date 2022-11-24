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


struct user_container_type : public std::vector<int> {
	typedef std::vector<int> base_t;
	using base_t::base_t;
};

TEST_CASE("usertype/container checking fake container", "A container should not respond yes in .is<> to every table in Lua") {
	sol::state lua;
	sol::stack_guard luasg(lua);

	sol::optional<sol::error> err0 = lua.safe_script("a = {}");
	REQUIRE_FALSE(err0.has_value());

	sol::object a = lua["a"];
	REQUIRE(a.is<user_container_type>());
	REQUIRE_FALSE(a.is<user_container_type&>());
	REQUIRE_FALSE(a.is<user_container_type&&>());
	REQUIRE_FALSE(a.is<const user_container_type&>());
	REQUIRE_FALSE(a.is<const user_container_type&&>());
}
