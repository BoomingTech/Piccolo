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

#ifndef SOL_STACK_UNQUALIFIED_GET_HPP
#define SOL_STACK_UNQUALIFIED_GET_HPP

#include <sol/stack_core.hpp>
#include <sol/usertype_traits.hpp>
#include <sol/inheritance.hpp>
#include <sol/overload.hpp>
#include <sol/error.hpp>
#include <sol/unicode.hpp>

#include <memory>
#include <functional>
#include <utility>
#include <cstdlib>
#include <cmath>
#include <string_view>
#if SOL_IS_ON(SOL_STD_VARIANT)
#include <variant>
#endif // Apple clang screwed up

namespace sol { namespace stack {

	namespace stack_detail {
		template <typename Ch>
		struct count_code_units_utf {
			std::size_t needed_size;

			count_code_units_utf() : needed_size(0) {
			}

			void operator()(const unicode::encoded_result<Ch> er) {
				needed_size += er.code_units_size;
			}
		};

		template <typename Ch, typename ErCh>
		struct copy_code_units_utf {
			Ch* target_;

			copy_code_units_utf(Ch* target) : target_(target) {
			}

			void operator()(const unicode::encoded_result<ErCh> er) {
				std::memcpy(target_, er.code_units.data(), er.code_units_size * sizeof(ErCh));
				target_ += er.code_units_size;
			}
		};

		template <typename Ch, typename F>
		inline void convert(const char* strb, const char* stre, F&& f) {
			char32_t cp = 0;
			for (const char* strtarget = strb; strtarget < stre;) {
				auto dr = unicode::utf8_to_code_point(strtarget, stre);
				if (dr.error != unicode::error_code::ok) {
					cp = unicode::unicode_detail::replacement;
					++strtarget;
				}
				else {
					cp = dr.codepoint;
					strtarget = dr.next;
				}
				if constexpr (std::is_same_v<Ch, char32_t>) {
					auto er = unicode::code_point_to_utf32(cp);
					f(er);
				}
				else {
					auto er = unicode::code_point_to_utf16(cp);
					f(er);
				}
			}
		}

		template <typename BaseCh, typename S>
		inline S get_into(lua_State* L, int index, record& tracking) {
			using Ch = typename S::value_type;
			tracking.use(1);
			size_t len;
			auto utf8p = lua_tolstring(L, index, &len);
			if (len < 1)
				return S();
			const char* strb = utf8p;
			const char* stre = utf8p + len;
			stack_detail::count_code_units_utf<BaseCh> count_units;
			convert<BaseCh>(strb, stre, count_units);
			S r(count_units.needed_size, static_cast<Ch>(0));
			r.resize(count_units.needed_size);
			Ch* target = &r[0];
			stack_detail::copy_code_units_utf<Ch, BaseCh> copy_units(target);
			convert<BaseCh>(strb, stre, copy_units);
			return r;
		}
	} // namespace stack_detail

