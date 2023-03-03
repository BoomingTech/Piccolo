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

#ifndef SOL_LUA_VALUE_HPP
#define SOL_LUA_VALUE_HPP

#include <sol/stack.hpp>
#include <sol/reference.hpp>
#include <sol/make_reference.hpp>

namespace sol {
	struct lua_value {
	public:
		struct arr : detail::ebco<std::initializer_list<lua_value>> {
		private:
			using base_t = detail::ebco<std::initializer_list<lua_value>>;

		public:
			using base_t::base_t;
		};

	private:
		template <typename T>
		using is_reference_or_lua_value_init_list
		     = meta::any<meta::is_specialization_of<T, std::initializer_list>, std::is_same<T, reference>, std::is_same<T, arr>>;

		template <typename T>
		using is_lua_value_single_constructible = meta::any<std::is_same<T, lua_value>, is_reference_or_lua_value_init_list<T>>;

		static lua_State*& thread_local_lua_state() {
#if SOL_IS_ON(SOL_USE_THREAD_LOCAL)
			static thread_local lua_State* L = nullptr;
#else
			static lua_State* L = nullptr;
#endif
			return L;
		}

		reference ref_value;

	public:
		static void set_lua_state(lua_State* L) {
			thread_local_lua_state() = L;
		}

		template <typename T, meta::disable<is_reference_or_lua_value_init_list<meta::unqualified_t<T>>> = meta::enabler>
		lua_value(lua_State* L_, T&& value) : lua_value(((set_lua_state(L_)), std::forward<T>(value))) {
		}

		template <typename T, meta::disable<is_lua_value_single_constructible<meta::unqualified_t<T>>> = meta::enabler>
		lua_value(T&& value) : ref_value(make_reference(thread_local_lua_state(), std::forward<T>(value))) {
		}

		lua_value(lua_State* L_, std::initializer_list<std::pair<lua_value, lua_value>> il)
		: lua_value([&L_, &il]() {
			set_lua_state(L_);
			return std::move(il);
		}()) {
		}

		lua_value(std::initializer_list<std::pair<lua_value, lua_value>> il) : ref_value(make_reference(thread_local_lua_state(), std::move(il))) {
		}

		lua_value(lua_State* L_, arr il)
		: lua_value([&L_, &il]() {
			set_lua_state(L_);
			return std::move(il);
		}()) {
		}

		lua_value(arr il) : ref_value(make_reference(thread_local_lua_state(), std::move(il.value()))) {
		}

		lua_value(lua_State* L_, reference r)
		: lua_value([&L_, &r]() {
			set_lua_state(L_);
			return std::move(r);
		}()) {
		}

		lua_value(reference r) : ref_value(std::move(r)) {
		}

		lua_value(const lua_value&) noexcept = default;
		lua_value(lua_value&&) = default;
		lua_value& operator=(const lua_value&) = default;
		lua_value& operator=(lua_value&&) = default;

		const reference& value() const& {
			return ref_value;
		}

		reference& value() & {
			return ref_value;
		}

		reference&& value() && {
			return std::move(ref_value);
		}

		template <typename T>
		decltype(auto) as() const {
			ref_value.push();
			return stack::pop<T>(ref_value.lua_state());
		}

		template <typename T>
		bool is() const {
			int r = ref_value.registry_index();
			if (r == LUA_REFNIL)
				return meta::any_same<meta::unqualified_t<T>, lua_nil_t, nullopt_t, std::nullptr_t>::value ? true : false;
			if (r == LUA_NOREF)
				return false;
			auto pp = stack::push_pop(ref_value);
			return stack::check<T>(ref_value.lua_state(), -1, &no_panic);
		}
	};

	using array_value = typename lua_value::arr;

	namespace stack {
		template <>
		struct unqualified_pusher<lua_value> {
			static int push(lua_State* L, const lua_value& lv) {
				return stack::push(L, lv.value());
			}

			static int push(lua_State* L, lua_value&& lv) {
				return stack::push(L, std::move(lv).value());
			}
		};

		template <>
		struct unqualified_getter<lua_value> {
			static lua_value get(lua_State* L, int index, record& tracking) {
				return lua_value(L, stack::get<reference>(L, index, tracking));
			}
		};
	} // namespace stack
} // namespace sol

#endif // SOL_LUA_VALUE_HPP
