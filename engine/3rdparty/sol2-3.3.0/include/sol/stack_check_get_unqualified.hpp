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

#ifndef SOL_STACK_CHECK_UNQUALIFIED_GET_HPP
#define SOL_STACK_CHECK_UNQUALIFIED_GET_HPP

#include <sol/stack_core.hpp>
#include <sol/stack_get.hpp>
#include <sol/stack_check.hpp>
#include <sol/optional.hpp>

#include <cstdlib>
#include <cmath>
#include <optional>
#if SOL_IS_ON(SOL_STD_VARIANT)
#include <variant>
#endif // variant shenanigans (thanks, Mac OSX)


namespace sol { namespace stack {
	template <typename T, typename>
	struct unqualified_check_getter {
		typedef decltype(stack_detail::unchecked_unqualified_get<T>(nullptr, -1, std::declval<record&>())) R;

		template <typename Optional, typename Handler>
		static Optional get_using(lua_State* L, int index, Handler&& handler, record& tracking) {
			if constexpr (!meta::meta_detail::is_adl_sol_lua_check_v<T> && !meta::meta_detail::is_adl_sol_lua_get_v<T>) {
				if constexpr (is_lua_reference_v<T>) {
					if constexpr (is_global_table_v<T>) {
						(void)L;
						(void)index;
						(void)handler;
						tracking.use(1);
						return true;
					}
					else {
						// actually check if it's none here, otherwise
						// we'll have a none object inside an optional!
						bool success = lua_isnoneornil(L, index) == 0 && stack::check<T>(L, index, &no_panic);
						if (!success) {
							// expected type, actual type
							tracking.use(static_cast<int>(success));
							handler(L, index, type::poly, type_of(L, index), "");
							return detail::associated_nullopt_v<Optional>;
						}
						return stack_detail::unchecked_get<T>(L, index, tracking);
					}
				}
				else if constexpr ((std::is_integral_v<T> || std::is_same_v<T, lua_Integer>)&&!std::is_same_v<T, bool>) {
#if SOL_LUA_VERSION_I_ >= 503
					if (lua_isinteger(L, index) != 0) {
						tracking.use(1);
						return static_cast<T>(lua_tointeger(L, index));
					}
#endif
					int isnum = 0;
					const lua_Number value = lua_tonumberx(L, index, &isnum);
					if (isnum != 0) {
#if SOL_IS_ON(SOL_NUMBER_PRECISION_CHECKS)
						const auto integer_value = llround(value);
						if (static_cast<lua_Number>(integer_value) == value) {
							tracking.use(1);
							return static_cast<T>(integer_value);
						}
#else
						tracking.use(1);
						return static_cast<T>(value);
#endif
					}
					const type t = type_of(L, index);
					tracking.use(static_cast<int>(t != type::none));
					handler(L, index, type::number, t, "not an integer");
					return detail::associated_nullopt_v<Optional>;
				}
				else if constexpr (std::is_floating_point_v<T> || std::is_same_v<T, lua_Number>) {
					int isnum = 0;
					lua_Number value = lua_tonumberx(L, index, &isnum);
					if (isnum == 0) {
						type t = type_of(L, index);
						tracking.use(static_cast<int>(t != type::none));
						handler(L, index, type::number, t, "not a valid floating point number");
						return detail::associated_nullopt_v<Optional>;
					}
					tracking.use(1);
					return static_cast<T>(value);
				}
				else if constexpr (std::is_enum_v<T> && !meta::any_same_v<T, meta_function, type>) {
					int isnum = 0;
					lua_Integer value = lua_tointegerx(L, index, &isnum);
					if (isnum == 0) {
						type t = type_of(L, index);
						tracking.use(static_cast<int>(t != type::none));
						handler(L, index, type::number, t, "not a valid enumeration value");
						return detail::associated_nullopt_v<Optional>;
					}
					tracking.use(1);
					return static_cast<T>(value);
				}
				else {
					if (!unqualified_check<T>(L, index, std::forward<Handler>(handler))) {
						tracking.use(static_cast<int>(!lua_isnone(L, index)));
						return detail::associated_nullopt_v<Optional>;
					}
					return stack_detail::unchecked_unqualified_get<T>(L, index, tracking);
				}
			}
			else {
				if (!unqualified_check<T>(L, index, std::forward<Handler>(handler))) {
					tracking.use(static_cast<int>(!lua_isnone(L, index)));
					return detail::associated_nullopt_v<Optional>;
				}
				return stack_detail::unchecked_unqualified_get<T>(L, index, tracking);
			}
		}

		template <typename Handler>
		static optional<R> get(lua_State* L, int index, Handler&& handler, record& tracking) {
			return get_using<optional<R>>(L, index, std::forward<Handler>(handler), tracking);
		}
	};

#if SOL_IS_ON(SOL_STD_VARIANT)
	template <typename... Tn, typename C>
	struct unqualified_check_getter<std::variant<Tn...>, C> {
		typedef std::variant<Tn...> V;
		typedef std::variant_size<V> V_size;
		typedef std::integral_constant<bool, V_size::value == 0> V_is_empty;

		template <typename Handler>
		static optional<V> get_empty(std::true_type, lua_State*, int, Handler&&, record&) {
			return nullopt;
		}

		template <typename Handler>
		static optional<V> get_empty(std::false_type, lua_State* L, int index, Handler&& handler, record&) {
			// This should never be reached...
			// please check your code and understand what you did to bring yourself here
			// maybe file a bug report, or 5
			handler(
				L, index, type::poly, type_of(L, index), "this variant code should never be reached: if it has, you have done something so terribly wrong");
			return nullopt;
		}

		template <typename Handler>
		static optional<V> get_one(std::integral_constant<std::size_t, 0>, lua_State* L, int index, Handler&& handler, record& tracking) {
			return get_empty(V_is_empty(), L, index, std::forward<Handler>(handler), tracking);
		}

		template <std::size_t I, typename Handler>
		static optional<V> get_one(std::integral_constant<std::size_t, I>, lua_State* L, int index, Handler&& handler, record& tracking) {
			typedef std::variant_alternative_t<I - 1, V> T;
			if (stack::check<T>(L, index, &no_panic, tracking)) {
				return V(std::in_place_index<I - 1>, stack::get<T>(L, index));
			}
			return get_one(std::integral_constant<std::size_t, I - 1>(), L, index, std::forward<Handler>(handler), tracking);
		}

		template <typename Handler>
		static optional<V> get(lua_State* L, int index, Handler&& handler, record& tracking) {
			return get_one(std::integral_constant<std::size_t, V_size::value>(), L, index, std::forward<Handler>(handler), tracking);
		}
	};
#endif // standard variant
}}     // namespace sol::stack

#endif // SOL_STACK_CHECK_UNQUALIFIED_GET_HPP