	template <typename T, typename>
	struct unqualified_getter {
		static decltype(auto) get(lua_State* L, int index, record& tracking) {
			if constexpr (std::is_same_v<T, bool>) {
				tracking.use(1);
				return lua_toboolean(L, index) != 0;
			}
			else if constexpr (std::is_enum_v<T>) {
				tracking.use(1);
				return static_cast<T>(lua_tointegerx(L, index, nullptr));
			}
			else if constexpr (std::is_integral_v<T> || std::is_same_v<T, lua_Integer>) {
				tracking.use(1);
#if SOL_LUA_VERSION_I_ >= 503
				if (lua_isinteger(L, index) != 0) {
					return static_cast<T>(lua_tointeger(L, index));
				}
#endif
				return static_cast<T>(llround(lua_tonumber(L, index)));
			}
			else if constexpr (std::is_floating_point_v<T> || std::is_same_v<T, lua_Number>) {
				tracking.use(1);
				return static_cast<T>(lua_tonumber(L, index));
			}
			else if constexpr (is_lua_reference_v<T>) {
				if constexpr (is_global_table_v<T>) {
					tracking.use(1);
					return T(L, global_tag);
				}
				else {
					tracking.use(1);
					return T(L, index);
				}
			}
			else if constexpr (is_unique_usertype_v<T>) {
				using actual = unique_usertype_actual_t<T>;

				tracking.use(1);
				void* memory = lua_touserdata(L, index);
				void* aligned_memory = detail::align_usertype_unique<actual>(memory);
				actual* typed_memory = static_cast<actual*>(aligned_memory);
				return *typed_memory;
			}
			else if constexpr (meta::is_optional_v<T>) {
				using ValueType = typename T::value_type;
				return unqualified_check_getter<ValueType>::template get_using<T>(L, index, &no_panic, tracking);
			}
			else if constexpr (std::is_same_v<T, luaL_Stream*>) {
				luaL_Stream* pstream = static_cast<luaL_Stream*>(lua_touserdata(L, index));
				return pstream;
			}
			else if constexpr (std::is_same_v<T, luaL_Stream>) {
				luaL_Stream* pstream = static_cast<luaL_Stream*>(lua_touserdata(L, index));
				return *pstream;
			}
#if SOL_IS_ON(SOL_GET_FUNCTION_POINTER_UNSAFE)
			else if constexpr (std::is_function_v<T> || (std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>)) {
				return stack_detail::get_function_pointer<std::remove_pointer_t<T>>(L, index, tracking);
			}
#endif
			else {
				return stack_detail::unchecked_unqualified_get<detail::as_value_tag<T>>(L, index, tracking);
			}
		}
	};

	template <typename X, typename>
	struct qualified_getter {
		static decltype(auto) get(lua_State* L, int index, record& tracking) {
			using Tu = meta::unqualified_t<X>;
			static constexpr bool is_userdata_of_some_kind
				= !std::is_reference_v<
				       X> && is_container_v<Tu> && std::is_default_constructible_v<Tu> && !is_lua_primitive_v<Tu> && !is_transparent_argument_v<Tu>;
			if constexpr (is_userdata_of_some_kind) {
				if (type_of(L, index) == type::userdata) {
					return static_cast<Tu>(stack_detail::unchecked_unqualified_get<Tu>(L, index, tracking));
				}
				else {
					return stack_detail::unchecked_unqualified_get<sol::nested<Tu>>(L, index, tracking);
				}
			}
			else if constexpr (!std::is_reference_v<X> && is_unique_usertype_v<Tu> && !is_actual_type_rebindable_for_v<Tu>) {
				using element = unique_usertype_element_t<Tu>;
				using actual = unique_usertype_actual_t<Tu>;
				tracking.use(1);
				void* memory = lua_touserdata(L, index);
				memory = detail::align_usertype_unique_destructor(memory);
				detail::unique_destructor& pdx = *static_cast<detail::unique_destructor*>(memory);
				if (&detail::usertype_unique_alloc_destroy<element, Tu> == pdx) {
					memory = detail::align_usertype_unique_tag<true, false>(memory);
					memory = detail::align_usertype_unique<actual, true, false>(memory);
					actual* mem = static_cast<actual*>(memory);
					return static_cast<actual>(*mem);
				}
				actual r {};
				if constexpr (!derive<element>::value) {
#if SOL_IS_ON(SOL_DEBUG_BUILD)
					// In debug mode we would rather abort you for this grave failure rather
					// than let you deref a null pointer and fuck everything over
					std::abort();
#endif
					return static_cast<actual>(std::move(r));
				}
				else {
					memory = detail::align_usertype_unique_tag<true, false>(memory);
					detail::unique_tag& ic = *reinterpret_cast<detail::unique_tag*>(memory);
					memory = detail::align_usertype_unique<actual, true, false>(memory);
					string_view ti = usertype_traits<element>::qualified_name();
					int cast_operation;
					if constexpr (is_actual_type_rebindable_for_v<Tu>) {
						using rebound_actual_type = unique_usertype_rebind_actual_t<Tu, void>;
						string_view rebind_ti = usertype_traits<rebound_actual_type>::qualified_name();
						cast_operation = ic(memory, &r, ti, rebind_ti);
					}
					else {
						string_view rebind_ti("");
						cast_operation = ic(memory, &r, ti, rebind_ti);
					}
					switch (cast_operation) {
					case 1: {
						// it's a perfect match,
						// alias memory directly
						actual* mem = static_cast<actual*>(memory);
						return static_cast<actual>(*mem);
					}
					case 2:
						// it's a base match, return the
						// aliased creation
						return static_cast<actual>(std::move(r));
					default:
						// uh oh..
						break;
					}
#if SOL_IS_ON(SOL_DEBUG_BUILD)
					// In debug mode we would rather abort you for this grave failure rather
					// than let you deref a null pointer and fuck everything over
					std::abort();
#endif
					return static_cast<actual>(r);
				}
			}
			else {
				return stack_detail::unchecked_unqualified_get<Tu>(L, index, tracking);
			}
		}
	};

