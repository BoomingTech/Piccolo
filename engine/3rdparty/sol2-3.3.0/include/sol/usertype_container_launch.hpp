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

#ifndef SOL_USERTYPE_CONTAINER_LAUNCH_HPP
#define SOL_USERTYPE_CONTAINER_LAUNCH_HPP

#include <sol/stack.hpp>
#include <sol/usertype_container.hpp>

#include <unordered_map>

namespace sol {

	namespace container_detail {
		template <typename X>
		struct u_c_launch {
			using T = std::remove_pointer_t<meta::unqualified_t<X>>;
			using uc = usertype_container<T>;
			using default_uc = usertype_container_default<T>;

			static inline int real_index_get_traits(std::true_type, lua_State* L) {
				return uc::index_get(L);
			}

			static inline int real_index_get_traits(std::false_type, lua_State* L) {
				return default_uc::index_get(L);
			}

			static inline int real_index_call(lua_State* L) {
				static const std::unordered_map<string_view, lua_CFunction> calls {
					{ "at", &real_at_call },
					{ "get", &real_get_call },
					{ "set", &real_set_call },
					{ "size", &real_length_call },
					{ "add", &real_add_call },
					{ "empty", &real_empty_call },
					{ "insert", &real_insert_call },
					{ "clear", &real_clear_call },
					{ "find", &real_find_call },
					{ "index_of", &real_index_of_call },
					{ "erase", &real_erase_call },
					{ "pairs", &pairs_call },
					{ "next", &next_call },
				};
				auto maybenameview = stack::unqualified_check_get<string_view>(L, 2);
				if (maybenameview) {
					const string_view& name = *maybenameview;
					auto it = calls.find(name);
					if (it != calls.cend()) {
						return stack::push(L, it->second);
					}
				}
				return real_index_get_traits(container_detail::has_traits_index_get<uc>(), L);
			}

			static inline int real_at_traits(std::true_type, lua_State* L) {
				return uc::at(L);
			}

			static inline int real_at_traits(std::false_type, lua_State* L) {
				return default_uc::at(L);
			}

			static inline int real_at_call(lua_State* L) {
				return real_at_traits(container_detail::has_traits_at<uc>(), L);
			}

			static inline int real_get_traits(std::true_type, lua_State* L) {
				return uc::get(L);
			}

			static inline int real_get_traits(std::false_type, lua_State* L) {
				return default_uc::get(L);
			}

			static inline int real_get_call(lua_State* L) {
				return real_get_traits(container_detail::has_traits_get<uc>(), L);
			}

			static inline int real_set_traits(std::true_type, lua_State* L) {
				return uc::set(L);
			}

			static inline int real_set_traits(std::false_type, lua_State* L) {
				return default_uc::set(L);
			}

			static inline int real_set_call(lua_State* L) {
				return real_set_traits(container_detail::has_traits_set<uc>(), L);
			}

			static inline int real_index_set_traits(std::true_type, lua_State* L) {
				return uc::index_set(L);
			}

			static inline int real_index_set_traits(std::false_type, lua_State* L) {
				return default_uc::index_set(L);
			}

			static inline int real_new_index_call(lua_State* L) {
				return real_index_set_traits(container_detail::has_traits_index_set<uc>(), L);
			}

			static inline int real_pairs_traits(std::true_type, lua_State* L) {
				return uc::pairs(L);
			}

			static inline int real_pairs_traits(std::false_type, lua_State* L) {
				return default_uc::pairs(L);
			}

			static inline int real_pairs_call(lua_State* L) {
				return real_pairs_traits(container_detail::has_traits_pairs<uc>(), L);
			}

			static inline int real_ipairs_traits(std::true_type, lua_State* L) {
				return uc::ipairs(L);
			}

			static inline int real_ipairs_traits(std::false_type, lua_State* L) {
				return default_uc::ipairs(L);
			}

			static inline int real_ipairs_call(lua_State* L) {
				return real_ipairs_traits(container_detail::has_traits_ipairs<uc>(), L);
			}

			static inline int real_next_traits(std::true_type, lua_State* L) {
				return uc::next(L);
			}

			static inline int real_next_traits(std::false_type, lua_State* L) {
				return default_uc::next(L);
			}

			static inline int real_next_call(lua_State* L) {
				return real_next_traits(container_detail::has_traits_next<uc>(), L);
			}

			static inline int real_size_traits(std::true_type, lua_State* L) {
				return uc::size(L);
			}

