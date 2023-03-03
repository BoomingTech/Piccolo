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

#ifndef SOL_USERTYPE_CORE_HPP
#define SOL_USERTYPE_CORE_HPP

#include <sol/wrapper.hpp>
#include <sol/stack.hpp>
#include <sol/types.hpp>
#include <sol/stack_reference.hpp>
#include <sol/usertype_traits.hpp>
#include <sol/inheritance.hpp>
#include <sol/raii.hpp>
#include <sol/deprecate.hpp>
#include <sol/object.hpp>
#include <sol/function_types.hpp>
#include <sol/usertype_container_launch.hpp>

#include <sstream>
#include <type_traits>

namespace sol {
	namespace u_detail {
		constexpr const lua_Integer toplevel_magic = static_cast<lua_Integer>(0xCCC2CCC1);

		constexpr const int environment_index = 1;
		constexpr const int usertype_storage_index = 2;
		constexpr const int usertype_storage_base_index = 3;
		constexpr const int exact_function_index = 4;
		constexpr const int magic_index = 5;

		constexpr const int simple_usertype_storage_index = 2;
		constexpr const int index_function_index = 3;
		constexpr const int new_index_function_index = 4;

		constexpr const int base_walking_failed_index = -32467;
		constexpr const int lookup_failed_index = -42469;

		enum class submetatable_type {
			// must be sequential
			value,
			reference,
			unique,
			const_reference,
			const_value,
			// must be LAST!
			named
		};

		inline auto make_string_view(string_view s) {
			return s;
		}

#if SOL_IS_ON(SOL_CHAR8_T)
		inline auto make_string_view(const char8_t* s) {
			return string_view(reinterpret_cast<const char*>(s));
		}
#endif

		inline auto make_string_view(call_construction) {
			return string_view(to_string(meta_function::call_function));
		}

		inline auto make_string_view(meta_function mf) {
			return string_view(to_string(mf));
		}

		inline auto make_string_view(base_classes_tag) {
			return string_view(detail::base_class_cast_key());
		}

		template <typename Arg>
		inline std::string make_string(Arg&& arg) {
			string_view s = make_string_view(arg);
			return std::string(s.data(), s.size());
		}

		inline int is_indexer(string_view s) {
			if (s == to_string(meta_function::index)) {
				return 1;
			}
			else if (s == to_string(meta_function::new_index)) {
				return 2;
			}
			return 0;
		}

		inline int is_indexer(meta_function mf) {
			if (mf == meta_function::index) {
				return 1;
			}
			else if (mf == meta_function::new_index) {
				return 2;
			}
			return 0;
		}

		inline int is_indexer(call_construction) {
			return 0;
		}
	} // namespace u_detail

	namespace detail {

		template <typename T, typename IFx, typename Fx>
		inline void insert_default_registrations(IFx&& ifx, Fx&& fx) {
			(void)ifx;
			(void)fx;
			if constexpr (is_automagical<T>::value) {
				if (fx(meta_function::less_than)) {
					if constexpr (meta::supports_op_less<T>::value) {
						lua_CFunction f = &comparsion_operator_wrap<T, std::less<>>;
						ifx(meta_function::less_than, f);
					}
				}
				if (fx(meta_function::less_than_or_equal_to)) {
					if constexpr (meta::supports_op_less_equal<T>::value) {
						lua_CFunction f = &comparsion_operator_wrap<T, std::less_equal<>>;
						ifx(meta_function::less_than_or_equal_to, f);
					}
				}
				if (fx(meta_function::equal_to)) {
					if constexpr (meta::supports_op_equal<T>::value) {
						lua_CFunction f = &comparsion_operator_wrap<T, std::equal_to<>>;
						ifx(meta_function::equal_to, f);
					}
					else {
						lua_CFunction f = &comparsion_operator_wrap<T, no_comp>;
						ifx(meta_function::equal_to, f);
					}
				}
				if (fx(meta_function::pairs)) {
					ifx(meta_function::pairs, &container_detail::u_c_launch<as_container_t<T>>::pairs_call);
				}
				if (fx(meta_function::length)) {
					if constexpr (meta::has_size<const T>::value || meta::has_size<T>::value) {
						auto f = &default_size<T>;
						ifx(meta_function::length, f);
					}
				}
				if (fx(meta_function::to_string)) {
					if constexpr (is_to_stringable_v<T>) {
						if constexpr (!meta::is_probably_stateless_lambda_v<T> && !std::is_member_pointer_v<T>) {
							auto f = &detail::static_trampoline<&default_to_string<T>>;
							ifx(meta_function::to_string, f);
						}
					}
				}
				if (fx(meta_function::call_function)) {
					if constexpr (is_callable_v<T>) {
						if constexpr (meta::call_operator_deducible_v<T>) {
							auto f = &c_call<decltype(&T::operator()), &T::operator()>;
							ifx(meta_function::call_function, f);
						}
					}
				}
			}
		}
	} // namespace detail

	namespace stack { namespace stack_detail {
		template <typename X>
		void set_undefined_methods_on(stack_reference t) {
			using T = std::remove_pointer_t<X>;

			lua_State* L = t.lua_state();

			t.push();

			detail::lua_reg_table l {};
			int index = 0;
			detail::indexed_insert insert_fx(l, index);
			detail::insert_default_registrations<T>(insert_fx, detail::property_always_true);
			if constexpr (!std::is_pointer_v<X>) {
				l[index] = luaL_Reg { to_string(meta_function::garbage_collect).c_str(), detail::make_destructor<T>() };
			}
			luaL_setfuncs(L, l, 0);

			// __type table
			lua_createtable(L, 0, 2);
			const std::string& name = detail::demangle<T>();
			lua_pushlstring(L, name.c_str(), name.size());
			lua_setfield(L, -2, "name");
			lua_CFunction is_func = &detail::is_check<T>;
			lua_pushcclosure(L, is_func, 0);
			lua_setfield(L, -2, "is");
			lua_setfield(L, t.stack_index(), to_string(meta_function::type).c_str());

			t.pop();
		}
	}} // namespace stack::stack_detail
} // namespace sol

#endif // SOL_USERTYPE_CORE_HPP