	template <typename T>
	struct unqualified_getter<as_table_t<T>> {
		using Tu = meta::unqualified_t<T>;

		template <typename V>
		static void push_back_at_end(std::true_type, types<V>, lua_State* L, T& cont, std::size_t) {
			cont.push_back(stack::get<V>(L, -lua_size<V>::value));
		}

		template <typename V>
		static void push_back_at_end(std::false_type, types<V> t, lua_State* L, T& cont, std::size_t idx) {
			insert_at_end(meta::has_insert<Tu>(), t, L, cont, idx);
		}

		template <typename V>
		static void insert_at_end(std::true_type, types<V>, lua_State* L, T& cont, std::size_t) {
			using std::cend;
			cont.insert(cend(cont), stack::get<V>(L, -lua_size<V>::value));
		}

		template <typename V>
		static void insert_at_end(std::false_type, types<V>, lua_State* L, T& cont, std::size_t idx) {
			cont[idx] = stack::get<V>(L, -lua_size<V>::value);
		}

		static bool max_size_check(std::false_type, T&, std::size_t) {
			return false;
		}

		static bool max_size_check(std::true_type, T& cont, std::size_t idx) {
			return idx >= cont.max_size();
		}

		static T get(lua_State* L, int relindex, record& tracking) {
			return get(meta::is_associative<Tu>(), L, relindex, tracking);
		}

		static T get(std::false_type, lua_State* L, int relindex, record& tracking) {
			typedef typename Tu::value_type V;
			return get(types<V>(), L, relindex, tracking);
		}

