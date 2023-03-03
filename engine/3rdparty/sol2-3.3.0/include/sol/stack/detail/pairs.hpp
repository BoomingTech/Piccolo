// sol2

// The MIT License (MIT)

// Copyright (c) 2013-2022 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this Spermission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_DETAIL_PAIRS_HPP
#define SOL_DETAIL_PAIRS_HPP

#include <sol/version.hpp>

#include <sol/stack.hpp>
#include <sol/stack_reference.hpp>
#include <sol/protected_function.hpp>
#include <sol/assert.hpp>

#include <optional>

namespace sol { namespace stack { namespace stack_detail {

	inline bool maybe_push_lua_next_function(lua_State* L_) {
		stack::get_field<true, false>(L_, "next");
		bool is_next = stack::check<protected_function>(L_);
		if (is_next) {
			return true;
		}
		stack::get_field<true, false>(L_, "table");
		stack::record tracking{};
		if (!stack::loose_table_check(L_, -1, &no_panic, tracking)) {
			return false;
		}
		lua_getfield(L_, -1, "next");
		bool is_table_next_func = stack::check<protected_function>(L_, -1);
		if (is_table_next_func) {
			return true;
		}
		lua_pop(L_, 1);
		return false;
	}

	inline std::optional<protected_function> find_lua_next_function(lua_State* L_) {
		if (maybe_push_lua_next_function(L_)) {
			return stack::pop<protected_function>(L_);
		}
		return std::nullopt;
	}

	inline int c_lua_next(lua_State* L_) noexcept {
		stack_reference table_stack_ref(L_, raw_index(1));
		stateless_stack_reference key_stack_ref(L_, raw_index(2));
		int result = lua_next(table_stack_ref.lua_state(), table_stack_ref.stack_index());
		if (result == 0) {
			stack::push(L_, lua_nil);
			return 1;
		}
		return 2;
	}

	inline int readonly_pairs(lua_State* L_) noexcept {
		int pushed = 0;
		if (!maybe_push_lua_next_function(L_)) {
			// we do not have the "next" function in the global namespace
			// from the "table" global entiry, use our own
			pushed += stack::push(L_, &c_lua_next);
		}
		else {
			pushed += 1;
		}
		int metatable_exists = lua_getmetatable(L_, 1);
		sol_c_assert(metatable_exists == 1);
		const auto& index_key = to_string(sol::meta_function::index);
		lua_getfield(L_, lua_gettop(L_), index_key.c_str());
		lua_remove(L_, -2);
		pushed += 1;
		pushed += stack::push(L_, lua_nil);
		return pushed;
	}

}}} // sol::stack::stack_detail

#endif // SOL_DETAIL_PAIRS_HPP
