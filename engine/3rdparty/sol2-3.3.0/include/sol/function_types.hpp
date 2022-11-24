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

#ifndef SOL_FUNCTION_TYPES_HPP
#define SOL_FUNCTION_TYPES_HPP

#include <sol/function_types_core.hpp>
#include <sol/function_types_templated.hpp>
#include <sol/function_types_stateless.hpp>
#include <sol/function_types_stateful.hpp>
#include <sol/function_types_overloaded.hpp>
#include <sol/resolve.hpp>
#include <sol/call.hpp>

namespace sol {
	namespace function_detail {
		template <typename T>
		struct class_indicator {
			using type = T;
		};

		struct call_indicator { };

		template <bool yielding>
		int lua_c_wrapper(lua_State* L) {
			lua_CFunction cf = lua_tocfunction(L, lua_upvalueindex(2));
			int nr = cf(L);
			if constexpr (yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		template <bool yielding>
		int lua_c_noexcept_wrapper(lua_State* L) noexcept {
			detail::lua_CFunction_noexcept cf = reinterpret_cast<detail::lua_CFunction_noexcept>(lua_tocfunction(L, lua_upvalueindex(2)));
			int nr = cf(L);
			if constexpr (yielding) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		struct c_function_invocation { };

		template <bool is_yielding, bool no_trampoline, typename Fx, typename... Args>
		void select(lua_State* L, Fx&& fx, Args&&... args);

		template <bool is_yielding, bool no_trampoline, typename Fx, typename... Args>
		void select_set_fx(lua_State* L, Args&&... args) {
			lua_CFunction freefunc = no_trampoline ? function_detail::call<meta::unqualified_t<Fx>, 2, is_yielding>
			                                       : detail::static_trampoline<function_detail::call<meta::unqualified_t<Fx>, 2, is_yielding>>;

			int upvalues = 0;
			upvalues += stack::push(L, nullptr);
			upvalues += stack::push<user<Fx>>(L, std::forward<Args>(args)...);
			stack::push(L, c_closure(freefunc, upvalues));
		}

		template <bool is_yielding, bool no_trampoline, typename R, typename... A, typename Fx, typename... Args>
		void select_convertible(types<R(A...)>, lua_State* L, Fx&& fx, Args&&... args) {
			using dFx = std::decay_t<meta::unwrap_unqualified_t<Fx>>;
			using fx_ptr_t = R (*)(A...);
			constexpr bool is_convertible = std::is_convertible_v<dFx, fx_ptr_t>;
			if constexpr (is_convertible) {
				fx_ptr_t fxptr = detail::unwrap(std::forward<Fx>(fx));
				select<is_yielding, no_trampoline>(L, std::move(fxptr), std::forward<Args>(args)...);
			}
			else {
				using F = function_detail::functor_function<dFx, false, true>;
				select_set_fx<is_yielding, no_trampoline, F>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
		}

		template <bool is_yielding, bool no_trampoline, typename Fx, typename... Args>
		void select_convertible(types<>, lua_State* L, Fx&& fx, Args&&... args) {
			typedef meta::function_signature_t<meta::unwrap_unqualified_t<Fx>> Sig;
			select_convertible<is_yielding, no_trampoline>(types<Sig>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
		}

		template <bool is_yielding, bool no_trampoline, typename Fx, typename... Args>
		void select_member_variable(lua_State* L, Fx&& fx, Args&&... args) {
			using uFx = meta::unqualified_t<Fx>;
			if constexpr (sizeof...(Args) < 1) {
				using C = typename meta::bind_traits<uFx>::object_type;
				lua_CFunction freefunc = &function_detail::upvalue_this_member_variable<C, Fx>::template call<is_yielding, no_trampoline>;

				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::stack_detail::push_as_upvalues(L, fx);
				stack::push(L, c_closure(freefunc, upvalues));
			}
			else if constexpr (sizeof...(Args) < 2) {
				using Tu = typename meta::meta_detail::unqualified_non_alias<Args...>::type;
				constexpr bool is_reference = meta::is_specialization_of_v<Tu, std::reference_wrapper> || std::is_pointer_v<Tu>;
				if constexpr (meta::is_specialization_of_v<Tu, function_detail::class_indicator>) {
					lua_CFunction freefunc
					     = &function_detail::upvalue_this_member_variable<typename Tu::type, Fx>::template call<is_yielding, no_trampoline>;

					int upvalues = 0;
					upvalues += stack::push(L, nullptr);
					upvalues += stack::stack_detail::push_as_upvalues(L, fx);
					stack::push(L, c_closure(freefunc, upvalues));
				}
				else if constexpr (is_reference) {
					typedef std::decay_t<Fx> dFx;
					dFx memfxptr(std::forward<Fx>(fx));
					auto userptr = detail::ptr(std::forward<Args>(args)...);
					lua_CFunction freefunc = &function_detail::upvalue_member_variable<std::decay_t<decltype(*userptr)>,
					     meta::unqualified_t<Fx>>::template call<is_yielding, no_trampoline>;

					int upvalues = 0;
					upvalues += stack::push(L, nullptr);
					upvalues += stack::stack_detail::push_as_upvalues(L, memfxptr);
					upvalues += stack::push(L, static_cast<void const*>(userptr));
					stack::push(L, c_closure(freefunc, upvalues));
				}
				else {
					using clean_fx = std::remove_pointer_t<std::decay_t<Fx>>;
					using F = function_detail::member_variable<Tu, clean_fx, is_yielding, no_trampoline>;
					select_set_fx<is_yielding, no_trampoline, F>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
				}
			}
			else {
				using C = typename meta::bind_traits<uFx>::object_type;
				using clean_fx = std::remove_pointer_t<std::decay_t<Fx>>;
				using F = function_detail::member_variable<C, clean_fx, is_yielding, no_trampoline>;
				select_set_fx<is_yielding, no_trampoline, F>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
		}

		template <bool is_yielding, bool no_trampoline, typename Fx, typename T, typename... Args>
		void select_member_function_with(lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
			using dFx = std::decay_t<Fx>;
			using Tu = meta::unqualified_t<T>;
			if constexpr (meta::is_specialization_of_v<Tu, function_detail::class_indicator>) {
				(void)obj;
				using C = typename Tu::type;
				lua_CFunction freefunc = &function_detail::upvalue_this_member_function<C, dFx>::template call<is_yielding, no_trampoline>;

				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<dFx>>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
				stack::push(L, c_closure(freefunc, upvalues));
			}
			else {
				constexpr bool is_reference = meta::is_specialization_of_v<Tu, std::reference_wrapper> || std::is_pointer_v<Tu>;
				if constexpr (is_reference) {
					auto userptr = detail::ptr(std::forward<T>(obj));
					lua_CFunction freefunc
					     = &function_detail::upvalue_member_function<std::decay_t<decltype(*userptr)>, dFx>::template call<is_yielding, no_trampoline>;

					int upvalues = 0;
					upvalues += stack::push(L, nullptr);
					upvalues += stack::push<user<dFx>>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
					upvalues += stack::push(L, lightuserdata_value(static_cast<void*>(userptr)));
					stack::push(L, c_closure(freefunc, upvalues));
				}
				else {
					using F = function_detail::member_function<Tu, dFx, is_yielding, no_trampoline>;
					select_set_fx<is_yielding, no_trampoline, F>(L, std::forward<Fx>(fx), std::forward<T>(obj), std::forward<Args>(args)...);
				}
			}
		}

		template <bool is_yielding, bool no_trampoline, typename Fx, typename... Args>
		void select_member_function(lua_State* L, Fx&& fx, Args&&... args) {
			using dFx = std::decay_t<Fx>;
			if constexpr (sizeof...(Args) < 1) {
				using C = typename meta::bind_traits<meta::unqualified_t<Fx>>::object_type;
				lua_CFunction freefunc = &function_detail::upvalue_this_member_function<C, dFx>::template call<is_yielding, no_trampoline>;

				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<dFx>>(L, std::forward<Fx>(fx));
				stack::push(L, c_closure(freefunc, upvalues));
			}
			else {
				select_member_function_with<is_yielding, no_trampoline>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
		}

		template <bool is_yielding, bool no_trampoline, typename Fx, typename... Args>
		void select(lua_State* L, Fx&& fx, Args&&... args) {
			using uFx = meta::unqualified_t<Fx>;
			if constexpr (is_lua_reference_v<uFx>) {
				// TODO: hoist into lambda in this case for yielding???
				stack::push(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
			else if constexpr (is_lua_c_function_v<uFx>) {
				if constexpr (no_trampoline) {
					if (is_yielding) {
						int upvalues = 0;
						upvalues += stack::push(L, nullptr);
						upvalues += stack::push(L, std::forward<Fx>(fx));
#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
						if constexpr (std::is_nothrow_invocable_r_v<int, uFx, lua_State*>) {
							detail::lua_CFunction_noexcept cf = &lua_c_noexcept_wrapper<true>;
							lua_pushcclosure(L, reinterpret_cast<lua_CFunction>(cf), upvalues);
						}
						else
#endif
						{
							lua_CFunction cf = &function_detail::lua_c_wrapper<true>;
							lua_pushcclosure(L, cf, upvalues);
						}
					}
					else {
						lua_pushcclosure(L, std::forward<Fx>(fx), 0);
					}
				}
				else {
					int upvalues = 0;
					upvalues += stack::push(L, nullptr);
					upvalues += stack::push(L, std::forward<Fx>(fx));
#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
					if constexpr (std::is_nothrow_invocable_r_v<int, uFx, lua_State*>) {
						detail::lua_CFunction_noexcept cf = &lua_c_noexcept_wrapper<is_yielding>;
						lua_pushcclosure(L, reinterpret_cast<lua_CFunction>(cf), upvalues);
					}
					else {
						lua_CFunction cf = &function_detail::lua_c_wrapper<is_yielding>;
						lua_pushcclosure(L, cf, upvalues);
					}
#else
					lua_CFunction cf = &function_detail::lua_c_wrapper<is_yielding>;
					lua_pushcclosure(L, cf, upvalues);
#endif
				}
			}
			else if constexpr (std::is_function_v<std::remove_pointer_t<uFx>>) {
				std::decay_t<Fx> target(std::forward<Fx>(fx), std::forward<Args>(args)...);
				lua_CFunction freefunc = &function_detail::upvalue_free_function<Fx>::template call<is_yielding, no_trampoline>;

				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::stack_detail::push_as_upvalues(L, target);
				stack::push(L, c_closure(freefunc, upvalues));
			}
			else if constexpr (std::is_member_function_pointer_v<uFx>) {
				select_member_function<is_yielding, no_trampoline>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
			else if constexpr (meta::is_member_object_v<uFx>) {
				select_member_variable<is_yielding, no_trampoline>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
			else {
				select_convertible<is_yielding, no_trampoline>(types<>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
		}
	} // namespace function_detail

	namespace stack {
		template <typename... Sigs>
		struct unqualified_pusher<function_sig<Sigs...>> {
			template <bool is_yielding, typename Arg0, typename... Args>
			static int push_yielding(lua_State* L, Arg0&& arg0, Args&&... args) {
				if constexpr (meta::is_specialization_of_v<meta::unqualified_t<Arg0>, std::function>) {
					if constexpr (is_yielding) {
						return stack::push<meta::unqualified_t<Arg0>>(L, detail::yield_tag, std::forward<Arg0>(arg0), std::forward<Args>(args)...);
					}
					else {
						return stack::push(L, std::forward<Arg0>(arg0), std::forward<Args>(args)...);
					}
				}
				else {
					function_detail::select<is_yielding, false>(L, std::forward<Arg0>(arg0), std::forward<Args>(args)...);
					return 1;
				}
			}

			template <typename Arg0, typename... Args>
			static int push(lua_State* L, Arg0&& arg0, Args&&... args) {
				if constexpr (std::is_same_v<meta::unqualified_t<Arg0>, detail::yield_tag_t>) {
					push_yielding<true>(L, std::forward<Args>(args)...);
				}
				else if constexpr (meta::is_specialization_of_v<meta::unqualified_t<Arg0>, yielding_t>) {
					push_yielding<true>(L, std::forward<Arg0>(arg0).func, std::forward<Args>(args)...);
				}
				else {
					push_yielding<false>(L, std::forward<Arg0>(arg0), std::forward<Args>(args)...);
				}
				return 1;
			}
		};

		template <typename T>
		struct unqualified_pusher<yielding_t<T>> {
			template <typename... Args>
			static int push(lua_State* L, const yielding_t<T>& f, Args&&... args) {
				if constexpr (meta::is_specialization_of_v<meta::unqualified_t<T>, std::function>) {
					return stack::push<T>(L, detail::yield_tag, f.func, std::forward<Args>(args)...);
				}
				else {
					function_detail::select<true, false>(L, f.func, std::forward<Args>(args)...);
					return 1;
				}
			}

			template <typename... Args>
			static int push(lua_State* L, yielding_t<T>&& f, Args&&... args) {
				if constexpr (meta::is_specialization_of_v<meta::unqualified_t<T>, std::function>) {
					return stack::push<T>(L, detail::yield_tag, std::move(f.func), std::forward<Args>(args)...);
				}
				else {
					function_detail::select<true, false>(L, std::move(f.func), std::forward<Args>(args)...);
					return 1;
				}
			}
		};

		template <typename T, typename... Args>
		struct unqualified_pusher<function_arguments<T, Args...>> {
			template <std::size_t... I, typename FP>
			static int push_func(std::index_sequence<I...>, lua_State* L, FP&& fp) {
				return stack::push<T>(L, std::get<I>(std::forward<FP>(fp).arguments)...);
			}

			static int push(lua_State* L, const function_arguments<T, Args...>& fp) {
				return push_func(std::make_index_sequence<sizeof...(Args)>(), L, fp);
			}

			static int push(lua_State* L, function_arguments<T, Args...>&& fp) {
				return push_func(std::make_index_sequence<sizeof...(Args)>(), L, std::move(fp));
			}
		};

		template <typename Signature>
		struct unqualified_pusher<std::function<Signature>> {
			using TargetFunctor = function_detail::functor_function<std::function<Signature>, false, true>;

			static int push(lua_State* L, detail::yield_tag_t, const std::function<Signature>& fx) {
				if (fx) {
					function_detail::select_set_fx<true, false, TargetFunctor>(L, fx);
					return 1;
				}
				return stack::push(L, lua_nil);
			}

			static int push(lua_State* L, detail::yield_tag_t, std::function<Signature>&& fx) {
				if (fx) {
					function_detail::select_set_fx<true, false, TargetFunctor>(L, std::move(fx));
					return 1;
				}
				return stack::push(L, lua_nil);
			}

			static int push(lua_State* L, const std::function<Signature>& fx) {
				if (fx) {
					function_detail::select_set_fx<false, false, TargetFunctor>(L, fx);
					return 1;
				}
				return stack::push(L, lua_nil);
			}

			static int push(lua_State* L, std::function<Signature>&& fx) {
				if (fx) {
					function_detail::select_set_fx<false, false, TargetFunctor>(L, std::move(fx));
					return 1;
				}
				return stack::push(L, lua_nil);
			}
		};

		template <typename Signature>
		struct unqualified_pusher<Signature, std::enable_if_t<meta::is_member_object_or_function_v<Signature>>> {
			template <typename... Args>
			static int push(lua_State* L, Args&&... args) {
				function_detail::select<false, false>(L, std::forward<Args>(args)...);
				return 1;
			}
		};

		template <typename Signature>
		struct unqualified_pusher<Signature,
		     std::enable_if_t<meta::all<std::is_function<std::remove_pointer_t<Signature>>, meta::neg<std::is_same<Signature, lua_CFunction>>,
		          meta::neg<std::is_same<Signature, std::remove_pointer_t<lua_CFunction>>>
#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
		          ,
		          meta::neg<std::is_same<Signature, detail::lua_CFunction_noexcept>>,
		          meta::neg<std::is_same<Signature, std::remove_pointer_t<detail::lua_CFunction_noexcept>>>
#endif // noexcept function types
		          >::value>> {
			template <typename F>
			static int push(lua_State* L, F&& f) {
				function_detail::select<false, true>(L, std::forward<F>(f));
				return 1;
			}
		};

		template <typename... Functions>
		struct unqualified_pusher<overload_set<Functions...>> {
			static int push(lua_State* L, overload_set<Functions...>&& set) {
				using F = function_detail::overloaded_function<0, Functions...>;
				function_detail::select_set_fx<false, false, F>(L, std::move(set.functions));
				return 1;
			}

			static int push(lua_State* L, const overload_set<Functions...>& set) {
				using F = function_detail::overloaded_function<0, Functions...>;
				function_detail::select_set_fx<false, false, F>(L, set.functions);
				return 1;
			}
		};

		template <typename T>
		struct unqualified_pusher<protect_t<T>> {
			static int push(lua_State* L, protect_t<T>&& pw) {
				lua_CFunction cf = call_detail::call_user<void, false, false, protect_t<T>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<protect_t<T>>>(L, std::move(pw.value));
				return stack::push(L, c_closure(cf, upvalues));
			}

			static int push(lua_State* L, const protect_t<T>& pw) {
				lua_CFunction cf = call_detail::call_user<void, false, false, protect_t<T>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<protect_t<T>>>(L, pw.value);
				return stack::push(L, c_closure(cf, upvalues));
			}
		};

		template <typename F, typename G>
		struct unqualified_pusher<property_wrapper<F, G>> {
			static int push(lua_State* L, property_wrapper<F, G>&& pw) {
				if constexpr (std::is_void_v<F>) {
					return stack::push(L, std::move(pw.write()));
				}
				else if constexpr (std::is_void_v<G>) {
					return stack::push(L, std::move(pw.read()));
				}
				else {
					return stack::push(L, overload(std::move(pw.read()), std::move(pw.write())));
				}
			}

			static int push(lua_State* L, const property_wrapper<F, G>& pw) {
				if constexpr (std::is_void_v<F>) {
					return stack::push(L, pw.write);
				}
				else if constexpr (std::is_void_v<G>) {
					return stack::push(L, pw.read);
				}
				else {
					return stack::push(L, overload(pw.read, pw.write));
				}
			}
		};

		template <typename T>
		struct unqualified_pusher<var_wrapper<T>> {
			static int push(lua_State* L, var_wrapper<T>&& vw) {
				return stack::push(L, std::move(vw.value()));
			}
			static int push(lua_State* L, const var_wrapper<T>& vw) {
				return stack::push(L, vw.value());
			}
		};

		template <typename... Functions>
		struct unqualified_pusher<factory_wrapper<Functions...>> {
			static int push(lua_State* L, const factory_wrapper<Functions...>& fw) {
				using F = function_detail::overloaded_function<0, Functions...>;
				function_detail::select_set_fx<false, false, F>(L, fw.functions);
				return 1;
			}

			static int push(lua_State* L, factory_wrapper<Functions...>&& fw) {
				using F = function_detail::overloaded_function<0, Functions...>;
				function_detail::select_set_fx<false, false, F>(L, std::move(fw.functions));
				return 1;
			}

			static int push(lua_State* L, const factory_wrapper<Functions...>& fw, function_detail::call_indicator) {
				using F = function_detail::overloaded_function<1, Functions...>;
				function_detail::select_set_fx<false, false, F>(L, fw.functions);
				return 1;
			}

			static int push(lua_State* L, factory_wrapper<Functions...>&& fw, function_detail::call_indicator) {
				using F = function_detail::overloaded_function<1, Functions...>;
				function_detail::select_set_fx<false, false, F>(L, std::move(fw.functions));
				return 1;
			}
		};

		template <>
		struct unqualified_pusher<no_construction> {
			static int push(lua_State* L, no_construction) {
				lua_CFunction cf = &function_detail::no_construction_error;
				return stack::push(L, cf);
			}

			static int push(lua_State* L, no_construction c, function_detail::call_indicator) {
				return push(L, c);
			}
		};

		template <typename T>
		struct unqualified_pusher<detail::tagged<T, no_construction>> {
			static int push(lua_State* L, detail::tagged<T, no_construction>) {
				lua_CFunction cf = &function_detail::no_construction_error;
				return stack::push(L, cf);
			}

			static int push(lua_State* L, no_construction, function_detail::call_indicator) {
				lua_CFunction cf = &function_detail::no_construction_error;
				return stack::push(L, cf);
			}
		};

		template <typename T, typename... Lists>
		struct unqualified_pusher<detail::tagged<T, constructor_list<Lists...>>> {
			static int push(lua_State* L, detail::tagged<T, constructor_list<Lists...>>) {
				lua_CFunction cf = call_detail::construct<T, detail::default_safe_function_calls, true, Lists...>;
				return stack::push(L, cf);
			}

			static int push(lua_State* L, constructor_list<Lists...>) {
				lua_CFunction cf = call_detail::construct<T, detail::default_safe_function_calls, true, Lists...>;
				return stack::push(L, cf);
			}
		};

		template <typename L0, typename... Lists>
		struct unqualified_pusher<constructor_list<L0, Lists...>> {
			typedef constructor_list<L0, Lists...> cl_t;
			static int push(lua_State* L, cl_t cl) {
				typedef typename meta::bind_traits<L0>::return_type T;
				return stack::push<detail::tagged<T, cl_t>>(L, cl);
			}
		};

		template <typename T, typename... Fxs>
		struct unqualified_pusher<detail::tagged<T, constructor_wrapper<Fxs...>>> {
			static int push(lua_State* L, detail::tagged<T, constructor_wrapper<Fxs...>>&& c) {
				return push(L, std::move(c.value()));
			}

			static int push(lua_State* L, const detail::tagged<T, const constructor_wrapper<Fxs...>>& c) {
				return push(L, c.value());
			}

			static int push(lua_State* L, constructor_wrapper<Fxs...>&& c) {
				lua_CFunction cf = call_detail::call_user<T, false, false, constructor_wrapper<Fxs...>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<constructor_wrapper<Fxs...>>>(L, std::move(c));
				return stack::push(L, c_closure(cf, upvalues));
			}

			static int push(lua_State* L, const constructor_wrapper<Fxs...>& c) {
				lua_CFunction cf = call_detail::call_user<T, false, false, constructor_wrapper<Fxs...>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<constructor_wrapper<Fxs...>>>(L, c);
				return stack::push(L, c_closure(cf, upvalues));
			}
		};

		template <typename F, typename... Fxs>
		struct unqualified_pusher<constructor_wrapper<F, Fxs...>> {
			static int push(lua_State* L, const constructor_wrapper<F, Fxs...>& c) {
				typedef typename meta::bind_traits<F>::template arg_at<0> arg0;
				typedef meta::unqualified_t<std::remove_pointer_t<arg0>> T;
				return stack::push<detail::tagged<T, constructor_wrapper<F, Fxs...>>>(L, c);
			}

			static int push(lua_State* L, constructor_wrapper<F, Fxs...>&& c) {
				typedef typename meta::bind_traits<F>::template arg_at<0> arg0;
				typedef meta::unqualified_t<std::remove_pointer_t<arg0>> T;
				return stack::push<detail::tagged<T, constructor_wrapper<F, Fxs...>>>(L, std::move(c));
			}
		};

		template <typename T>
		struct unqualified_pusher<detail::tagged<T, destructor_wrapper<void>>> {
			static int push(lua_State* L, destructor_wrapper<void>) {
				lua_CFunction cf = detail::usertype_alloc_destroy<T>;
				return stack::push(L, cf);
			}
		};

		template <typename T, typename Fx>
		struct unqualified_pusher<detail::tagged<T, destructor_wrapper<Fx>>> {
			static int push(lua_State* L, destructor_wrapper<Fx>&& c) {
				lua_CFunction cf = call_detail::call_user<T, false, false, destructor_wrapper<Fx>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<destructor_wrapper<Fx>>>(L, std::move(c));
				return stack::push(L, c_closure(cf, upvalues));
			}

			static int push(lua_State* L, const destructor_wrapper<Fx>& c) {
				lua_CFunction cf = call_detail::call_user<T, false, false, destructor_wrapper<Fx>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<destructor_wrapper<Fx>>>(L, c);
				return stack::push(L, c_closure(cf, upvalues));
			}
		};

		template <typename Fx>
		struct unqualified_pusher<destructor_wrapper<Fx>> {
			static int push(lua_State* L, destructor_wrapper<Fx>&& c) {
				lua_CFunction cf = call_detail::call_user<void, false, false, destructor_wrapper<Fx>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<destructor_wrapper<Fx>>>(L, std::move(c));
				return stack::push(L, c_closure(cf, upvalues));
			}

			static int push(lua_State* L, const destructor_wrapper<Fx>& c) {
				lua_CFunction cf = call_detail::call_user<void, false, false, destructor_wrapper<Fx>, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<destructor_wrapper<Fx>>>(L, c);
				return stack::push(L, c_closure(cf, upvalues));
			}
		};

		template <typename F, typename... Policies>
		struct unqualified_pusher<policy_wrapper<F, Policies...>> {
			using P = policy_wrapper<F, Policies...>;

			static int push(lua_State* L, const P& p) {
				lua_CFunction cf = call_detail::call_user<void, false, false, P, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<P>>(L, p);
				return stack::push(L, c_closure(cf, upvalues));
			}

			static int push(lua_State* L, P&& p) {
				lua_CFunction cf = call_detail::call_user<void, false, false, P, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<P>>(L, std::move(p));
				return stack::push(L, c_closure(cf, upvalues));
			}
		};

		template <typename T, typename F, typename... Policies>
		struct unqualified_pusher<detail::tagged<T, policy_wrapper<F, Policies...>>> {
			using P = policy_wrapper<F, Policies...>;
			using Tagged = detail::tagged<T, P>;

			static int push(lua_State* L, const Tagged& p) {
				lua_CFunction cf = call_detail::call_user<T, false, false, P, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<P>>(L, p.value());
				return stack::push(L, c_closure(cf, upvalues));
			}

			static int push(lua_State* L, Tagged&& p) {
				lua_CFunction cf = call_detail::call_user<T, false, false, P, 2>;
				int upvalues = 0;
				upvalues += stack::push(L, nullptr);
				upvalues += stack::push<user<P>>(L, std::move(p.value()));
				return stack::push(L, c_closure(cf, upvalues));
			}
		};

		template <typename T>
		struct unqualified_pusher<push_invoke_t<T>> {
			static int push(lua_State* L, push_invoke_t<T>&& pi) {
				if constexpr (std::is_invocable_v<std::add_rvalue_reference_t<T>, lua_State*>) {
					return stack::push(L, std::move(pi.value())(L));
				}
				else {
					return stack::push(L, std::move(pi.value())());
				}
			}

			static int push(lua_State* L, const push_invoke_t<T>& pi) {
				if constexpr (std::is_invocable_v<const T, lua_State*>) {
					return stack::push(L, pi.value()(L));
				}
				else {
					return stack::push(L, pi.value()());
				}
			}
		};

		namespace stack_detail {
			template <typename Function, typename Handler>
			bool check_function_pointer(lua_State* L, int index, Handler&& handler, record& tracking) noexcept {
#if SOL_IS_ON(SOL_GET_FUNCTION_POINTER_UNSAFE)
				tracking.use(1);
				bool success = lua_iscfunction(L, index) == 1;
				if (success) {
					// there must be at LEAST 2 upvalues; otherwise, we didn't serialize it.
					const char* upvalue_name = lua_getupvalue(L, index, 2);
					lua_pop(L, 1);
					success = upvalue_name != nullptr;
				}
				if (!success) {
					// expected type, actual type
					handler(
					     L, index, type::function, type_of(L, index), "type must be a Lua C Function gotten from a function pointer serialized by sol2");
				}
				return success;
#else
				(void)L;
				(void)index;
				(void)handler;
				(void)tracking;
				return false;
#endif
			}

			template <typename Function>
			Function* get_function_pointer(lua_State* L, int index, record& tracking) noexcept {
#if SOL_IS_ON(SOL_GET_FUNCTION_POINTER_UNSAFE)
				tracking.use(1);
				auto udata = stack::stack_detail::get_as_upvalues_using_function<Function*>(L, index);
				Function* fx = udata.first;
				return fx;
#else
				(void)L;
				(void)index;
				(void)tracking;
				static_assert(meta::meta_detail::always_true<Function>::value,
#if SOL_IS_DEFAULT_OFF(SOL_GET_FUNCTION_POINTER_UNSAFE)
				     "You are attempting to retrieve a function pointer type. "
				     "This is inherently unsafe in sol2. In order to do this, you must turn on the "
				     "SOL_GET_FUNCTION_POINTER_UNSAFE configuration macro, as detailed in the documentation. "
				     "Please be careful!"
#else
				     "You are attempting to retrieve a function pointer type. "
				     "You explicitly turned off the ability to do this by defining "
				     "SOL_GET_FUNCTION_POINTER_UNSAFE or similar to be off. "
				     "Please reconsider this!"
#endif
				);
				return nullptr;
#endif
			}
		} // namespace stack_detail
	}      // namespace stack
} // namespace sol

#endif // SOL_FUNCTION_TYPES_HPP