			static inline int real_size_traits(std::false_type, lua_State* L) {
				return default_uc::size(L);
			}

			static inline int real_length_call(lua_State* L) {
				return real_size_traits(container_detail::has_traits_size<uc>(), L);
			}

			static inline int real_add_traits(std::true_type, lua_State* L) {
				return uc::add(L);
			}

			static inline int real_add_traits(std::false_type, lua_State* L) {
				return default_uc::add(L);
			}

			static inline int real_add_call(lua_State* L) {
				return real_add_traits(container_detail::has_traits_add<uc>(), L);
			}

			static inline int real_insert_traits(std::true_type, lua_State* L) {
				return uc::insert(L);
			}

			static inline int real_insert_traits(std::false_type, lua_State* L) {
				return default_uc::insert(L);
			}

			static inline int real_insert_call(lua_State* L) {
				return real_insert_traits(container_detail::has_traits_insert<uc>(), L);
			}

			static inline int real_clear_traits(std::true_type, lua_State* L) {
				return uc::clear(L);
			}

			static inline int real_clear_traits(std::false_type, lua_State* L) {
				return default_uc::clear(L);
			}

			static inline int real_clear_call(lua_State* L) {
				return real_clear_traits(container_detail::has_traits_clear<uc>(), L);
			}

			static inline int real_empty_traits(std::true_type, lua_State* L) {
				return uc::empty(L);
			}

			static inline int real_empty_traits(std::false_type, lua_State* L) {
				return default_uc::empty(L);
			}

			static inline int real_empty_call(lua_State* L) {
				return real_empty_traits(container_detail::has_traits_empty<uc>(), L);
			}

			static inline int real_erase_traits(std::true_type, lua_State* L) {
				return uc::erase(L);
			}

			static inline int real_erase_traits(std::false_type, lua_State* L) {
				return default_uc::erase(L);
			}

			static inline int real_erase_call(lua_State* L) {
				return real_erase_traits(container_detail::has_traits_erase<uc>(), L);
			}

			static inline int real_find_traits(std::true_type, lua_State* L) {
				return uc::find(L);
			}

			static inline int real_find_traits(std::false_type, lua_State* L) {
				return default_uc::find(L);
			}

			static inline int real_find_call(lua_State* L) {
				return real_find_traits(container_detail::has_traits_find<uc>(), L);
			}

			static inline int real_index_of_call(lua_State* L) {
				if constexpr (container_detail::has_traits_index_of<uc>()) {
					return uc::index_of(L);
				}
				else {
					return default_uc::index_of(L);
				}
			}

			static inline int add_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_add_call), (&real_add_call)>(L);
			}

