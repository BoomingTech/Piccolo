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

#ifndef SOL_STACK_PUSH_HPP
#define SOL_STACK_PUSH_HPP

#include <sol/stack_core.hpp>
#include <sol/raii.hpp>
#include <sol/optional.hpp>
#include <sol/usertype_traits.hpp>
#include <sol/policies.hpp>
#include <sol/unicode.hpp>
#include <sol/assert.hpp>

#include <memory>
#include <type_traits>
#include <cassert>
#include <limits>
#include <cmath>
#include <string_view>
#if SOL_IS_ON(SOL_STD_VARIANT)
#include <variant>
#endif // Can use variant

#include <sol/debug.hpp>

namespace sol { namespace stack {
	namespace stack_detail {
		template <typename T>
		inline bool integer_value_fits(const T& value) {
			// We check if we can rely on casts or a lack of padding bits to satisfy
			// the requirements here
			// If it lacks padding bits, we can jump back and forth between lua_Integer and whatever type without
			// loss of information
			constexpr bool is_same_signedness
				= (std::is_signed_v<T> && std::is_signed_v<lua_Integer>) || (std::is_unsigned_v<T> && std::is_unsigned_v<lua_Integer>);
			constexpr bool probaby_fits_within_lua_Integer = sizeof(T) == sizeof(lua_Integer)
#if SOL_IS_ON(SOL_ALL_INTEGER_VALUES_FIT)
				&& ((std::has_unique_object_representations_v<T> && std::has_unique_object_representations_v<lua_Integer>) ? true : is_same_signedness)
#else
				&& is_same_signedness
#endif
				;
			if constexpr (sizeof(T) < sizeof(lua_Integer) || probaby_fits_within_lua_Integer) {
				(void)value;
				return true;
			}
			else {
				auto u_min = static_cast<std::intmax_t>((std::numeric_limits<lua_Integer>::min)());
				auto u_max = static_cast<std::uintmax_t>((std::numeric_limits<lua_Integer>::max)());
				auto t_min = static_cast<std::intmax_t>((std::numeric_limits<T>::min)());
				auto t_max = static_cast<std::uintmax_t>((std::numeric_limits<T>::max)());
				return (u_min <= t_min || value >= static_cast<T>(u_min)) && (u_max >= t_max || value <= static_cast<T>(u_max));
			}
		}

		template <typename T>
		int msvc_is_ass_with_if_constexpr_push_enum(std::true_type, lua_State* L, const T& value) {
			if constexpr (meta::any_same_v<std::underlying_type_t<T>,
				              char
#if SOL_IS_ON(SOL_CHAR8_T)
				              ,
				              char8_t
#endif
				              ,
				              char16_t,
				              char32_t>) {
				if constexpr (std::is_signed_v<T>) {
					return stack::push(L, static_cast<std::int_least32_t>(value));
				}
				else {
					return stack::push(L, static_cast<std::uint_least32_t>(value));
				}
			}
			else {
				return stack::push(L, static_cast<std::underlying_type_t<T>>(value));
			}
		}

		template <typename T>
		int msvc_is_ass_with_if_constexpr_push_enum(std::false_type, lua_State*, const T&) {
			return 0;
		}
	} // namespace stack_detail

	inline int push_environment_of(lua_State* L, int target_index = -1) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
		luaL_checkstack(L, 1, detail::not_enough_stack_space_environment);
#endif // make sure stack doesn't overflow
#if SOL_LUA_VERSION_I_ < 502
		// Use lua_getfenv
		lua_getfenv(L, target_index);
#else

		if (lua_iscfunction(L, target_index) != 0) {
			const char* maybe_upvalue_name = lua_getupvalue(L, target_index, 1);
			if (maybe_upvalue_name != nullptr) {
				// it worked, take this one
				return 1;
			}
		}
		// Nominally, we search for the `"_ENV"` value.
		// If we don't find it.... uh, well. We've got a problem?
		for (int upvalue_index = 1;; ++upvalue_index) {
			const char* maybe_upvalue_name = lua_getupvalue(L, target_index, upvalue_index);
			if (maybe_upvalue_name == nullptr) {
				push(L, lua_nil);
				break;
			}

			string_view upvalue_name(maybe_upvalue_name);
			if (upvalue_name == "_ENV") {
				// Keep this one!
				break;
			}
			// Discard what we received, loop back around
			lua_pop(L, 1);
		}
#endif
		return 1;
	}

	template <typename T>
	int push_environment_of(const T& target) {
		lua_State* target_L = target.lua_state();
		int target_index = absolute_index(target_L, -target.push());
		int env_count = push_environment_of(target_L, target_index);
		sol_c_assert(env_count == 1);
		lua_rotate(target_L, target_index, 1);
		lua_pop(target_L, 1);
		return env_count;
	}

