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

inline namespace sol2_tests_usertypes_auxiliary_keys {
	struct aux { };

	struct aux_index {
		int index(sol::stack_reference) const noexcept {
			return 500;
		}
	};

} // namespace sol2_tests_usertypes_auxiliary_keys

TEST_CASE("usertype/auxiliary keys", "Make sure set and set_function on metatable / usertype<T> have feature parity and do the right thing") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// the index only works when set in the constructor?
	auto aux_ut = lua.new_usertype<aux>("aux");
	auto aux_index_ut = lua.new_usertype<aux_index>("aux_index");
	sol::metatable aux_mt = lua["aux"];
	sol::metatable aux_index_mt = lua["aux_index"];

	aux_ut[1] = "value-1";
	aux_mt[2] = 2;

	aux_index_ut.set_function(sol::meta_function::index, &aux_index::index);
	aux_index_ut[1] = "value-1";
	aux_index_mt[2] = 2;

	sol::optional<sol::error> maybe_error = lua.safe_script(
	     "a = aux.new()\n"
	     "ai = aux_index.new()\n"
	     "aux[3] = function () return 321 end\n"
	     "aux_index[3] = function () return 123 end\n"
	     "assert(a[1] == \"value-1\")\n"
	     "assert(a[2] == 2)\n"
	     "assert(a[3]() == 321)\n"
	     "assert(a[4] == nil)\n"
	     "assert(ai[1] == \"value-1\")\n"
	     "assert(ai[2] == 2)\n"
	     "assert(ai[3]() == 123)\n"
	     "assert(ai[4] == 500)\n",
	     &sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error.has_value());
}