			static inline int erase_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_erase_call), (&real_erase_call)>(L);
			}

			static inline int insert_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_insert_call), (&real_insert_call)>(L);
			}

			static inline int clear_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_clear_call), (&real_clear_call)>(L);
			}

			static inline int empty_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_empty_call), (&real_empty_call)>(L);
			}

			static inline int find_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_find_call), (&real_find_call)>(L);
			}

			static inline int index_of_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_index_of_call), (&real_index_of_call)>(L);
			}

			static inline int length_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_length_call), (&real_length_call)>(L);
			}

			static inline int pairs_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_pairs_call), (&real_pairs_call)>(L);
			}

			static inline int ipairs_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_ipairs_call), (&real_ipairs_call)>(L);
			}

			static inline int next_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_next_call), (&real_next_call)>(L);
			}

			static inline int at_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_at_call), (&real_at_call)>(L);
			}

			static inline int get_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_get_call), (&real_get_call)>(L);
			}

			static inline int set_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_set_call), (&real_set_call)>(L);
			}

			static inline int index_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_index_call), (&real_index_call)>(L);
			}

			static inline int new_index_call(lua_State* L) {
				return detail::typed_static_trampoline<decltype(&real_new_index_call), (&real_new_index_call)>(L);
			}
		};
	} // namespace container_detail

	namespace stack {
		namespace stack_detail {
			template <typename T, bool is_shim = false>
			struct metatable_setup {
				lua_State* L;

				metatable_setup(lua_State* L) : L(L) {
				}

				void operator()() {
					using meta_usertype_container
					     = container_detail::u_c_launch<meta::conditional_t<is_shim, as_container_t<std::remove_pointer_t<T>>, std::remove_pointer_t<T>>>;
					static const char* metakey
					     = is_shim ? &usertype_traits<as_container_t<std::remove_pointer_t<T>>>::metatable()[0] : &usertype_traits<T>::metatable()[0];
					static const std::array<luaL_Reg, 20> reg = { {
						// clang-format off
						{ "__pairs", &meta_usertype_container::pairs_call },
						{ "__ipairs", &meta_usertype_container::ipairs_call },
						{ "__len", &meta_usertype_container::length_call },
						{ "__index", &meta_usertype_container::index_call },
						{ "__newindex", &meta_usertype_container::new_index_call },
						{ "pairs", &meta_usertype_container::pairs_call },
						{ "next", &meta_usertype_container::next_call },
						{ "at", &meta_usertype_container::at_call },
						{ "get", &meta_usertype_container::get_call },
						{ "set", &meta_usertype_container::set_call },
						{ "size", &meta_usertype_container::length_call },
						{ "empty", &meta_usertype_container::empty_call },
						{ "clear", &meta_usertype_container::clear_call },
						{ "insert", &meta_usertype_container::insert_call },
						{ "add", &meta_usertype_container::add_call },
						{ "find", &meta_usertype_container::find_call },
						{ "index_of", &meta_usertype_container::index_of_call },
						{ "erase", &meta_usertype_container::erase_call },
						std::is_pointer<T>::value ? luaL_Reg{ nullptr, nullptr } : luaL_Reg{ "__gc", &detail::usertype_alloc_destroy<T> },
						{ nullptr, nullptr }
						// clang-format on 
					} };

					if (luaL_newmetatable(L, metakey) == 1) {
						luaL_setfuncs(L, reg.data(), 0);
					}
					lua_setmetatable(L, -2);
				}
			};
		} // namespace stack_detail

		template <typename T>
		struct unqualified_pusher<as_container_t<T>> {
			using C = meta::unqualified_t<T>;

			static int push_lvalue(std::true_type, lua_State* L, const C& cont) {
				stack_detail::metatable_setup<C*, true> fx(L);
				return stack::push<detail::as_pointer_tag<const C>>(L, detail::with_function_tag(), fx, detail::ptr(cont));
			}

			static int push_lvalue(std::false_type, lua_State* L, const C& cont) {
				stack_detail::metatable_setup<C, true> fx(L);
				return stack::push<detail::as_value_tag<C>>(L, detail::with_function_tag(), fx, cont);
			}

			static int push_rvalue(std::true_type, lua_State* L, C&& cont) {
				stack_detail::metatable_setup<C, true> fx(L);
				return stack::push<detail::as_value_tag<C>>(L, detail::with_function_tag(), fx, std::move(cont));
			}

			static int push_rvalue(std::false_type, lua_State* L, const C& cont) {
				return push_lvalue(std::is_lvalue_reference<T>(), L, cont);
			}

			static int push(lua_State* L, const as_container_t<T>& as_cont) {
				return push_lvalue(std::is_lvalue_reference<T>(), L, as_cont.value());
			}

			static int push(lua_State* L, as_container_t<T>&& as_cont) {
				return push_rvalue(meta::all<std::is_rvalue_reference<T>, meta::neg<std::is_lvalue_reference<T>>>(), L, std::forward<T>(as_cont.value()));
			}
		};

		template <typename T>
		struct unqualified_pusher<as_container_t<T*>> {
			using C = std::add_pointer_t<meta::unqualified_t<std::remove_pointer_t<T>>>;

			static int push(lua_State* L, T* cont) {
				stack_detail::metatable_setup<C> fx(L);
				return stack::push<detail::as_pointer_tag<T>>(L, detail::with_function_tag(), fx, cont);
			}
		};

		template <typename T>
		struct unqualified_pusher<T, std::enable_if_t<is_container_v<T>>> {
			using C = T;

			template <typename... Args>
			static int push(lua_State* L, Args&&... args) {
				stack_detail::metatable_setup<C> fx(L);
				return stack::push<detail::as_value_tag<T>>(L, detail::with_function_tag(), fx, std::forward<Args>(args)...);
			}
		};

		template <typename T>
		struct unqualified_pusher<T*, std::enable_if_t<is_container_v<T>>> {
			using C = std::add_pointer_t<meta::unqualified_t<std::remove_pointer_t<T>>>;

			static int push(lua_State* L, T* cont) {
				stack_detail::metatable_setup<C> fx(L);
				return stack::push<detail::as_pointer_tag<T>>(L, detail::with_function_tag(), fx, cont);
			}
		};
	} // namespace stack

} // namespace sol

#endif // SOL_USERTYPE_CONTAINER_LAUNCH_HPP