	template <typename T>
	struct unqualified_pusher<detail::as_value_tag<T>> {
		template <typename F, typename... Args>
		static int push_fx(lua_State* L, F&& f, Args&&... args) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_userdata);
#endif // make sure stack doesn't overflow
       // Basically, we store all user-data like this:
       // If it's a movable/copyable value (no std::ref(x)), then we store the pointer to the new
       // data in the first sizeof(T*) bytes, and then however many bytes it takes to
       // do the actual object. Things that are std::ref or plain T* are stored as
       // just the sizeof(T*), and nothing else.
			T* obj = detail::usertype_allocate<T>(L);
			f();
			std::allocator<T> alloc {};
			std::allocator_traits<std::allocator<T>>::construct(alloc, obj, std::forward<Args>(args)...);
			return 1;
		}

		template <typename K, typename... Args>
		static int push_keyed(lua_State* L, K&& k, Args&&... args) {
			stack_detail::undefined_metatable fx(L, &k[0], &stack::stack_detail::set_undefined_methods_on<T>);
			return push_fx(L, fx, std::forward<Args>(args)...);
		}

		template <typename Arg, typename... Args>
		static int push(lua_State* L, Arg&& arg, Args&&... args) {
			if constexpr (std::is_same_v<meta::unqualified_t<Arg>, detail::with_function_tag>) {
				(void)arg;
				return push_fx(L, std::forward<Args>(args)...);
			}
			else {
				return push_keyed(L, usertype_traits<T>::metatable(), std::forward<Arg>(arg), std::forward<Args>(args)...);
			}
		}

		static int push(lua_State* L) {
			return push_keyed(L, usertype_traits<T>::metatable());
		}
	};

	template <typename T>
	struct unqualified_pusher<detail::as_pointer_tag<T>> {
		typedef meta::unqualified_t<T> U;

		template <typename F>
		static int push_fx(lua_State* L, F&& f, T* obj) {
			if (obj == nullptr)
				return stack::push(L, lua_nil);
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_userdata);
#endif // make sure stack doesn't overflow
			T** pref = detail::usertype_allocate_pointer<T>(L);
			f();
			*pref = obj;
			return 1;
		}

		template <typename K>
		static int push_keyed(lua_State* L, K&& k, T* obj) {
			stack_detail::undefined_metatable fx(L, &k[0], &stack::stack_detail::set_undefined_methods_on<U*>);
			return push_fx(L, fx, obj);
		}

		template <typename Arg, typename... Args>
		static int push(lua_State* L, Arg&& arg, Args&&... args) {
			if constexpr (std::is_same_v<meta::unqualified_t<Arg>, detail::with_function_tag>) {
				(void)arg;
				return push_fx(L, std::forward<Args>(args)...);
			}
			else {
				return push_keyed(L, usertype_traits<U*>::metatable(), std::forward<Arg>(arg), std::forward<Args>(args)...);
			}
		}
	};

	template <>
	struct unqualified_pusher<detail::as_reference_tag> {
		template <typename T>
		static int push(lua_State* L, T&& obj) {
			return stack::push(L, detail::ptr(obj));
		}
	};

	namespace stack_detail {
		template <typename T>
		struct uu_pusher {
			using element = unique_usertype_element_t<T>;
			using actual = unique_usertype_actual_t<T>;

			template <typename Arg, typename... Args>
			static int push(lua_State* L, Arg&& arg, Args&&... args) {
				if constexpr (std::is_base_of_v<actual, meta::unqualified_t<Arg>>) {
					if (detail::unique_is_null(L, arg)) {
						return stack::push(L, lua_nil);
					}
					return push_deep(L, std::forward<Arg>(arg), std::forward<Args>(args)...);
				}
				else {
					return push_deep(L, std::forward<Arg>(arg), std::forward<Args>(args)...);
				}
			}

			template <typename... Args>
			static int push_deep(lua_State* L, Args&&... args) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_userdata);
#endif // make sure stack doesn't overflow
				element** pointer_to_memory = nullptr;
				detail::unique_destructor* fx = nullptr;
				detail::unique_tag* id = nullptr;
				actual* typed_memory = detail::usertype_unique_allocate<element, actual>(L, pointer_to_memory, fx, id);
				if (luaL_newmetatable(L, &usertype_traits<d::u<std::remove_cv_t<element>>>::metatable()[0]) == 1) {
					detail::lua_reg_table registration_table {};
					int index = 0;
					detail::indexed_insert insert_callable(registration_table, index);
					detail::insert_default_registrations<element>(insert_callable, detail::property_always_true);
					registration_table[index] = { to_string(meta_function::garbage_collect).c_str(), detail::make_destructor<T>() };
					luaL_setfuncs(L, registration_table, 0);
				}
				lua_setmetatable(L, -2);
				*fx = detail::usertype_unique_alloc_destroy<element, actual>;
				*id = &detail::inheritance<element>::template type_unique_cast<actual>;
				detail::default_construct::construct(typed_memory, std::forward<Args>(args)...);
				*pointer_to_memory = detail::unique_get<T>(L, *typed_memory);
				return 1;
			}
		};
	} // namespace stack_detail

	template <typename T>
	struct unqualified_pusher<detail::as_unique_tag<T>> {
		template <typename... Args>
		static int push(lua_State* L, Args&&... args) {
			stack_detail::uu_pusher<T> p;
			(void)p;
			return p.push(L, std::forward<Args>(args)...);
		}
	};

	template <typename T, typename>
	struct unqualified_pusher {
		template <typename... Args>
		static int push(lua_State* L, Args&&... args) {
			using Tu = meta::unqualified_t<T>;
			if constexpr (is_lua_reference_v<Tu>) {
				using int_arr = int[];
				int_arr p { (std::forward<Args>(args).push(L))... };
				return p[0];
			}
			else if constexpr (std::is_same_v<Tu, bool>) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
				lua_pushboolean(L, std::forward<Args>(args)...);
				return 1;
			}
			else if constexpr (std::is_integral_v<Tu> || std::is_same_v<Tu, lua_Integer>) {
				const Tu& value(std::forward<Args>(args)...);
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_integral);
#endif // make sure stack doesn't overflow
#if SOL_LUA_VERSION_I_ >= 503
				if (stack_detail::integer_value_fits<Tu>(value)) {
					lua_pushinteger(L, static_cast<lua_Integer>(value));
					return 1;
				}
#endif // Lua 5.3 and above
#if SOL_IS_ON(SOL_NUMBER_PRECISION_CHECKS)
				if (static_cast<T>(llround(static_cast<lua_Number>(value))) != value) {
#if SOL_IS_OFF(SOL_EXCEPTIONS)
					// Is this really worth it?
					sol_m_assert(false, "integer value will be misrepresented in lua");
					lua_pushinteger(L, static_cast<lua_Integer>(value));
					return 1;
#else
					throw error(detail::direct_error, "integer value will be misrepresented in lua");
#endif // No Exceptions
				}
#endif // Safe Numerics and Number Precision Check
				lua_pushnumber(L, static_cast<lua_Number>(value));
				return 1;
			}
			else if constexpr (std::is_floating_point_v<Tu> || std::is_same_v<Tu, lua_Number>) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_floating);