		template <typename V>
		static T get(types<V> t, lua_State* L, int relindex, record& tracking) {
			tracking.use(1);

			// the W4 flag is really great,
			// so great that it can tell my for loops (twice nested)
			// below never actually terminate
			// without hitting where the gotos have infested

			// so now I would get the error W4XXX unreachable
			// me that the return cont at the end of this function
			// which is fair until other compilers complain
			// that there isn't a return and that based on
			// SOME MAGICAL FORCE
			// control flow falls off the end of a non-void function
			// so it needs to be there for the compilers that are
			// too flimsy to analyze the basic blocks...
			// (I'm sure I should file a bug but those compilers are already
			// in the wild; it doesn't matter if I fix them,
			// someone else is still going to get some old-ass compiler
			// and then bother me about the unclean build for the 30th
			// time)

			// "Why not an IIFE?"
			// Because additional lambdas / functions which serve as
			// capture-all-and-then-invoke bloat binary sizes
			// by an actually detectable amount
			// (one user uses sol2 pretty heavily and 22 MB of binary size
			// was saved by reducing reliance on lambdas in templates)

			// This would really be solved by having break N;
			// be a real, proper thing...
			// but instead, we have to use labels and gotos
			// and earn the universal vitriol of the dogmatic
			// programming community

			// all in all: W4 is great!~

			int index = lua_absindex(L, relindex);
			T cont;
			std::size_t idx = 0;
#if SOL_LUA_VERSION_I_ >= 503
			// This method is HIGHLY performant over regular table iteration
			// thanks to the Lua API changes in 5.3
			// Questionable in 5.4
			for (lua_Integer i = 0;; i += lua_size<V>::value) {
				if (max_size_check(meta::has_max_size<Tu>(), cont, idx)) {
					// see above comment
					goto done;
				}
				bool isnil = false;
				for (int vi = 0; vi < lua_size<V>::value; ++vi) {
#if SOL_IS_ON(SOL_LUA_NIL_IN_TABLES) && SOL_LUA_VERSION_I_ >= 600
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
					luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
					lua_pushinteger(L, static_cast<lua_Integer>(i + vi));
					if (lua_keyin(L, index) == 0) {
						// it's time to stop
						isnil = true;
					}
					else {
						// we have a key, have to get the value
						lua_geti(L, index, i + vi);
					}
#else
					type vt = static_cast<type>(lua_geti(L, index, i + vi));
					isnil = vt == type::none || vt == type::lua_nil;
#endif
					if (isnil) {
						if (i == 0) {
							break;
						}
#if SOL_IS_ON(SOL_LUA_NIL_IN_TABLES) && SOL_LUA_VERSION_I_ >= 600
						lua_pop(L, vi);
#else
						lua_pop(L, (vi + 1));
#endif
						// see above comment
						goto done;
					}
				}
				if (isnil) {
#if SOL_IS_ON(SOL_LUA_NIL_IN_TABLES) && SOL_LUA_VERSION_I_ >= 600
#else
					lua_pop(L, lua_size<V>::value);
#endif
					continue;
				}

				push_back_at_end(meta::has_push_back<Tu>(), t, L, cont, idx);
				++idx;
				lua_pop(L, lua_size<V>::value);
			}
#else
			// Zzzz slower but necessary thanks to the lower version API and missing functions qq
			for (lua_Integer i = 0;; i += lua_size<V>::value, lua_pop(L, lua_size<V>::value)) {
				if (idx >= cont.max_size()) {
					// see above comment
					goto done;
				}
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 2, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
				bool isnil = false;
				for (int vi = 0; vi < lua_size<V>::value; ++vi) {
					lua_pushinteger(L, i);
					lua_gettable(L, index);
					type vt = type_of(L, -1);
					isnil = vt == type::lua_nil;
					if (isnil) {
						if (i == 0) {
							break;
						}
						lua_pop(L, (vi + 1));
						// see above comment
						goto done;
					}
				}
				if (isnil)
					continue;
				push_back_at_end(meta::has_push_back<Tu>(), t, L, cont, idx);
				++idx;
			}
#endif
		done:
			return cont;
		}

		static T get(std::true_type, lua_State* L, int index, record& tracking) {
			typedef typename Tu::value_type P;
			typedef typename P::first_type K;
			typedef typename P::second_type V;
			return get(types<K, V>(), L, index, tracking);
		}

		template <typename K, typename V>
		static T get(types<K, V>, lua_State* L, int relindex, record& tracking) {
			tracking.use(1);

#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 3, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow

			T associative;
			int index = lua_absindex(L, relindex);
			lua_pushnil(L);
			while (lua_next(L, index) != 0) {
				decltype(auto) key = stack::check_get<K>(L, -2);
				if (!key) {
					lua_pop(L, 1);
					continue;
				}
				associative.emplace(std::forward<decltype(*key)>(*key), stack::get<V>(L, -1));
				lua_pop(L, 1);
			}
			return associative;
		}
	};

	template <typename T, typename Al>
	struct unqualified_getter<as_table_t<std::forward_list<T, Al>>> {
		typedef std::forward_list<T, Al> C;

		static C get(lua_State* L, int relindex, record& tracking) {
			return get(meta::has_key_value_pair<C>(), L, relindex, tracking);
		}

		static C get(std::true_type, lua_State* L, int index, record& tracking) {
			typedef typename T::value_type P;
			typedef typename P::first_type K;
			typedef typename P::second_type V;
			return get(types<K, V>(), L, index, tracking);
		}

		static C get(std::false_type, lua_State* L, int relindex, record& tracking) {
			typedef typename C::value_type V;
			return get(types<V>(), L, relindex, tracking);
		}

