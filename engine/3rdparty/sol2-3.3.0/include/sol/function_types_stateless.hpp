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

#ifndef SOL_FUNCTION_TYPES_STATELESS_HPP
#define SOL_FUNCTION_TYPES_STATELESS_HPP

#include <sol/stack.hpp>
#include <sol/call.hpp>
#include <sol/bind_traits.hpp>

namespace sol { namespace function_detail {
	template <typename Function>
	struct upvalue_free_function {
		using function_type = std::remove_pointer_t<std::decay_t<Function>>;
		using traits_type = meta::bind_traits<function_type>;

		static int real_call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			auto udata = stack::stack_detail::get_as_upvalues<function_type*>(L);
			function_type* fx = udata.first;
			return call_detail::call_wrapped<void, true, false>(L, fx);
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L) {
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}
	};

	template <typename T, typename Function>
	struct upvalue_member_function {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef lua_bind_traits<function_type> traits_type;

		static int real_call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			// Layout:
			// idx 1...n: verbatim data of member function pointer
			// idx n + 1: is the object's void pointer
			// We don't need to store the size, because the other side is templated
			// with the same member function pointer type
			function_type& memfx = stack::get<user<function_type>>(L, upvalue_index(2));
			auto& item = *static_cast<T*>(stack::get<void*>(L, upvalue_index(3)));
			return call_detail::call_wrapped<T, true, false, -1>(L, memfx, item);
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			return call(L);
		}
	};

	template <typename T, typename Function>
	struct upvalue_member_variable {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef lua_bind_traits<function_type> traits_type;

		static int real_call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			// Layout:
			// idx 1...n: verbatim data of member variable pointer
			// idx n + 1: is the object's void pointer
			// We don't need to store the size, because the other side is templated
			// with the same member function pointer type
			auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L);
			auto objdata = stack::stack_detail::get_as_upvalues<T*>(L, memberdata.second);
			auto& mem = *objdata.first;
			function_type& var = memberdata.first;
			switch (lua_gettop(L)) {
			case 0:
				return call_detail::call_wrapped<T, true, false, -1>(L, var, mem);
			case 1:
				return call_detail::call_wrapped<T, false, false, -1>(L, var, mem);
			default:
				return luaL_error(L, "sol: incorrect number of arguments to member variable function");
			}
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			return call(L);
		}
	};

	template <typename T, typename Function>
	struct upvalue_member_variable<T, readonly_wrapper<Function>> {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef lua_bind_traits<function_type> traits_type;

		static int real_call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			// Layout:
			// idx 1...n: verbatim data of member variable pointer
			// idx n + 1: is the object's void pointer
			// We don't need to store the size, because the other side is templated
			// with the same member function pointer type
			auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L);
			auto objdata = stack::stack_detail::get_as_upvalues<T*>(L, memberdata.second);
			auto& mem = *objdata.first;
			function_type& var = memberdata.first;
			switch (lua_gettop(L)) {
			case 0:
				return call_detail::call_wrapped<T, true, false, -1>(L, var, mem);
			default:
				return luaL_error(L, "sol: incorrect number of arguments to member variable function");
			}
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			return call(L);
		}
	};

	template <typename T, typename Function>
	struct upvalue_this_member_function {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef lua_bind_traits<function_type> traits_type;

		static int real_call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			// Layout:
			// idx 1...n: verbatim data of member variable pointer
			function_type& memfx = stack::get<user<function_type>>(L, upvalue_index(2));
			return call_detail::call_wrapped<T, false, false>(L, memfx);
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			return call(L);
		}
	};

	template <typename T, typename Function>
	struct upvalue_this_member_variable {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;

		static int real_call(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			// Layout:
			// idx 1...n: verbatim data of member variable pointer
			auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L);
			function_type& var = memberdata.first;
			switch (lua_gettop(L)) {
			case 1:
				return call_detail::call_wrapped<T, true, false>(L, var);
			case 2:
				return call_detail::call_wrapped<T, false, false>(L, var);
			default:
				return luaL_error(L, "sol: incorrect number of arguments to member variable function");
			}
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			return call(L);
		}
	};

	template <typename T, typename Function>
	struct upvalue_this_member_variable<T, readonly_wrapper<Function>> {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef lua_bind_traits<function_type> traits_type;

		static int real_call(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			// Layout:
			// idx 1...n: verbatim data of member variable pointer
			auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L);
			function_type& var = memberdata.first;
			switch (lua_gettop(L)) {
			case 1:
				return call_detail::call_wrapped<T, true, false>(L, var);
			default:
				return luaL_error(L, "sol: incorrect number of arguments to member variable function");
			}
		}

		template <bool is_yielding, bool no_trampoline>
		static int call(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			int nr;
			if constexpr (no_trampoline) {
				nr = real_call(L);
			}
			else {
				nr = detail::typed_static_trampoline<decltype(&real_call), (&real_call)>(L);
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			return call(L);
		}
	};
}} // namespace sol::function_detail

#endif // SOL_FUNCTION_TYPES_STATELESS_HPP