#endif // make sure stack doesn't overflow
				lua_pushnumber(L, std::forward<Args>(args)...);
				return 1;
			}
			else if constexpr (std::is_same_v<Tu, luaL_Stream*>) {
				luaL_Stream* source { std::forward<Args>(args)... };
				luaL_Stream* stream = static_cast<luaL_Stream*>(detail::alloc_newuserdata(L, sizeof(luaL_Stream)));
				stream->f = source->f;
#if SOL_IS_ON(SOL_LUAL_STREAM_USE_CLOSE_FUNCTION)
				stream->closef = source->closef;
#endif // LuaJIT and Lua 5.1 and below do not have
				return 1;
			}
			else if constexpr (std::is_same_v<Tu, luaL_Stream>) {
				luaL_Stream& source(std::forward<Args>(args)...);
				luaL_Stream* stream = static_cast<luaL_Stream*>(detail::alloc_newuserdata(L, sizeof(luaL_Stream)));
				stream->f = source.f;
#if SOL_IS_ON(SOL_LUAL_STREAM_USE_CLOSE_FUNCTION)
				stream->closef = source.closef;
#endif // LuaJIT and Lua 5.1 and below do not have
				return 1;
			}
			else if constexpr (std::is_enum_v<Tu>) {
				return stack_detail::msvc_is_ass_with_if_constexpr_push_enum(std::true_type(), L, std::forward<Args>(args)...);
			}
			else if constexpr (std::is_pointer_v<Tu>) {
				return stack::push<detail::as_pointer_tag<std::remove_pointer_t<T>>>(L, std::forward<Args>(args)...);
			}
			else if constexpr (is_unique_usertype_v<Tu>) {
				return stack::push<detail::as_unique_tag<T>>(L, std::forward<Args>(args)...);
			}
			else {
				return stack::push<detail::as_value_tag<T>>(L, std::forward<Args>(args)...);
			}
		}
	};

	template <typename T>
	struct unqualified_pusher<std::reference_wrapper<T>> {
		static int push(lua_State* L, const std::reference_wrapper<T>& t) {
			return stack::push(L, std::addressof(detail::deref(t.get())));
		}
	};

	template <typename T>
	struct unqualified_pusher<detail::as_table_tag<T>> {
		using has_kvp = meta::has_key_value_pair<meta::unqualified_t<std::remove_pointer_t<T>>>;

		static int push(lua_State* L, const T& tablecont) {
			return push(has_kvp(), std::false_type(), L, tablecont);
		}

		static int push(lua_State* L, const T& tablecont, nested_tag_t) {
			return push(has_kvp(), std::true_type(), L, tablecont);
		}

		static int push(std::true_type, lua_State* L, const T& tablecont) {
			return push(has_kvp(), std::true_type(), L, tablecont);
		}

		static int push(std::false_type, lua_State* L, const T& tablecont) {
			return push(has_kvp(), std::false_type(), L, tablecont);
		}

		template <bool is_nested>
		static int push(std::true_type, std::integral_constant<bool, is_nested>, lua_State* L, const T& tablecont) {
			auto& cont = detail::deref(detail::unwrap(tablecont));
			lua_createtable(L, static_cast<int>(cont.size()), 0);
			int tableindex = lua_gettop(L);
			for (const auto& pair : cont) {
				if (is_nested) {
					set_field(L, pair.first, as_nested_ref(pair.second), tableindex);
				}
				else {
					set_field(L, pair.first, pair.second, tableindex);
				}
			}
			return 1;
		}

		template <bool is_nested>
		static int push(std::false_type, std::integral_constant<bool, is_nested>, lua_State* L, const T& tablecont) {
			auto& cont = detail::deref(detail::unwrap(tablecont));
			lua_createtable(L, stack_detail::get_size_hint(cont), 0);
			int tableindex = lua_gettop(L);
			std::size_t index = 1;
			for (const auto& i : cont) {
#if SOL_LUA_VERSION_I_ >= 503
				int p = is_nested ? stack::push(L, as_nested_ref(i)) : stack::push(L, i);
				for (int pi = 0; pi < p; ++pi) {
					lua_seti(L, tableindex, static_cast<lua_Integer>(index++));
				}
#else
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
				lua_pushinteger(L, static_cast<lua_Integer>(index));
				int p = is_nested ? stack::push(L, as_nested_ref(i)) : stack::push(L, i);
				if (p == 1) {
					++index;
					lua_settable(L, tableindex);
				}
				else {
					int firstindex = tableindex + 1 + 1;
					for (int pi = 0; pi < p; ++pi) {
						stack::push(L, index);
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
						luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
						lua_pushvalue(L, firstindex);
						lua_settable(L, tableindex);
						++index;
						++firstindex;
					}
					lua_pop(L, 1 + p);
				}
#endif // Lua Version 5.3 and others
			}
			// TODO: figure out a better way to do this...?
			// set_field(L, -1, cont.size());
			return 1;
		}
	};

	template <typename T>
	struct unqualified_pusher<as_table_t<T>> {
		static int push(lua_State* L, const as_table_t<T>& value_) {
			using inner_t = std::remove_pointer_t<meta::unwrap_unqualified_t<T>>;
			if constexpr (is_container_v<inner_t>) {
				return stack::push<detail::as_table_tag<T>>(L, value_.value());
			}
			else {
				return stack::push(L, value_.value());
			}
		}

		static int push(lua_State* L, const T& value_) {
			using inner_t = std::remove_pointer_t<meta::unwrap_unqualified_t<T>>;
			if constexpr (is_container_v<inner_t>) {
				return stack::push<detail::as_table_tag<T>>(L, value_);
			}
			else {
				return stack::push(L, value_);
			}
		}
	};

	template <typename T>
	struct unqualified_pusher<nested<T>> {
		static int push(lua_State* L, const T& nested_value) noexcept {
			using Tu = meta::unwrap_unqualified_t<T>;
			using inner_t = std::remove_pointer_t<Tu>;
			if constexpr (is_container_v<inner_t>) {
				return stack::push<detail::as_table_tag<T>>(L, nested_value, nested_tag);
			}
			else {
				return stack::push<Tu>(L, nested_value);
			}
		}

		static int push(lua_State* L, const nested<T>& nested_wrapper_) noexcept {
			using Tu = meta::unwrap_unqualified_t<T>;
			using inner_t = std::remove_pointer_t<Tu>;
			if constexpr (is_container_v<inner_t>) {
				return stack::push<detail::as_table_tag<T>>(L, nested_wrapper_.value(), nested_tag);
			}
			else {
				return stack::push<Tu>(L, nested_wrapper_.value());
			}
		}
	};

	template <typename T>
	struct unqualified_pusher<std::initializer_list<T>> {
		static int push(lua_State* L, const std::initializer_list<T>& il) noexcept {
			unqualified_pusher<detail::as_table_tag<std::initializer_list<T>>> p {};
			return p.push(L, il);
		}
	};

	template <>
	struct unqualified_pusher<lua_nil_t> {
		static int push(lua_State* L, lua_nil_t) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushnil(L);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<stack_count> {
		static int push(lua_State*, stack_count st) noexcept {
			return st.count;
		}
	};

	template <>
	struct unqualified_pusher<metatable_key_t> {
		static int push(lua_State* L, metatable_key_t) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushlstring(L, to_string(meta_function::metatable).c_str(), 4);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<std::remove_pointer_t<lua_CFunction>> {
		static int push(lua_State* L, lua_CFunction func, int n = 0) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushcclosure(L, func, n);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<lua_CFunction> {
		static int push(lua_State* L, lua_CFunction func, int n = 0) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushcclosure(L, func, n);
			return 1;
		}
	};

#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
	template <>
	struct unqualified_pusher<std::remove_pointer_t<detail::lua_CFunction_noexcept>> {
		static int push(lua_State* L, detail::lua_CFunction_noexcept func, int n = 0) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushcclosure(L, func, n);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<detail::lua_CFunction_noexcept> {
		static int push(lua_State* L, detail::lua_CFunction_noexcept func, int n = 0) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushcclosure(L, func, n);
			return 1;
		}
	};
#endif // noexcept function type

	template <>
	struct unqualified_pusher<c_closure> {
		static int push(lua_State* L, c_closure cc) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushcclosure(L, cc.c_function, cc.upvalues);
			return 1;
		}
	};

	template <typename Arg, typename... Args>
	struct unqualified_pusher<closure<Arg, Args...>> {
		template <std::size_t... I, typename T>
		static int push(std::index_sequence<I...>, lua_State* L, T&& c) {
			using f_tuple = decltype(std::forward<T>(c).upvalues);
			int pushcount = multi_push(L, std::get<I>(std::forward<f_tuple>(std::forward<T>(c).upvalues))...);
			return stack::push(L, c_closure(c.c_function, pushcount));
		}

		template <typename T>
		static int push(lua_State* L, T&& c) {
			return push(std::make_index_sequence<1 + sizeof...(Args)>(), L, std::forward<T>(c));
		}
	};

	template <>
	struct unqualified_pusher<void*> {
		static int push(lua_State* L, void* userdata) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushlightuserdata(L, userdata);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<const void*> {
		static int push(lua_State* L, const void* userdata) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushlightuserdata(L, const_cast<void*>(userdata));
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<lightuserdata_value> {
		static int push(lua_State* L, lightuserdata_value userdata) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushlightuserdata(L, userdata);
			return 1;
		}
	};

	template <typename T>
	struct unqualified_pusher<light<T>> {
		static int push(lua_State* L, light<T> l) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushlightuserdata(L, static_cast<void*>(l.value()));
			return 1;
		}
	};

	template <typename T>
	struct unqualified_pusher<user<T>> {
		template <bool with_meta = true, typename Key, typename... Args>
		static int push_with(lua_State* L, Key&& name, Args&&... args) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_userdata);
#endif // make sure stack doesn't overflow
       // A dumb pusher
			T* data = detail::user_allocate<T>(L);
			if (with_meta) {
				// Make sure we have a plain GC set for this data
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
				if (luaL_newmetatable(L, name) != 0) {
					lua_CFunction cdel = detail::user_alloc_destroy<T>;
					lua_pushcclosure(L, cdel, 0);
					lua_setfield(L, -2, "__gc");
				}
				lua_setmetatable(L, -2);
			}
			std::allocator<T> alloc {};
			std::allocator_traits<std::allocator<T>>::construct(alloc, data, std::forward<Args>(args)...);
			return 1;
		}

		template <typename Arg, typename... Args>
		static int push(lua_State* L, Arg&& arg, Args&&... args) {
			if constexpr (std::is_same_v<meta::unqualified_t<Arg>, metatable_key_t>) {
				const auto name = &arg[0];
				return push_with<true>(L, name, std::forward<Args>(args)...);
			}
			else if constexpr (std::is_same_v<meta::unqualified_t<Arg>, no_metatable_t>) {
				(void)arg;
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with<false>(L, name, std::forward<Args>(args)...);
			}
			else {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with(L, name, std::forward<Arg>(arg), std::forward<Args>(args)...);
			}
		}

		static int push(lua_State* L, const user<T>& u) {
			const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
			return push_with(L, name, u.value);
		}

		static int push(lua_State* L, user<T>&& u) {
			const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
			return push_with(L, name, std::move(u.value()));
		}

		static int push(lua_State* L, no_metatable_t, const user<T>& u) {
			const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
			return push_with<false>(L, name, u.value());
		}

		static int push(lua_State* L, no_metatable_t, user<T>&& u) {
			const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
			return push_with<false>(L, name, std::move(u.value()));
		}
	};

	template <>
	struct unqualified_pusher<userdata_value> {
		static int push(lua_State* L, userdata_value data) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_userdata);