		template <typename V>
		static C get(types<V>, lua_State* L, int relindex, record& tracking) {
			tracking.use(1);
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 3, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow

			int index = lua_absindex(L, relindex);
			C cont;
			auto at = cont.cbefore_begin();
			std::size_t idx = 0;
#if SOL_LUA_VERSION_I_ >= 503
			// This method is HIGHLY performant over regular table iteration thanks to the Lua API changes in 5.3
			for (lua_Integer i = 0;; i += lua_size<V>::value, lua_pop(L, lua_size<V>::value)) {
				if (idx >= cont.max_size()) {
					goto done;
				}
				bool isnil = false;
				for (int vi = 0; vi < lua_size<V>::value; ++vi) {
					type t = static_cast<type>(lua_geti(L, index, i + vi));
					isnil = t == type::lua_nil;
					if (isnil) {
						if (i == 0) {
							break;
						}
						lua_pop(L, (vi + 1));
						goto done;
					}
				}
				if (isnil)
					continue;
				at = cont.insert_after(at, stack::get<V>(L, -lua_size<V>::value));
				++idx;
			}
#else
			// Zzzz slower but necessary thanks to the lower version API and missing functions qq
			for (lua_Integer i = 0;; i += lua_size<V>::value, lua_pop(L, lua_size<V>::value)) {
				if (idx >= cont.max_size()) {
					goto done;
				}
				bool isnil = false;
				for (int vi = 0; vi < lua_size<V>::value; ++vi) {
					lua_pushinteger(L, i);
					lua_gettable(L, index);
					type t = type_of(L, -1);
					isnil = t == type::lua_nil;
					if (isnil) {
						if (i == 0) {
							break;
						}
						lua_pop(L, (vi + 1));
						goto done;
					}
				}
				if (isnil)
					continue;
				at = cont.insert_after(at, stack::get<V>(L, -lua_size<V>::value));
				++idx;
			}
#endif
		done:
			return cont;
		}

		template <typename K, typename V>
		static C get(types<K, V>, lua_State* L, int relindex, record& tracking) {
			tracking.use(1);

#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 3, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow

			C associative;
			auto at = associative.cbefore_begin();
			int index = lua_absindex(L, relindex);
			lua_pushnil(L);
			while (lua_next(L, index) != 0) {
				decltype(auto) key = stack::check_get<K>(L, -2);
				if (!key) {
					lua_pop(L, 1);
					continue;
				}
				at = associative.emplace_after(at, std::forward<decltype(*key)>(*key), stack::get<V>(L, -1));
				lua_pop(L, 1);
			}
			return associative;
		}
	};

	template <typename T>
	struct unqualified_getter<nested<T>> {
		static T get(lua_State* L, int index, record& tracking) {
			using Tu = meta::unqualified_t<T>;
			if constexpr (is_container_v<Tu>) {
				if constexpr (meta::is_associative<Tu>::value) {
					typedef typename T::value_type P;
					typedef typename P::first_type K;
					typedef typename P::second_type V;
					unqualified_getter<as_table_t<T>> g{};
					return g.get(types<K, nested<V>>(), L, index, tracking);
				}
				else {
					typedef typename T::value_type V;
					unqualified_getter<as_table_t<T>> g{};
					return g.get(types<nested<V>>(), L, index, tracking);
				}
			}
			else {
				unqualified_getter<Tu> g{};
				return g.get(L, index, tracking);
			}
		}
	};

	template <typename T>
	struct unqualified_getter<as_container_t<T>> {
		static decltype(auto) get(lua_State* L, int index, record& tracking) {
			return stack::unqualified_get<T>(L, index, tracking);
		}
	};

	template <typename T>
	struct unqualified_getter<as_container_t<T>*> {
		static decltype(auto) get(lua_State* L, int index, record& tracking) {
			return stack::unqualified_get<T*>(L, index, tracking);
		}
	};

	template <>
	struct unqualified_getter<userdata_value> {
		static userdata_value get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return userdata_value(lua_touserdata(L, index));
		}
	};

