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

#pragma once

#ifndef SOL_CALL_HPP
#define SOL_CALL_HPP

#include <sol/property.hpp>
#include <sol/protect.hpp>
#include <sol/wrapper.hpp>
#include <sol/trampoline.hpp>
#include <sol/policies.hpp>
#include <sol/stack.hpp>
#include <sol/unique_usertype_traits.hpp>

namespace sol {
	namespace u_detail {

	} // namespace u_detail

	namespace policy_detail {
		template <int I, int... In>
		inline void handle_policy(static_stack_dependencies<I, In...>, lua_State* L, int&) {
			if constexpr (sizeof...(In) == 0) {
				(void)L;
				return;
			}
			else {
				absolute_index ai(L, I);
				if (type_of(L, ai) != type::userdata) {
					return;
				}
				lua_createtable(L, static_cast<int>(sizeof...(In)), 0);
				stack_reference deps(L, -1);
				auto per_dep = [&L, &deps](int i) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
					luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
					lua_pushvalue(L, i);
					luaL_ref(L, deps.stack_index());
				};
				(void)per_dep;
				(void)detail::swallow { int(), (per_dep(In), int())... };
				lua_setuservalue(L, ai);
			}
		}

		template <int... In>
		inline void handle_policy(returns_self_with<In...>, lua_State* L, int& pushed) {
			pushed = stack::push(L, raw_index(1));
			handle_policy(static_stack_dependencies<-1, In...>(), L, pushed);
		}

		inline void handle_policy(const stack_dependencies& sdeps, lua_State* L, int&) {
			absolute_index ai(L, sdeps.target);
			if (type_of(L, ai) != type::userdata) {
				return;
			}
			lua_createtable(L, static_cast<int>(sdeps.size()), 0);
			stack_reference deps(L, -1);
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, static_cast<int>(sdeps.size()), detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			for (std::size_t i = 0; i < sdeps.size(); ++i) {
				lua_pushvalue(L, sdeps.stack_indices[i]);
				luaL_ref(L, deps.stack_index());
			}
			lua_setuservalue(L, ai);
		}

