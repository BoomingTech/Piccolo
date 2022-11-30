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

#ifndef SOL_FUNCTION_TYPES_STATEFUL_HPP
#define SOL_FUNCTION_TYPES_STATEFUL_HPP

#include <sol/function_types_core.hpp>
#include <sol/call.hpp>

namespace sol { namespace function_detail {
	template <typename Func, bool is_yielding, bool no_trampoline>
	struct functor_function {
		typedef std::decay_t<meta::unwrap_unqualified_t<Func>> function_type;
		function_type invocation;

		template <typename... Args>
		functor_function(function_type f, Args&&... args) noexcept(std::is_nothrow_constructible_v<function_type, function_type, Args...>)
		: invocation(std::move(f), std::forward<Args>(args)...) {
		}

		static int call(lua_State* L, functor_function& self) noexcept(noexcept(call_detail::call_wrapped<void, true, false>(L, self.invocation))) {
			int nr = call_detail::call_wrapped<void, true, false>(L, self.invocation);
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L) noexcept(noexcept(call_detail::call_wrapped<void, true, false>(L, invocation))) {
			if constexpr (no_trampoline) {
				return call(L, *this);
			}
			else {
				return detail::trampoline(L, &call, *this);
			}
		}
	};

	template <typename T, typename Function, bool is_yielding, bool no_trampoline>
	struct member_function {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef meta::function_return_t<function_type> return_type;
		typedef meta::function_args_t<function_type> args_lists;
		using traits_type = meta::bind_traits<function_type>;
		function_type invocation;
		T member;

		template <typename... Args>
		member_function(function_type f, Args&&... args) noexcept(
			std::is_nothrow_constructible_v<function_type, function_type>&& std::is_nothrow_constructible_v<T, Args...>)
		: invocation(std::move(f)), member(std::forward<Args>(args)...) {
		}

		static int call(lua_State* L, member_function& self)
#if SOL_IS_ON(SOL_COMPILER_VCXX)
		// MSVC is broken, what a surprise...
#else
			noexcept(traits_type::is_noexcept)
#endif
		{
			int nr = call_detail::call_wrapped<T, true, false, -1>(L, self.invocation, detail::unwrap(detail::deref(self.member)));
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
			if constexpr (no_trampoline) {
				return call(L, *this);
			}
			else {
				return detail::trampoline(L, &call, *this);
			}
		}
	};

	template <typename T, typename Function, bool is_yielding, bool no_trampoline>
	struct member_variable {
		typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
		typedef typename meta::bind_traits<function_type>::return_type return_type;
		typedef typename meta::bind_traits<function_type>::args_list args_lists;
		function_type var;
		T member;
		typedef std::add_lvalue_reference_t<meta::unwrapped_t<std::remove_reference_t<decltype(detail::deref(member))>>> M;

		template <typename... Args>
		member_variable(function_type v, Args&&... args) noexcept(
			std::is_nothrow_constructible_v<function_type, function_type>&& std::is_nothrow_constructible_v<T, Args...>)
		: var(std::move(v)), member(std::forward<Args>(args)...) {
		}

		static int call(lua_State* L, member_variable& self) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			int nr;
			{
				M mem = detail::unwrap(detail::deref(self.member));
				switch (lua_gettop(L)) {
				case 0:
					nr = call_detail::call_wrapped<T, true, false, -1>(L, self.var, mem);
					break;
				case 1:
					nr = call_detail::call_wrapped<T, false, false, -1>(L, self.var, mem);
					break;
				default:
					nr = luaL_error(L, "sol: incorrect number of arguments to member variable function");
					break;
				}
			}
			if (is_yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		int operator()(lua_State* L) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			if constexpr (no_trampoline) {
				return call(L, *this);
			}
			else {
				return detail::trampoline(L, &call, *this);
			}
		}
	};
}} // namespace sol::function_detail

#endif // SOL_FUNCTION_TYPES_STATEFUL_HPP