	template <>
	struct unqualified_getter<lightuserdata_value> {
		static lightuserdata_value get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return lightuserdata_value(lua_touserdata(L, index));
		}
	};

	template <typename T>
	struct unqualified_getter<light<T>> {
		static light<T> get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			void* memory = lua_touserdata(L, index);
			return light<T>(static_cast<T*>(memory));
		}
	};

	template <typename T>
	struct unqualified_getter<user<T>> {
		static std::add_lvalue_reference_t<T> get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			void* memory = lua_touserdata(L, index);
			memory = detail::align_user<T>(memory);
			return *static_cast<std::remove_reference_t<T>*>(memory);
		}
	};

	template <typename T>
	struct unqualified_getter<user<T*>> {
		static T* get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			void* memory = lua_touserdata(L, index);
			memory = detail::align_user<T*>(memory);
			return static_cast<T*>(memory);
		}
	};

	template <>
	struct unqualified_getter<type> {
		static type get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return static_cast<type>(lua_type(L, index));
		}
	};

	template <>
	struct unqualified_getter<std::string> {
		static std::string get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			std::size_t len;
			auto str = lua_tolstring(L, index, &len);
			return std::string(str, len);
		}
	};

	template <>
	struct unqualified_getter<const char*> {
		static const char* get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			size_t sz;
			return lua_tolstring(L, index, &sz);
		}
	};

	template <>
	struct unqualified_getter<char> {
		static char get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			size_t len;
			auto str = lua_tolstring(L, index, &len);
			return len > 0 ? str[0] : '\0';
		}
	};

	template <typename Traits>
	struct unqualified_getter<basic_string_view<char, Traits>> {
		static string_view get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			size_t sz;
			const char* str = lua_tolstring(L, index, &sz);
			return basic_string_view<char, Traits>(str, sz);
		}
	};

	template <typename Traits, typename Al>
	struct unqualified_getter<std::basic_string<wchar_t, Traits, Al>> {
		using S = std::basic_string<wchar_t, Traits, Al>;
		static S get(lua_State* L, int index, record& tracking) {
			using Ch = meta::conditional_t<sizeof(wchar_t) == 2, char16_t, char32_t>;
			return stack_detail::get_into<Ch, S>(L, index, tracking);
		}
	};

	template <typename Traits, typename Al>
	struct unqualified_getter<std::basic_string<char16_t, Traits, Al>> {
		static std::basic_string<char16_t, Traits, Al> get(lua_State* L, int index, record& tracking) {
			return stack_detail::get_into<char16_t, std::basic_string<char16_t, Traits, Al>>(L, index, tracking);
		}
	};

	template <typename Traits, typename Al>
	struct unqualified_getter<std::basic_string<char32_t, Traits, Al>> {
		static std::basic_string<char32_t, Traits, Al> get(lua_State* L, int index, record& tracking) {
			return stack_detail::get_into<char32_t, std::basic_string<char32_t, Traits, Al>>(L, index, tracking);
		}
	};

	template <>
	struct unqualified_getter<char16_t> {
		static char16_t get(lua_State* L, int index, record& tracking) {
			string_view utf8 = stack::get<string_view>(L, index, tracking);
			const char* strb = utf8.data();
			const char* stre = utf8.data() + utf8.size();
			char32_t cp = 0;
			auto dr = unicode::utf8_to_code_point(strb, stre);
			if (dr.error != unicode::error_code::ok) {
				cp = unicode::unicode_detail::replacement;
			}
			else {
				cp = dr.codepoint;
			}
			auto er = unicode::code_point_to_utf16(cp);
			return er.code_units[0];
		}
	};

	template <>
	struct unqualified_getter<char32_t> {
		static char32_t get(lua_State* L, int index, record& tracking) {
			string_view utf8 = stack::get<string_view>(L, index, tracking);
			const char* strb = utf8.data();
			const char* stre = utf8.data() + utf8.size();
			char32_t cp = 0;
			auto dr = unicode::utf8_to_code_point(strb, stre);
			if (dr.error != unicode::error_code::ok) {
				cp = unicode::unicode_detail::replacement;
			}
			else {
				cp = dr.codepoint;
			}
			auto er = unicode::code_point_to_utf32(cp);
			return er.code_units[0];
		}
	};

	template <>
	struct unqualified_getter<wchar_t> {
		static wchar_t get(lua_State* L, int index, record& tracking) {
			typedef meta::conditional_t<sizeof(wchar_t) == 2, char16_t, char32_t> Ch;
			unqualified_getter<Ch> g;
			(void)g;
			auto c = g.get(L, index, tracking);
			return static_cast<wchar_t>(c);
		}
	};

	template <>
	struct unqualified_getter<meta_function> {
		static meta_function get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			const char* name = unqualified_getter<const char*> {}.get(L, index, tracking);
			const auto& mfnames = meta_function_names();
			for (std::size_t i = 0; i < mfnames.size(); ++i)
				if (mfnames[i] == name)
					return static_cast<meta_function>(i);
			return meta_function::construct;
		}
	};

	template <>
	struct unqualified_getter<lua_nil_t> {
		static lua_nil_t get(lua_State*, int, record& tracking) {
			tracking.use(1);
			return lua_nil;
		}
	};

	template <>
	struct unqualified_getter<std::nullptr_t> {
		static std::nullptr_t get(lua_State*, int, record& tracking) {
			tracking.use(1);
			return nullptr;
		}
	};

	template <>
	struct unqualified_getter<nullopt_t> {
		static nullopt_t get(lua_State*, int, record& tracking) {
			tracking.use(1);
			return nullopt;
		}
	};

	template <>
	struct unqualified_getter<this_state> {
		static this_state get(lua_State* L, int, record& tracking) {
			tracking.use(0);
			return this_state(L);
		}
	};

	template <>
	struct unqualified_getter<this_main_state> {
		static this_main_state get(lua_State* L, int, record& tracking) {
			tracking.use(0);
			return this_main_state(main_thread(L, L));
		}
	};

	template <>
	struct unqualified_getter<lua_CFunction> {
		static lua_CFunction get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return lua_tocfunction(L, index);
		}
	};

	template <>
	struct unqualified_getter<c_closure> {
		static c_closure get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return c_closure(lua_tocfunction(L, index), -1);
		}
	};

	template <>
	struct unqualified_getter<error> {
		static error get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			size_t sz = 0;
			const char* err = lua_tolstring(L, index, &sz);
			if (err == nullptr) {
				return error(detail::direct_error, "");
			}
			return error(detail::direct_error, std::string(err, sz));
		}
	};

	template <>
	struct unqualified_getter<void*> {
		static void* get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return lua_touserdata(L, index);
		}
	};

	template <>
	struct unqualified_getter<const void*> {
		static const void* get(lua_State* L, int index, record& tracking) {
			tracking.use(1);
			return lua_touserdata(L, index);
		}
	};

	template <typename T>
	struct unqualified_getter<detail::as_value_tag<T>> {
		static T* get_no_lua_nil(lua_State* L, int index, record& tracking) {
			void* memory = lua_touserdata(L, index);
#if SOL_IS_ON(SOL_USE_INTEROP)
			auto ugr = stack_detail::interop_get<T>(L, index, memory, tracking);
			if (ugr.first) {
				return ugr.second;
			}
#endif // interop extensibility
			tracking.use(1);
			void* rawdata = detail::align_usertype_pointer(memory);
			void** pudata = static_cast<void**>(rawdata);
			void* udata = *pudata;
			return get_no_lua_nil_from(L, udata, index, tracking);
		}

		static T* get_no_lua_nil_from(lua_State* L, void* udata, int index, record&) {
			bool has_derived = derive<T>::value || weak_derive<T>::value;
			if (has_derived) {
				if (lua_getmetatable(L, index) == 1) {
					lua_getfield(L, -1, &detail::base_class_cast_key()[0]);
					if (type_of(L, -1) != type::lua_nil) {
						void* basecastdata = lua_touserdata(L, -1);
						detail::inheritance_cast_function ic = reinterpret_cast<detail::inheritance_cast_function>(basecastdata);
						// use the casting function to properly adjust the pointer for the desired T
						udata = ic(udata, usertype_traits<T>::qualified_name());
					}
					lua_pop(L, 2);
				}
			}
			if constexpr (std::is_function_v<T>) {
				T* func = reinterpret_cast<T*>(udata);
				return func;
			}
			else {
				T* obj = static_cast<T*>(udata);
				return obj;
			}
		}

		static T& get(lua_State* L, int index, record& tracking) {
			return *get_no_lua_nil(L, index, tracking);
		}
	};

	template <typename T>
	struct unqualified_getter<detail::as_pointer_tag<T>> {
		static T* get(lua_State* L, int index, record& tracking) {
			type t = type_of(L, index);
			if (t == type::lua_nil) {
				tracking.use(1);
				return nullptr;
			}
			unqualified_getter<detail::as_value_tag<T>> g{};
			return g.get_no_lua_nil(L, index, tracking);
		}
	};

	template <typename T>
	struct unqualified_getter<non_null<T*>> {
		static T* get(lua_State* L, int index, record& tracking) {
			unqualified_getter<detail::as_value_tag<T>> g{};
			return g.get_no_lua_nil(L, index, tracking);
		}
	};

	template <typename T>
	struct unqualified_getter<T&> {
		static T& get(lua_State* L, int index, record& tracking) {
			unqualified_getter<detail::as_value_tag<T>> g{};
			return g.get(L, index, tracking);
		}
	};

	template <typename T>
	struct unqualified_getter<std::reference_wrapper<T>> {
		static T& get(lua_State* L, int index, record& tracking) {
			unqualified_getter<T&> g{};
			return g.get(L, index, tracking);
		}
	};

	template <typename T>
	struct unqualified_getter<T*> {
		static T* get(lua_State* L, int index, record& tracking) {
#if SOL_IS_ON(SOL_GET_FUNCTION_POINTER_UNSAFE)
			if constexpr (std::is_function_v<T>) {
				return stack_detail::get_function_pointer<T>(L, index, tracking);
			}
			else {
				unqualified_getter<detail::as_pointer_tag<T>> g{};
				return g.get(L, index, tracking);
			}
#else
			unqualified_getter<detail::as_pointer_tag<T>> g{};
			return g.get(L, index, tracking);
#endif
		}
	};

	template <typename... Tn>
	struct unqualified_getter<std::tuple<Tn...>> {
		typedef std::tuple<decltype(stack::get<Tn>(nullptr, 0))...> R;

		template <typename... Args>
		static R apply(std::index_sequence<>, lua_State*, int, record&, Args&&... args) {
			// Fuck you too, VC++
			return R { std::forward<Args>(args)... };
		}

		template <std::size_t I, std::size_t... Ix, typename... Args>
		static R apply(std::index_sequence<I, Ix...>, lua_State* L, int index, record& tracking, Args&&... args) {
			// Fuck you too, VC++
			typedef std::tuple_element_t<I, std::tuple<Tn...>> T;
			return apply(std::index_sequence<Ix...>(), L, index, tracking, std::forward<Args>(args)..., stack::get<T>(L, index + tracking.used, tracking));
		}

		static R get(lua_State* L, int index, record& tracking) {
			return apply(std::make_index_sequence<sizeof...(Tn)>(), L, index, tracking);
		}
	};

	template <typename A, typename B>
	struct unqualified_getter<std::pair<A, B>> {
		static decltype(auto) get(lua_State* L, int index, record& tracking) {
			return std::pair<decltype(stack::get<A>(L, index)), decltype(stack::get<B>(L, index))> { stack::get<A>(L, index, tracking),
				stack::get<B>(L, index + tracking.used, tracking) };
		}
	};