#endif // make sure stack doesn't overflow
			void** ud = detail::usertype_allocate_pointer<void>(L);
			*ud = data.value();
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<const char*> {
		static int push_sized(lua_State* L, const char* str, std::size_t len) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
			lua_pushlstring(L, str, len);
			return 1;
		}

		static int push(lua_State* L, const char* str) {
			if (str == nullptr)
				return stack::push(L, lua_nil);
			return push_sized(L, str, std::char_traits<char>::length(str));
		}

		static int push(lua_State* L, const char* strb, const char* stre) {
			return push_sized(L, strb, static_cast<std::size_t>(stre - strb));
		}

		static int push(lua_State* L, const char* str, std::size_t len) {
			return push_sized(L, str, len);
		}
	};

	template <>
	struct unqualified_pusher<char*> {
		static int push_sized(lua_State* L, const char* str, std::size_t len) {
			unqualified_pusher<const char*> p {};
			(void)p;
			return p.push_sized(L, str, len);
		}

		static int push(lua_State* L, const char* str) {
			unqualified_pusher<const char*> p {};
			(void)p;
			return p.push(L, str);
		}

		static int push(lua_State* L, const char* strb, const char* stre) {
			unqualified_pusher<const char*> p {};
			(void)p;
			return p.push(L, strb, stre);
		}

		static int push(lua_State* L, const char* str, std::size_t len) {
			unqualified_pusher<const char*> p {};
			(void)p;
			return p.push(L, str, len);
		}
	};

	template <size_t N>
	struct unqualified_pusher<char[N]> {
		static int push(lua_State* L, const char (&str)[N]) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
			lua_pushlstring(L, str, std::char_traits<char>::length(str));
			return 1;
		}

		static int push(lua_State* L, const char (&str)[N], std::size_t sz) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
			lua_pushlstring(L, str, sz);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<char> {
		static int push(lua_State* L, char c) {
			const char str[2] = { c, '\0' };
			return stack::push(L, static_cast<const char*>(str), 1u);
		}
	};