		template <typename P, meta::disable<std::is_base_of<detail::policy_base_tag, meta::unqualified_t<P>>> = meta::enabler>
		inline void handle_policy(P&& p, lua_State* L, int& pushed) {
			pushed = std::forward<P>(p)(L, pushed);
		}
	} // namespace policy_detail

	namespace function_detail {
		inline int no_construction_error(lua_State* L) {
			return luaL_error(L, "sol: cannot call this constructor (tagged as non-constructible)");
		}
	} // namespace function_detail

	namespace call_detail {

		template <typename R, typename W>
		inline auto& pick(std::true_type, property_wrapper<R, W>& f) {
			return f.read();
		}

		template <typename R, typename W>
		inline auto& pick(std::false_type, property_wrapper<R, W>& f) {
			return f.write();
		}

		template <typename T, typename List>
		struct void_call : void_call<T, meta::function_args_t<List>> { };

		template <typename T, typename... Args>
		struct void_call<T, types<Args...>> {
			static void call(Args...) {
			}
		};

		template <typename T, bool checked, bool clean_stack>
		struct constructor_match {
			T* obj_;
			reference* obj_lua_ref_;
			stack::stack_detail::undefined_metatable* p_umf_;

			constructor_match(T* obj_ptr, reference& obj_lua_ref, stack::stack_detail::undefined_metatable& umf)
			: obj_(obj_ptr), obj_lua_ref_(&obj_lua_ref), p_umf_(&umf) {
			}

			template <typename Fx, std::size_t I, typename... R, typename... Args>
			int operator()(types<Fx>, meta::index_value<I>, types<R...> r, types<Args...> a, lua_State* L, int, int start) const {
				detail::default_construct func {};
				int result = stack::call_into_lua<checked, clean_stack>(r, a, L, start, func, this->obj_);
				// construct userdata table
				// SPECIFICALLY, after we've created it successfully.
				// If the constructor exits for any reason we have to break things down...
				if constexpr (clean_stack) {
					obj_lua_ref_->push();
					(*this->p_umf_)();
					obj_lua_ref_->pop();
				}
				else {
					(*this->p_umf_)();
				}
				return result;
			}
		};

		namespace overload_detail {
			template <std::size_t... M, typename Match, typename... Args>
			inline int overload_match_arity(types<>, std::index_sequence<>, std::index_sequence<M...>, Match&&, lua_State* L, int, int, Args&&...) {
				return luaL_error(L, "sol: no matching function call takes this number of arguments and the specified types");
			}

			template <typename Fx, typename... Fxs, std::size_t I, std::size_t... In, std::size_t... M, typename Match, typename... Args>
			inline int overload_match_arity(types<Fx, Fxs...>, std::index_sequence<I, In...>, std::index_sequence<M...>, Match&& matchfx, lua_State* L,
			     int fxarity, int start, Args&&... args) {
				typedef lua_bind_traits<meta::unwrap_unqualified_t<Fx>> traits;
				typedef meta::tuple_types<typename traits::return_type> return_types;
				typedef typename traits::free_args_list args_list;
				// compile-time eliminate any functions that we know ahead of time are of improper arity
				if constexpr (!traits::runtime_variadics_t::value
				     && meta::find_in_pack_v<meta::index_value<traits::free_arity>, meta::index_value<M>...>::value) {
					return overload_match_arity(types<Fxs...>(),
					     std::index_sequence<In...>(),
					     std::index_sequence<M...>(),
					     std::forward<Match>(matchfx),
					     L,
					     fxarity,
					     start,
					     std::forward<Args>(args)...);
				}
				else {
					if constexpr (!traits::runtime_variadics_t::value) {
						if (traits::free_arity != fxarity) {
							return overload_match_arity(types<Fxs...>(),
							     std::index_sequence<In...>(),
							     std::index_sequence<traits::free_arity, M...>(),
							     std::forward<Match>(matchfx),
							     L,
							     fxarity,
							     start,
							     std::forward<Args>(args)...);
						}
					}
					stack::record tracking {};
					if (!stack::stack_detail::check_types(args_list(), L, start, &no_panic, tracking)) {
						return overload_match_arity(types<Fxs...>(),
						     std::index_sequence<In...>(),
						     std::index_sequence<M...>(),
						     std::forward<Match>(matchfx),
						     L,
						     fxarity,
						     start,
						     std::forward<Args>(args)...);
					}
					return matchfx(types<Fx>(), meta::index_value<I>(), return_types(), args_list(), L, fxarity, start, std::forward<Args>(args)...);
				}
			}

			template <std::size_t... M, typename Match, typename... Args>
			inline int overload_match_arity_single(
			     types<>, std::index_sequence<>, std::index_sequence<M...>, Match&& matchfx, lua_State* L, int fxarity, int start, Args&&... args) {
				return overload_match_arity(types<>(),
				     std::index_sequence<>(),
				     std::index_sequence<M...>(),
				     std::forward<Match>(matchfx),
				     L,
				     fxarity,
				     start,
				     std::forward<Args>(args)...);
			}

			template <typename Fx, std::size_t I, std::size_t... M, typename Match, typename... Args>
			inline int overload_match_arity_single(
			     types<Fx>, std::index_sequence<I>, std::index_sequence<M...>, Match&& matchfx, lua_State* L, int fxarity, int start, Args&&... args) {
				typedef lua_bind_traits<meta::unwrap_unqualified_t<Fx>> traits;
				typedef meta::tuple_types<typename traits::return_type> return_types;
				typedef typename traits::free_args_list args_list;
				// compile-time eliminate any functions that we know ahead of time are of improper arity
				if constexpr (!traits::runtime_variadics_t::value
				     && meta::find_in_pack_v<meta::index_value<traits::free_arity>, meta::index_value<M>...>::value) {
					return overload_match_arity(types<>(),
					     std::index_sequence<>(),
					     std::index_sequence<M...>(),
					     std::forward<Match>(matchfx),
					     L,
					     fxarity,
					     start,
					     std::forward<Args>(args)...);
				}
				if constexpr (!traits::runtime_variadics_t::value) {
					if (traits::free_arity != fxarity) {
						return overload_match_arity(types<>(),
						     std::index_sequence<>(),
						     std::index_sequence<traits::free_arity, M...>(),
						     std::forward<Match>(matchfx),
						     L,
						     fxarity,
						     start,
						     std::forward<Args>(args)...);
					}
				}
				return matchfx(types<Fx>(), meta::index_value<I>(), return_types(), args_list(), L, fxarity, start, std::forward<Args>(args)...);
			}

			template <typename Fx, typename Fx1, typename... Fxs, std::size_t I, std::size_t I1, std::size_t... In, std::size_t... M, typename Match,
			     typename... Args>
			inline int overload_match_arity_single(types<Fx, Fx1, Fxs...>, std::index_sequence<I, I1, In...>, std::index_sequence<M...>, Match&& matchfx,
			     lua_State* L, int fxarity, int start, Args&&... args) {
				typedef lua_bind_traits<meta::unwrap_unqualified_t<Fx>> traits;
				typedef meta::tuple_types<typename traits::return_type> return_types;
				typedef typename traits::free_args_list args_list;
				// compile-time eliminate any functions that we know ahead of time are of improper arity
				if constexpr (!traits::runtime_variadics_t::value
				     && meta::find_in_pack_v<meta::index_value<traits::free_arity>, meta::index_value<M>...>::value) {
					return overload_match_arity(types<Fx1, Fxs...>(),
					     std::index_sequence<I1, In...>(),
					     std::index_sequence<M...>(),
					     std::forward<Match>(matchfx),
					     L,
					     fxarity,
					     start,
					     std::forward<Args>(args)...);
				}
				else {
					if constexpr (!traits::runtime_variadics_t::value) {
						if (traits::free_arity != fxarity) {
							return overload_match_arity(types<Fx1, Fxs...>(),
							     std::index_sequence<I1, In...>(),
							     std::index_sequence<traits::free_arity, M...>(),
							     std::forward<Match>(matchfx),
							     L,
							     fxarity,
							     start,
							     std::forward<Args>(args)...);
						}
					}
					stack::record tracking {};
					if (!stack::stack_detail::check_types(args_list(), L, start, &no_panic, tracking)) {
						return overload_match_arity(types<Fx1, Fxs...>(),
						     std::index_sequence<I1, In...>(),
						     std::index_sequence<M...>(),
						     std::forward<Match>(matchfx),
						     L,
						     fxarity,
						     start,
						     std::forward<Args>(args)...);
					}
					return matchfx(types<Fx>(), meta::index_value<I>(), return_types(), args_list(), L, fxarity, start, std::forward<Args>(args)...);
				}
			}
		} // namespace overload_detail

		template <typename... Functions, typename Match, typename... Args>
		inline int overload_match_arity(Match&& matchfx, lua_State* L, int fxarity, int start, Args&&... args) {
			return overload_detail::overload_match_arity_single(types<Functions...>(),
			     std::make_index_sequence<sizeof...(Functions)>(),
			     std::index_sequence<>(),
			     std::forward<Match>(matchfx),
			     L,
			     fxarity,
			     start,
			     std::forward<Args>(args)...);
		}

		template <typename... Functions, typename Match, typename... Args>
		inline int overload_match(Match&& matchfx, lua_State* L, int start, Args&&... args) {
			int fxarity = lua_gettop(L) - (start - 1);
			return overload_match_arity<Functions...>(std::forward<Match>(matchfx), L, fxarity, start, std::forward<Args>(args)...);
		}

		template <typename T, typename... TypeLists, typename Match, typename... Args>
		inline int construct_match(Match&& matchfx, lua_State* L, int fxarity, int start, Args&&... args) {
			// use same overload resolution matching as all other parts of the framework
			return overload_match_arity<decltype(void_call<T, TypeLists>::call)...>(
			     std::forward<Match>(matchfx), L, fxarity, start, std::forward<Args>(args)...);
		}

		template <typename T, bool checked, bool clean_stack, typename... TypeLists>
		inline int construct_trampolined(lua_State* L) {
			static const auto& meta = usertype_traits<T>::metatable();
			int argcount = lua_gettop(L);
			call_syntax syntax = argcount > 0 ? stack::get_call_syntax(L, usertype_traits<T>::user_metatable(), 1) : call_syntax::dot;
			argcount -= static_cast<int>(syntax);

			T* obj = detail::usertype_allocate<T>(L);
			reference userdataref(L, -1);
			stack::stack_detail::undefined_metatable umf(L, &meta[0], &stack::stack_detail::set_undefined_methods_on<T>);

			// put userdata at the first index
			lua_insert(L, 1);
			construct_match<T, TypeLists...>(constructor_match<T, checked, clean_stack>(obj, userdataref, umf), L, argcount, 1 + static_cast<int>(syntax));

			userdataref.push();
			return 1;
		}

		template <typename T, bool checked, bool clean_stack, typename... TypeLists>
		inline int construct(lua_State* L) {
			return detail::static_trampoline<&construct_trampolined<T, checked, clean_stack, TypeLists...>>(L);
		}

		template <typename F, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename = void>
		struct agnostic_lua_call_wrapper {
			template <typename Fx, typename... Args>
			static int call(lua_State* L, Fx&& f, Args&&... args) {
				using uFx = meta::unqualified_t<Fx>;
				static constexpr bool is_ref = is_lua_reference_v<uFx>;
				if constexpr (is_ref) {
					if constexpr (is_index) {
						return stack::push(L, std::forward<Fx>(f), std::forward<Args>(args)...);
					}
					else {
						std::forward<Fx>(f) = stack::unqualified_get<F>(L, boost + (is_variable ? 3 : 1));
						return 0;
					}
				}
				else {
					using wrap = wrapper<uFx>;
					using traits_type = typename wrap::traits_type;
					using fp_t = typename traits_type::function_pointer_type;
					constexpr bool is_function_pointer_convertible = std::is_class_v<uFx> && std::is_convertible_v<std::decay_t<Fx>, fp_t>;
					if constexpr (is_function_pointer_convertible) {
						fp_t fx = f;
						return agnostic_lua_call_wrapper<fp_t, is_index, is_variable, checked, boost, clean_stack> {}.call(
						     L, fx, std::forward<Args>(args)...);
					}
					else {
						using returns_list = typename wrap::returns_list;
						using args_list = typename wrap::free_args_list;
						using caller = typename wrap::caller;
						return stack::call_into_lua<checked, clean_stack>(
						     returns_list(), args_list(), L, boost + 1, caller(), std::forward<Fx>(f), std::forward<Args>(args)...);
					}
				}
			}
		};

		template <typename T, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<var_wrapper<T>, is_index, is_variable, checked, boost, clean_stack, C> {
			template <typename F>
			static int call(lua_State* L, F&& f) {
				if constexpr (is_index) {
					constexpr bool is_stack = is_stack_based_v<meta::unqualified_t<decltype(detail::unwrap(f.value()))>>;
					if constexpr (clean_stack && !is_stack) {
						lua_settop(L, 0);
					}
					return stack::push_reference(L, detail::unwrap(f.value()));
				}
				else {
					if constexpr (std::is_const_v<meta::unwrapped_t<T>>) {
						(void)f;
						return luaL_error(L, "sol: cannot write to a readonly (const) variable");
					}
					else {
						using R = meta::unwrapped_t<T>;
						if constexpr (std::is_assignable_v<std::add_lvalue_reference_t<meta::unqualified_t<R>>, R>) {
							detail::unwrap(f.value()) = stack::unqualified_get<meta::unwrapped_t<T>>(L, boost + (is_variable ? 3 : 1));
							if (clean_stack) {
								lua_settop(L, 0);
							}
							return 0;
						}
						else {
							return luaL_error(L, "sol: cannot write to this variable: copy assignment/constructor not available");
						}
					}
				}
			}
		};

		template <bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<lua_CFunction_ref, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, lua_CFunction_ref f) {
				return f(L);
			}
		};

		template <bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<lua_CFunction, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, lua_CFunction f) {
				return f(L);
			}
		};

