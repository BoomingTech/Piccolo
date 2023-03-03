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

#ifndef SOL_PROTECTED_HANDLER_HPP
#define SOL_PROTECTED_HANDLER_HPP

#include <sol/reference.hpp>
#include <sol/stack.hpp>
#include <sol/protected_function_result.hpp>
#include <sol/unsafe_function.hpp>
#include <cstdint>

namespace sol { namespace detail {
	inline const char (&default_handler_name())[9] {
		static const char name[9] = "sol.\xF0\x9F\x94\xA9";
		return name;
	}

	template <bool ShouldPush, typename Target = reference>
	struct protected_handler {
		lua_State* m_L;
		const Target& target;
		int stack_index;

		protected_handler(std::false_type, lua_State* L_, const Target& target_) : m_L(L_), target(target_), stack_index(0) {
			if (ShouldPush) {
				stack_index = lua_gettop(L_) + 1;
				target.push(L_);
			}
		}

		protected_handler(std::true_type, lua_State* L_, const Target& target_) : m_L(L_), target(target_), stack_index(0) {
			if (ShouldPush) {
				stack_index = target.stack_index();
			}
		}

		protected_handler(lua_State* L_, const Target& target_) : protected_handler(meta::boolean<is_stack_based_v<Target>>(), L_, target_) {
		}

		bool valid() const noexcept {
			return ShouldPush;
		}

		~protected_handler() {
			if constexpr (!is_stack_based_v<Target>) {
				if (stack_index != 0) {
					lua_remove(m_L, stack_index);
				}
			}
		}
	};

	template <typename Base, typename T>
	inline basic_function<Base> force_cast(T& p) {
		return p;
	}

	template <typename Reference, bool IsMainReference = false>
	inline Reference get_default_handler(lua_State* L_) {
		if (is_stack_based_v<Reference> || L_ == nullptr)
			return Reference(L_, lua_nil);
		L_ = IsMainReference ? main_thread(L_, L_) : L_;
		lua_getglobal(L_, default_handler_name());
		auto pp = stack::pop_n(L_, 1);
		return Reference(L_, -1);
	}

	template <typename T>
	inline void set_default_handler(lua_State* L, const T& ref) {
		if (L == nullptr) {
			return;
		}
		if (!ref.valid()) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushnil(L);
			lua_setglobal(L, default_handler_name());
		}
		else {
			ref.push(L);
			lua_setglobal(L, default_handler_name());
		}
	}
}} // namespace sol::detail

#endif // SOL_PROTECTED_HANDLER_HPP