#if SOL_IS_ON(SOL_CHAR8_T)
	template <>
	struct unqualified_pusher<const char8_t*> {
		static int push_sized(lua_State* L, const char8_t* str, std::size_t len) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
			lua_pushlstring(L, reinterpret_cast<const char*>(str), len);
			return 1;
		}

		static int push(lua_State* L, const char8_t* str) {
			if (str == nullptr)
				return stack::push(L, lua_nil);
			return push_sized(L, str, std::char_traits<char>::length(reinterpret_cast<const char*>(str)));
		}

		static int push(lua_State* L, const char8_t* strb, const char8_t* stre) {
			return push_sized(L, strb, static_cast<std::size_t>(stre - strb));
		}

		static int push(lua_State* L, const char8_t* str, std::size_t len) {
			return push_sized(L, str, len);
		}
	};

	template <>
	struct unqualified_pusher<char8_t*> {
		static int push_sized(lua_State* L, const char8_t* str, std::size_t len) {
			unqualified_pusher<const char8_t*> p {};
			(void)p;
			return p.push_sized(L, str, len);
		}

		static int push(lua_State* L, const char8_t* str) {
			unqualified_pusher<const char8_t*> p {};
			(void)p;
			return p.push(L, str);
		}

		static int push(lua_State* L, const char8_t* strb, const char8_t* stre) {
			unqualified_pusher<const char8_t*> p {};
			(void)p;
			return p.push(L, strb, stre);
		}

		static int push(lua_State* L, const char8_t* str, std::size_t len) {
			unqualified_pusher<const char8_t*> p {};
			(void)p;
			return p.push(L, str, len);
		}
	};

	template <size_t N>
	struct unqualified_pusher<char8_t[N]> {
		static int push(lua_State* L, const char8_t (&str)[N]) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
			const char* str_as_char = reinterpret_cast<const char*>(static_cast<const char8_t*>(str));
			lua_pushlstring(L, str_as_char, std::char_traits<char>::length(str_as_char));
			return 1;
		}

		static int push(lua_State* L, const char8_t (&str)[N], std::size_t sz) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
			lua_pushlstring(L, str, sz);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<char8_t> {
		static int push(lua_State* L, char8_t c) {
			const char8_t str[2] = { c, '\0' };
			return stack::push(L, static_cast<const char8_t*>(str), 1u);
		}
	};