#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
		template <bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<detail::lua_CFunction_noexcept, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, detail::lua_CFunction_noexcept f) {
				return f(L);
			}
		};
#endif // noexcept function types

		template <bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<detail::no_prop, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, const detail::no_prop&) {
				return luaL_error(L, is_index ? "sol: cannot read from a writeonly property" : "sol: cannot write to a readonly property");
			}
		};

		template <bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<no_construction, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, const no_construction&) {
				return function_detail::no_construction_error(L);
			}
		};

		template <typename... Args, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<bases<Args...>, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State*, const bases<Args...>&) {
				// Uh. How did you even call this, lul
				return 0;
			}
		};

		template <typename T, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct agnostic_lua_call_wrapper<std::reference_wrapper<T>, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, std::reference_wrapper<T> f) {
				agnostic_lua_call_wrapper<T, is_index, is_variable, checked, boost, clean_stack> alcw {};
				return alcw.call(L, f.get());
			}
		};

		template <typename T, typename F, bool is_index, bool is_variable, bool checked = detail::default_safe_function_calls, int boost = 0,
		     bool clean_stack = true, typename = void>
		struct lua_call_wrapper {
			template <typename Fx, typename... Args>
			static int call(lua_State* L, Fx&& fx, Args&&... args) {
				if constexpr (std::is_member_function_pointer_v<F>) {
					using wrap = wrapper<F>;
					using object_type = typename wrap::object_type;
					if constexpr (sizeof...(Args) < 1) {
						using Ta = meta::conditional_t<std::is_void_v<T>, object_type, T>;
						static_assert(std::is_base_of_v<object_type, Ta>,
						     "It seems like you might have accidentally bound a class type with a member function method that does not correspond to the "
						     "class. For example, there could be a small type in your new_usertype<T>(...) binding, where you specify one class \"T\" "
						     "but then bind member methods from a complete unrelated class. Check things over!");
#if SOL_IS_ON(SOL_SAFE_USERTYPE)
						auto maybeo = stack::check_get<Ta*>(L, 1);
						if (!maybeo || maybeo.value() == nullptr) {
							return luaL_error(L,
							     "sol: received nil for 'self' argument (use ':' for accessing member functions, make sure member variables are "
							     "preceeded by the "
							     "actual object with '.' syntax)");
						}
						object_type* o = static_cast<object_type*>(maybeo.value());
						return call(L, std::forward<Fx>(fx), *o);
#else
						object_type& o = static_cast<object_type&>(*stack::unqualified_get<non_null<Ta*>>(L, 1));
						return call(L, std::forward<Fx>(fx), o);
#endif // Safety
					}
					else {
						using returns_list = typename wrap::returns_list;
						using args_list = typename wrap::args_list;
						using caller = typename wrap::caller;
						return stack::call_into_lua<checked, clean_stack>(
						     returns_list(), args_list(), L, boost + (is_variable ? 3 : 2), caller(), std::forward<Fx>(fx), std::forward<Args>(args)...);
					}
				}
				else if constexpr (std::is_member_object_pointer_v<F>) {
					using wrap = wrapper<F>;
					using object_type = typename wrap::object_type;
					if constexpr (is_index) {
						if constexpr (sizeof...(Args) < 1) {
							using Ta = meta::conditional_t<std::is_void_v<T>, object_type, T>;
							static_assert(std::is_base_of_v<object_type, Ta>,
							     "It seems like you might have accidentally bound a class type with a member function method that does not correspond "
							     "to the class. For example, there could be a small type in your new_usertype<T>(...) binding, where you specify one "
							     "class \"T\" but then bind member methods from a complete unrelated class. Check things over!");
#if SOL_IS_ON(SOL_SAFE_USERTYPE)
							auto maybeo = stack::check_get<Ta*>(L, 1);
							if (!maybeo || maybeo.value() == nullptr) {
								if (is_variable) {
									return luaL_error(L, "sol: 'self' argument is lua_nil (bad '.' access?)");
								}
								return luaL_error(L, "sol: 'self' argument is lua_nil (pass 'self' as first argument)");
							}
							object_type* o = static_cast<object_type*>(maybeo.value());
							return call(L, std::forward<Fx>(fx), *o);
#else
							object_type& o = static_cast<object_type&>(*stack::get<non_null<Ta*>>(L, 1));
							return call(L, std::forward<Fx>(fx), o);
#endif // Safety
						}
						else {
							using returns_list = typename wrap::returns_list;
							using caller = typename wrap::caller;
							return stack::call_into_lua<checked, clean_stack>(returns_list(),
							     types<>(),
							     L,
							     boost + (is_variable ? 3 : 2),
							     caller(),
							     std::forward<Fx>(fx),
							     std::forward<Args>(args)...);
						}
					}
					else {
						using traits_type = lua_bind_traits<F>;
						using return_type = typename traits_type::return_type;
						constexpr bool ret_is_const = std::is_const_v<std::remove_reference_t<return_type>>;
						if constexpr (ret_is_const) {
							(void)fx;
							(void)detail::swallow { 0, (static_cast<void>(args), 0)... };
							return luaL_error(L, "sol: cannot write to a readonly (const) variable");
						}
						else {
							using u_return_type = meta::unqualified_t<return_type>;
							constexpr bool is_assignable = std::is_copy_assignable_v<u_return_type> || std::is_array_v<u_return_type>;
							if constexpr (!is_assignable) {
								(void)fx;
								(void)detail::swallow { 0, ((void)args, 0)... };
								return luaL_error(L, "sol: cannot write to this variable: copy assignment/constructor not available");
							}
							else {
								using args_list = typename wrap::args_list;
								using caller = typename wrap::caller;
								if constexpr (sizeof...(Args) > 0) {
									return stack::call_into_lua<checked, clean_stack>(types<void>(),
									     args_list(),
									     L,
									     boost + (is_variable ? 3 : 2),
									     caller(),
									     std::forward<Fx>(fx),
									     std::forward<Args>(args)...);
								}
								else {
									using Ta = meta::conditional_t<std::is_void_v<T>, object_type, T>;
#if SOL_IS_ON(SOL_SAFE_USERTYPE)
									auto maybeo = stack::check_get<Ta*>(L, 1);
									if (!maybeo || maybeo.value() == nullptr) {
										if (is_variable) {
											return luaL_error(L, "sol: received nil for 'self' argument (bad '.' access?)");
										}
										return luaL_error(L, "sol: received nil for 'self' argument (pass 'self' as first argument)");
									}
									object_type* po = static_cast<object_type*>(maybeo.value());
									object_type& o = *po;
#else
									object_type& o = static_cast<object_type&>(*stack::get<non_null<Ta*>>(L, 1));
#endif // Safety

									return stack::call_into_lua<checked, clean_stack>(
									     types<void>(), args_list(), L, boost + (is_variable ? 3 : 2), caller(), std::forward<Fx>(fx), o);
								}
							}
						}
					}
				}
				else {
					agnostic_lua_call_wrapper<F, is_index, is_variable, checked, boost, clean_stack> alcw {};
					return alcw.call(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
				}
			}
		};

		template <typename T, typename F, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, readonly_wrapper<F>, is_index, is_variable, checked, boost, clean_stack, C> {
			using traits_type = lua_bind_traits<F>;
			using wrap = wrapper<F>;
			using object_type = typename wrap::object_type;

			static int call(lua_State* L, readonly_wrapper<F>&& rw) {
				if constexpr (!is_index) {
					(void)rw;
					return luaL_error(L, "sol: cannot write to a sol::readonly variable");
				}
				else {
					lua_call_wrapper<T, F, true, is_variable, checked, boost, clean_stack, C> lcw;
					return lcw.call(L, std::move(rw.value()));
				}
			}

			static int call(lua_State* L, readonly_wrapper<F>&& rw, object_type& o) {
				if constexpr (!is_index) {
					(void)o;
					return call(L, std::move(rw));
				}
				else {
					lua_call_wrapper<T, F, true, is_variable, checked, boost, clean_stack, C> lcw;
					return lcw.call(L, rw.value(), o);
				}
			}

			static int call(lua_State* L, const readonly_wrapper<F>& rw) {
				if constexpr (!is_index) {
					(void)rw;
					return luaL_error(L, "sol: cannot write to a sol::readonly variable");
				}
				else {
					lua_call_wrapper<T, F, true, is_variable, checked, boost, clean_stack, C> lcw;
					return lcw.call(L, rw.value());
				}
			}

			static int call(lua_State* L, const readonly_wrapper<F>& rw, object_type& o) {
				if constexpr (!is_index) {
					(void)o;
					return call(L, rw);
				}
				else {
					lua_call_wrapper<T, F, true, is_variable, checked, boost, clean_stack, C> lcw;
					return lcw.call(L, rw.value(), o);
				}
			}
		};

		template <typename T, typename... Args, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, constructor_list<Args...>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef constructor_list<Args...> F;

			static int call(lua_State* L, F&) {
				const auto& meta = usertype_traits<T>::metatable();
				int argcount = lua_gettop(L);
				call_syntax syntax = argcount > 0 ? stack::get_call_syntax(L, usertype_traits<T>::user_metatable(), 1) : call_syntax::dot;
				argcount -= static_cast<int>(syntax);

				T* obj = detail::usertype_allocate<T>(L);
				reference userdataref(L, -1);
				stack::stack_detail::undefined_metatable umf(L, &meta[0], &stack::stack_detail::set_undefined_methods_on<T>);

				// put userdata at the first index
				lua_insert(L, 1);
				// Because of the way constructors work,
				// we have to kill the data, but only if the cosntructor is successfulyl invoked...
				// if it's not successfully invoked and we panic,
				// we cannot actually deallcoate/delete the data.
				construct_match<T, Args...>(
				     constructor_match<T, checked, clean_stack>(obj, userdataref, umf), L, argcount, boost + 1 + 1 + static_cast<int>(syntax));

				userdataref.push();
				return 1;
			}
		};

		template <typename T, typename... Cxs, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, constructor_wrapper<Cxs...>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef constructor_wrapper<Cxs...> F;

			struct onmatch {
				template <typename Fx, std::size_t I, typename... R, typename... Args>
				int operator()(types<Fx>, meta::index_value<I>, types<R...> r, types<Args...> a, lua_State* L, int, int start, F& f) {
					const auto& meta = usertype_traits<T>::metatable();
					T* obj = detail::usertype_allocate<T>(L);
					reference userdataref(L, -1);
					stack::stack_detail::undefined_metatable umf(L, &meta[0], &stack::stack_detail::set_undefined_methods_on<T>);
					umf();

					auto& func = std::get<I>(f.functions);
					// put userdata at the first index
					lua_insert(L, 1);
					stack::call_into_lua<checked, clean_stack>(r, a, L, boost + 1 + start, func, detail::implicit_wrapper<T>(obj));

					userdataref.push();
					return 1;
				}
			};

			static int call(lua_State* L, F& f) {
				call_syntax syntax = stack::get_call_syntax(L, usertype_traits<T>::user_metatable(), 1);
				int syntaxval = static_cast<int>(syntax);
				int argcount = lua_gettop(L) - syntaxval;
				return construct_match<T, meta::pop_front_type_t<meta::function_args_t<Cxs>>...>(onmatch(), L, argcount, 1 + syntaxval, f);
			}
		};

		template <typename T, typename Fx, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, destructor_wrapper<Fx>, is_index, is_variable, checked, boost, clean_stack, C> {

			template <typename F>
			static int call(lua_State* L, F&& f) {
				if constexpr (std::is_void_v<Fx>) {
					return detail::usertype_alloc_destroy<T>(L);
				}
				else {
					using uFx = meta::unqualified_t<Fx>;
					lua_call_wrapper<T, uFx, is_index, is_variable, checked, boost, clean_stack> lcw {};
					return lcw.call(L, std::forward<F>(f).fx);
				}
			}
		};

		template <typename T, typename... Fs, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, overload_set<Fs...>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef overload_set<Fs...> F;

			struct on_match {
				template <typename Fx, std::size_t I, typename... R, typename... Args>
				int operator()(types<Fx>, meta::index_value<I>, types<R...>, types<Args...>, lua_State* L, int, int, F& fx) {
					auto& f = std::get<I>(fx.functions);
					return lua_call_wrapper<T, Fx, is_index, is_variable, checked, boost> {}.call(L, f);
				}
			};

			static int call(lua_State* L, F& fx) {
				return overload_match_arity<Fs...>(on_match(), L, lua_gettop(L), 1, fx);
			}
		};

		template <typename T, typename... Fs, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, factory_wrapper<Fs...>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef factory_wrapper<Fs...> F;

			struct on_match {
				template <typename Fx, std::size_t I, typename... R, typename... Args>
				int operator()(types<Fx>, meta::index_value<I>, types<R...>, types<Args...>, lua_State* L, int, int, F& fx) {
					auto& f = std::get<I>(fx.functions);
					return lua_call_wrapper<T, Fx, is_index, is_variable, checked, boost, clean_stack> {}.call(L, f);
				}
			};

			static int call(lua_State* L, F& fx) {
				return overload_match_arity<Fs...>(on_match(), L, lua_gettop(L) - boost, 1 + boost, fx);
			}
		};

		template <typename T, typename R, typename W, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, property_wrapper<R, W>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef meta::conditional_t<is_index, R, W> P;
			typedef meta::unqualified_t<P> U;
			typedef wrapper<U> wrap;
			typedef lua_bind_traits<U> traits_type;
			typedef meta::unqualified_t<typename traits_type::template arg_at<0>> object_type;

			template <typename F, typename... Args>
			static int call(lua_State* L, F&& f, Args&&... args) {
				constexpr bool is_specialized = meta::any<std::is_same<U, detail::no_prop>,
				     meta::is_specialization_of<U, var_wrapper>,
				     meta::is_specialization_of<U, constructor_wrapper>,
				     meta::is_specialization_of<U, constructor_list>,
				     std::is_member_pointer<U>>::value;
				if constexpr (is_specialized) {
					if constexpr (is_index) {
						decltype(auto) p = f.read();
						lua_call_wrapper<T, meta::unqualified_t<decltype(p)>, is_index, is_variable, checked, boost, clean_stack> lcw {};
						return lcw.call(L, p, std::forward<Args>(args)...);
					}
					else {
						decltype(auto) p = f.write();
						lua_call_wrapper<T, meta::unqualified_t<decltype(p)>, is_index, is_variable, checked, boost, clean_stack> lcw {};
						return lcw.call(L, p, std::forward<Args>(args)...);
					}
				}
				else {
					constexpr bool non_class_object_type = meta::any<std::is_void<object_type>,
					     meta::boolean<lua_type_of<meta::unwrap_unqualified_t<object_type>>::value != type::userdata>>::value;
					if constexpr (non_class_object_type) {
						// The type being void means we don't have any arguments, so it might be a free functions?
						using args_list = typename traits_type::free_args_list;
						using returns_list = typename wrap::returns_list;
						using caller = typename wrap::caller;
						if constexpr (is_index) {
							decltype(auto) pf = f.read();
							return stack::call_into_lua<checked, clean_stack>(
							     returns_list(), args_list(), L, boost + (is_variable ? 3 : 2), caller(), pf);
						}
						else {
							decltype(auto) pf = f.write();
							return stack::call_into_lua<checked, clean_stack>(
							     returns_list(), args_list(), L, boost + (is_variable ? 3 : 2), caller(), pf);
						}
					}
					else {
						using args_list = meta::pop_front_type_t<typename traits_type::free_args_list>;
						using Ta = T;
						using Oa = std::remove_pointer_t<object_type>;
#if SOL_IS_ON(SOL_SAFE_USERTYPE)
						auto maybeo = stack::check_get<Ta*>(L, 1);
						if (!maybeo || maybeo.value() == nullptr) {
							if (is_variable) {
								return luaL_error(L, "sol: 'self' argument is lua_nil (bad '.' access?)");
							}
							return luaL_error(L, "sol: 'self' argument is lua_nil (pass 'self' as first argument)");
						}
						Oa* o = static_cast<Oa*>(maybeo.value());
#else
						Oa* o = static_cast<Oa*>(stack::get<non_null<Ta*>>(L, 1));
#endif // Safety
						using returns_list = typename wrap::returns_list;
						using caller = typename wrap::caller;
						if constexpr (is_index) {
							decltype(auto) pf = f.read();
							return stack::call_into_lua<checked, clean_stack>(
							     returns_list(), args_list(), L, boost + (is_variable ? 3 : 2), caller(), pf, detail::implicit_wrapper<Oa>(*o));
						}
						else {
							decltype(auto) pf = f.write();
							return stack::call_into_lua<checked, clean_stack>(
							     returns_list(), args_list(), L, boost + (is_variable ? 3 : 2), caller(), pf, detail::implicit_wrapper<Oa>(*o));
						}
					}
				}
			}
		};

		template <typename T, typename V, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, protect_t<V>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef protect_t<V> F;

			template <typename... Args>
			static int call(lua_State* L, F& fx, Args&&... args) {
				return lua_call_wrapper<T, V, is_index, is_variable, true, boost, clean_stack> {}.call(L, fx.value, std::forward<Args>(args)...);
			}
		};

		template <typename T, typename F, typename... Policies, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, policy_wrapper<F, Policies...>, is_index, is_variable, checked, boost, clean_stack, C> {
			typedef policy_wrapper<F, Policies...> P;

			template <std::size_t... In>
			static int call(std::index_sequence<In...>, lua_State* L, P& fx) {
				int pushed = lua_call_wrapper<T, F, is_index, is_variable, checked, boost, false, C> {}.call(L, fx.value);
				(void)detail::swallow { int(), (policy_detail::handle_policy(std::get<In>(fx.policies), L, pushed), int())... };
				return pushed;
			}

			static int call(lua_State* L, P& fx) {
				typedef typename P::indices indices;
				return call(indices(), L, fx);
			}
		};

		template <typename T, typename Y, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, yielding_t<Y>, is_index, is_variable, checked, boost, clean_stack, C> {
			template <typename F>
			static int call(lua_State* L, F&& f) {
				return lua_call_wrapper<T, meta::unqualified_t<Y>, is_index, is_variable, checked, boost, clean_stack> {}.call(L, f.func);
			}
		};

		template <typename T, typename Sig, typename P, bool is_index, bool is_variable, bool checked, int boost, bool clean_stack, typename C>
		struct lua_call_wrapper<T, function_arguments<Sig, P>, is_index, is_variable, checked, boost, clean_stack, C> {
			static int call(lua_State* L, const function_arguments<Sig, P>& f) {
				lua_call_wrapper<T, meta::unqualified_t<P>, is_index, is_variable, checked, boost, clean_stack> lcw {};
				return lcw.call(L, std::get<0>(f.arguments));
			}

			static int call(lua_State* L, function_arguments<Sig, P>& f) {
				lua_call_wrapper<T, meta::unqualified_t<P>, is_index, is_variable, checked, boost, clean_stack> lcw {};
				return lcw.call(L, std::get<0>(f.arguments));
			}

			static int call(lua_State* L, function_arguments<Sig, P>&& f) {
				lua_call_wrapper<T, meta::unqualified_t<P>, is_index, is_variable, checked, boost, clean_stack> lcw {};
				return lcw.call(L, std::get<0>(std::move(f.arguments)));
			}
		};

		template <typename T, bool is_index, bool is_variable, int boost = 0, bool checked = detail::default_safe_function_calls, bool clean_stack = true,
		     typename Fx, typename... Args>
		inline int call_wrapped(lua_State* L, Fx&& fx, Args&&... args) {
			using uFx = meta::unqualified_t<Fx>;
			if constexpr (meta::is_specialization_of_v<uFx, yielding_t>) {
				using real_fx = meta::unqualified_t<decltype(std::forward<Fx>(fx).func)>;
				lua_call_wrapper<T, real_fx, is_index, is_variable, checked, boost, clean_stack> lcw {};
				return lcw.call(L, std::forward<Fx>(fx).func, std::forward<Args>(args)...);
			}
			else {
				lua_call_wrapper<T, uFx, is_index, is_variable, checked, boost, clean_stack> lcw {};
				return lcw.call(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}
		}

		template <typename T, bool is_index, bool is_variable, typename F, int start = 1, bool checked = detail::default_safe_function_calls,
		     bool clean_stack = true>
		inline int call_user(lua_State* L) {
			auto& fx = stack::unqualified_get<user<F>>(L, upvalue_index(start));
			using uFx = meta::unqualified_t<F>;
			int nr = call_wrapped<T, is_index, is_variable, 0, checked, clean_stack>(L, fx);
			if constexpr (meta::is_specialization_of_v<uFx, yielding_t>) {
				return lua_yield(L, nr);
			}
			else {
				return nr;
			}
		}

		template <typename T, typename = void>
		struct is_var_bind : std::false_type { };

		template <typename T>
		struct is_var_bind<T, std::enable_if_t<std::is_member_object_pointer<T>::value>> : std::true_type { };

		template <typename T>
		struct is_var_bind<T, std::enable_if_t<is_lua_reference_or_proxy<T>::value>> : std::true_type { };

		template <>
		struct is_var_bind<detail::no_prop> : std::true_type { };

		template <typename R, typename W>
		struct is_var_bind<property_wrapper<R, W>> : std::true_type { };

		template <typename T>
		struct is_var_bind<var_wrapper<T>> : std::true_type { };

		template <typename T>
		struct is_var_bind<readonly_wrapper<T>> : is_var_bind<meta::unqualified_t<T>> { };

		template <typename F, typename... Policies>
		struct is_var_bind<policy_wrapper<F, Policies...>> : is_var_bind<meta::unqualified_t<F>> { };
	} // namespace call_detail

	template <typename T>
	struct is_variable_binding : call_detail::is_var_bind<meta::unqualified_t<T>> { };

	template <typename T>
	using is_var_wrapper = meta::is_specialization_of<T, var_wrapper>;

	template <typename T>
	struct is_function_binding : meta::neg<is_variable_binding<T>> { };

} // namespace sol

#endif // SOL_CALL_HPP