#if SOL_IS_ON(SOL_STD_VARIANT)

	template <typename... Tn>
	struct unqualified_getter<std::variant<Tn...>> {
		using V = std::variant<Tn...>;

		static V get_one(std::integral_constant<std::size_t, std::variant_size_v<V>>, lua_State* L, int index, record& tracking) {
			(void)L;
			(void)index;
			(void)tracking;
			if constexpr (std::variant_size_v<V> == 0) {
				return V();
			}
			else {
				// using T = std::variant_alternative_t<0, V>;
				std::abort();
				// return V(std::in_place_index<0>, stack::get<T>(L, index, tracking));
			}
		}

		template <std::size_t I>
		static V get_one(std::integral_constant<std::size_t, I>, lua_State* L, int index, record& tracking) {
			typedef std::variant_alternative_t<I, V> T;
			record temp_tracking = tracking;
			if (stack::check<T>(L, index, &no_panic, temp_tracking)) {
				tracking = temp_tracking;
				return V(std::in_place_index<I>, stack::get<T>(L, index));
			}
			return get_one(std::integral_constant<std::size_t, I + 1>(), L, index, tracking);
		}

		static V get(lua_State* L, int index, record& tracking) {
			return get_one(std::integral_constant<std::size_t, 0>(), L, index, tracking);
		}
	};
#endif // variant

}} // namespace sol::stack

#endif // SOL_STACK_UNQUALIFIED_GET_HPP