#endif // char8_t


	template <typename Ch, typename Traits, typename Al>
	struct unqualified_pusher<std::basic_string<Ch, Traits, Al>> {
		static int push(lua_State* L, const std::basic_string<Ch, Traits, Al>& str) {
			if constexpr (!std::is_same_v<Ch, char>) {
				return stack::push(L, str.data(), str.size());
			}
			else {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
				lua_pushlstring(L, str.c_str(), str.size());
				return 1;
			}
		}

		static int push(lua_State* L, const std::basic_string<Ch, Traits, Al>& str, std::size_t sz) {
			if constexpr (!std::is_same_v<Ch, char>) {
				return stack::push(L, str.data(), sz);
			}
			else {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, detail::not_enough_stack_space_string);
#endif // make sure stack doesn't overflow
				lua_pushlstring(L, str.c_str(), sz);
				return 1;
			}
		}
	};

	template <typename Ch, typename Traits>
	struct unqualified_pusher<basic_string_view<Ch, Traits>> {
		static int push(lua_State* L, const basic_string_view<Ch, Traits>& sv) {
			return stack::push(L, sv.data(), sv.length());
		}

		static int push(lua_State* L, const basic_string_view<Ch, Traits>& sv, std::size_t n) {
			return stack::push(L, sv.data(), n);
		}
	};

	template <>
	struct unqualified_pusher<meta_function> {
		static int push(lua_State* L, meta_function m) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_meta_function_name);
#endif // make sure stack doesn't overflow
			const std::string& str = to_string(m);
			lua_pushlstring(L, str.c_str(), str.size());
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<absolute_index> {
		static int push(lua_State* L, absolute_index ai) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushvalue(L, ai);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<raw_index> {
		static int push(lua_State* L, raw_index ri) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushvalue(L, ri);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<ref_index> {
		static int push(lua_State* L, ref_index ri) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_rawgeti(L, LUA_REGISTRYINDEX, ri);
			return 1;
		}
	};

	template <>
	struct unqualified_pusher<const wchar_t*> {
		static int push(lua_State* L, const wchar_t* wstr) {
			return push(L, wstr, std::char_traits<wchar_t>::length(wstr));
		}

		static int push(lua_State* L, const wchar_t* wstr, std::size_t sz) {
			return push(L, wstr, wstr + sz);
		}

		static int push(lua_State* L, const wchar_t* strb, const wchar_t* stre) {
			if constexpr (sizeof(wchar_t) == 2) {
				const char16_t* sb = reinterpret_cast<const char16_t*>(strb);
				const char16_t* se = reinterpret_cast<const char16_t*>(stre);
				return stack::push(L, sb, se);
			}
			else {
				const char32_t* sb = reinterpret_cast<const char32_t*>(strb);
				const char32_t* se = reinterpret_cast<const char32_t*>(stre);
				return stack::push(L, sb, se);
			}
		}
	};

	template <>
	struct unqualified_pusher<wchar_t*> {
		static int push(lua_State* L, const wchar_t* str) {
			unqualified_pusher<const wchar_t*> p {};
			(void)p;
			return p.push(L, str);
		}

		static int push(lua_State* L, const wchar_t* strb, const wchar_t* stre) {
			unqualified_pusher<const wchar_t*> p {};
			(void)p;
			return p.push(L, strb, stre);
		}

		static int push(lua_State* L, const wchar_t* str, std::size_t len) {
			unqualified_pusher<const wchar_t*> p {};
			(void)p;
			return p.push(L, str, len);
		}
	};

	template <>
	struct unqualified_pusher<const char16_t*> {
		static int convert_into(lua_State* L, char* start, std::size_t, const char16_t* strb, const char16_t* stre) {
			char* target = start;
			char32_t cp = 0;
			for (const char16_t* strtarget = strb; strtarget < stre;) {
				auto dr = unicode::utf16_to_code_point(strtarget, stre);
				if (dr.error != unicode::error_code::ok) {
					cp = unicode::unicode_detail::replacement;
				}
				else {
					cp = dr.codepoint;
				}
				auto er = unicode::code_point_to_utf8(cp);
				const char* utf8data = er.code_units.data();
				std::memcpy(target, utf8data, er.code_units_size);
				target += er.code_units_size;
				strtarget = dr.next;
			}

			return stack::push(L, start, target);
		}

		static int push(lua_State* L, const char16_t* u16str) {
			return push(L, u16str, std::char_traits<char16_t>::length(u16str));
		}

		static int push(lua_State* L, const char16_t* u16str, std::size_t sz) {
			return push(L, u16str, u16str + sz);
		}

		static int push(lua_State* L, const char16_t* strb, const char16_t* stre) {
			char sbo[SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_];
			// if our max string space is small enough, use SBO
			// right off the bat
			std::size_t max_possible_code_units = static_cast<std::size_t>(static_cast<std::size_t>(stre - strb) * static_cast<std::size_t>(4));
			if (max_possible_code_units <= SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_) {
				return convert_into(L, sbo, max_possible_code_units, strb, stre);
			}
			// otherwise, we must manually count/check size
			std::size_t needed_size = 0;
			for (const char16_t* strtarget = strb; strtarget < stre;) {
				auto dr = unicode::utf16_to_code_point(strtarget, stre);
				auto er = unicode::code_point_to_utf8(dr.codepoint);
				needed_size += er.code_units_size;
				strtarget = dr.next;
			}
			if (needed_size < SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_) {
				return convert_into(L, sbo, needed_size, strb, stre);
			}
			std::string u8str("", 0);
			u8str.resize(needed_size);
			char* target = const_cast<char*>(u8str.data());
			return convert_into(L, target, needed_size, strb, stre);
		}
	};

	template <>
	struct unqualified_pusher<char16_t*> {
		static int push(lua_State* L, const char16_t* str) {
			unqualified_pusher<const char16_t*> p {};
			(void)p;
			return p.push(L, str);
		}

		static int push(lua_State* L, const char16_t* strb, const char16_t* stre) {
			unqualified_pusher<const char16_t*> p {};
			(void)p;
			return p.push(L, strb, stre);
		}

		static int push(lua_State* L, const char16_t* str, std::size_t len) {
			unqualified_pusher<const char16_t*> p {};
			(void)p;
			return p.push(L, str, len);
		}
	};

	template <>
	struct unqualified_pusher<const char32_t*> {
		static int convert_into(lua_State* L, char* start, std::size_t, const char32_t* strb, const char32_t* stre) {
			char* target = start;
			char32_t cp = 0;
			for (const char32_t* strtarget = strb; strtarget < stre;) {
				auto dr = unicode::utf32_to_code_point(strtarget, stre);
				if (dr.error != unicode::error_code::ok) {
					cp = unicode::unicode_detail::replacement;
				}
				else {
					cp = dr.codepoint;
				}
				auto er = unicode::code_point_to_utf8(cp);
				const char* data = er.code_units.data();
				std::memcpy(target, data, er.code_units_size);
				target += er.code_units_size;
				strtarget = dr.next;
			}
			return stack::push(L, start, target);
		}

		static int push(lua_State* L, const char32_t* u32str) {
			return push(L, u32str, u32str + std::char_traits<char32_t>::length(u32str));
		}

		static int push(lua_State* L, const char32_t* u32str, std::size_t sz) {
			return push(L, u32str, u32str + sz);
		}

		static int push(lua_State* L, const char32_t* strb, const char32_t* stre) {
			char sbo[SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_];
			// if our max string space is small enough, use SBO
			// right off the bat
			std::size_t max_possible_code_units = static_cast<std::size_t>(static_cast<std::size_t>(stre - strb) * static_cast<std::size_t>(4));
			if (max_possible_code_units <= SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_) {
				return convert_into(L, sbo, max_possible_code_units, strb, stre);
			}
			// otherwise, we must manually count/check size
			std::size_t needed_size = 0;
			for (const char32_t* strtarget = strb; strtarget < stre;) {
				auto dr = unicode::utf32_to_code_point(strtarget, stre);
				auto er = unicode::code_point_to_utf8(dr.codepoint);
				needed_size += er.code_units_size;
				strtarget = dr.next;
			}
			if (needed_size < SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_) {
				return convert_into(L, sbo, needed_size, strb, stre);
			}
			std::string u8str("", 0);
			u8str.resize(needed_size);
			char* target = const_cast<char*>(u8str.data());
			return convert_into(L, target, needed_size, strb, stre);
		}
	};

	template <>
	struct unqualified_pusher<char32_t*> {
		static int push(lua_State* L, const char32_t* str) {
			unqualified_pusher<const char32_t*> p {};
			(void)p;
			return p.push(L, str);
		}

		static int push(lua_State* L, const char32_t* strb, const char32_t* stre) {
			unqualified_pusher<const char32_t*> p {};
			(void)p;
			return p.push(L, strb, stre);
		}

		static int push(lua_State* L, const char32_t* str, std::size_t len) {
			unqualified_pusher<const char32_t*> p {};
			(void)p;
			return p.push(L, str, len);
		}
	};

	template <size_t N>
	struct unqualified_pusher<wchar_t[N]> {
		static int push(lua_State* L, const wchar_t (&str)[N]) {
			return push(L, str, std::char_traits<wchar_t>::length(str));
		}

		static int push(lua_State* L, const wchar_t (&str)[N], std::size_t sz) {
			const wchar_t* str_ptr = static_cast<const wchar_t*>(str);
			return stack::push<const wchar_t*>(L, str_ptr, str_ptr + sz);
		}
	};

	template <size_t N>
	struct unqualified_pusher<char16_t[N]> {
		static int push(lua_State* L, const char16_t (&str)[N]) {
			return push(L, str, std::char_traits<char16_t>::length(str));
		}

		static int push(lua_State* L, const char16_t (&str)[N], std::size_t sz) {
			const char16_t* str_ptr = static_cast<const char16_t*>(str);
			return stack::push<const char16_t*>(L, str_ptr, str_ptr + sz);
		}
	};

	template <size_t N>
	struct unqualified_pusher<char32_t[N]> {
		static int push(lua_State* L, const char32_t (&str)[N]) {
			return push(L, str, std::char_traits<char32_t>::length(str));
		}

		static int push(lua_State* L, const char32_t (&str)[N], std::size_t sz) {
			const char32_t* str_ptr = static_cast<const char32_t*>(str);
			return stack::push<const char32_t*>(L, str_ptr, str_ptr + sz);
		}
	};

	template <>
	struct unqualified_pusher<wchar_t> {
		static int push(lua_State* L, wchar_t c) {
			const wchar_t str[2] = { c, '\0' };
			return stack::push(L, static_cast<const wchar_t*>(str), 1u);
		}
	};

	template <>
	struct unqualified_pusher<char16_t> {
		static int push(lua_State* L, char16_t c) {
			const char16_t str[2] = { c, '\0' };
			return stack::push(L, static_cast<const char16_t*>(str), 1u);
		}
	};

	template <>
	struct unqualified_pusher<char32_t> {
		static int push(lua_State* L, char32_t c) {
			const char32_t str[2] = { c, '\0' };
			return stack::push(L, static_cast<const char32_t*>(str), 1u);
		}
	};

	template <typename... Args>
	struct unqualified_pusher<std::tuple<Args...>> {
		template <std::size_t... I, typename T>
		static int push(std::index_sequence<I...>, lua_State* L, T&& t) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, static_cast<int>(sizeof...(I)), detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			int pushcount = 0;
			(void)detail::swallow { 0, (pushcount += stack::push(L, std::get<I>(std::forward<T>(t))), 0)... };
			return pushcount;
		}

		template <typename T>
		static int push(lua_State* L, T&& t) {
			return push(std::index_sequence_for<Args...>(), L, std::forward<T>(t));
		}
	};

	template <typename A, typename B>
	struct unqualified_pusher<std::pair<A, B>> {
		template <typename T>
		static int push(lua_State* L, T&& t) {
			int pushcount = stack::push(L, std::get<0>(std::forward<T>(t)));
			pushcount += stack::push(L, std::get<1>(std::forward<T>(t)));
			return pushcount;
		}
	};

	template <typename T>
	struct unqualified_pusher<T, std::enable_if_t<meta::is_optional_v<T>>> {
		using ValueType = typename meta::unqualified_t<T>::value_type;

		template <typename Optional>
		static int push(lua_State* L, Optional&& op) {
			using QualifiedValueType = meta::conditional_t<std::is_lvalue_reference_v<Optional>, ValueType&, ValueType&&>;
			if (!op) {
				return stack::push(L, nullopt);
			}
			return stack::push(L, static_cast<QualifiedValueType>(op.value()));
		}
	};

	template <typename T>
	struct unqualified_pusher<forward_as_value_t<T>> {
		static int push(lua_State* L, const forward_as_value_t<T>& value_) {
			return stack::push<T>(L, value_.value());
		}

		static int push(lua_State* L, forward_as_value_t<T>&& value_) {
			return stack::push<T>(L, std::move(value_).value());
		}
	};

	template <>
	struct unqualified_pusher<nullopt_t> {
		static int push(lua_State* L, nullopt_t) noexcept {
			return stack::push(L, lua_nil);
		}
	};

	template <>
	struct unqualified_pusher<std::nullptr_t> {
		static int push(lua_State* L, std::nullptr_t) noexcept {
			return stack::push(L, lua_nil);
		}
	};

	template <>
	struct unqualified_pusher<this_state> {
		static int push(lua_State*, const this_state&) noexcept {
			return 0;
		}
	};

	template <>
	struct unqualified_pusher<this_main_state> {
		static int push(lua_State*, const this_main_state&) noexcept {
			return 0;
		}
	};

	template <>
	struct unqualified_pusher<new_table> {
		static int push(lua_State* L, const new_table& nt) {
			lua_createtable(L, nt.sequence_hint, nt.map_hint);
			return 1;
		}
	};

	template <typename Allocator>
	struct unqualified_pusher<basic_bytecode<Allocator>> {
		template <typename T>
		static int push(lua_State* L, T&& bc, const char* bytecode_name) {
			const auto first = bc.data();
			const auto bcsize = bc.size();
			// pushes either the function, or an error
			// if it errors, shit goes south, and people can test that upstream
			(void)luaL_loadbuffer(
				L, reinterpret_cast<const char*>(first), static_cast<std::size_t>(bcsize * (sizeof(*first) / sizeof(const char))), bytecode_name);
			return 1;
		}

		template <typename T>
		static int push(lua_State* L, T&& bc) {
			return push(L, std::forward<bc>(bc), "bytecode");
		}
	};

#if SOL_IS_ON(SOL_STD_VARIANT)
	namespace stack_detail {

		struct push_function {
			lua_State* L;

			push_function(lua_State* L_) noexcept : L(L_) {
			}

			template <typename T>
			int operator()(T&& value) const {
				return stack::push<T>(L, std::forward<T>(value));
			}
		};

	} // namespace stack_detail

	template <typename... Tn>
	struct unqualified_pusher<std::variant<Tn...>> {
		static int push(lua_State* L, const std::variant<Tn...>& v) {
			return std::visit(stack_detail::push_function(L), v);
		}

		static int push(lua_State* L, std::variant<Tn...>&& v) {
			return std::visit(stack_detail::push_function(L), std::move(v));
		}
	};
#endif // Variant because Clang is terrible

}} // namespace sol::stack

#endif // SOL_STACK_PUSH_HPP
