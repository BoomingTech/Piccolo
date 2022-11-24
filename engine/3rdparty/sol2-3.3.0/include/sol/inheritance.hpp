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

#ifndef SOL_INHERITANCE_HPP
#define SOL_INHERITANCE_HPP

#include <sol/types.hpp>
#include <sol/usertype_traits.hpp>
#include <sol/unique_usertype_traits.hpp>

namespace sol {
	template <typename... Args>
	struct base_list { };
	template <typename... Args>
	using bases = base_list<Args...>;

	typedef bases<> base_classes_tag;
	const auto base_classes = base_classes_tag();

	template <typename... Args>
	struct is_to_stringable<base_list<Args...>> : std::false_type { };

	namespace detail {

		inline decltype(auto) base_class_check_key() {
			static const auto& key = "class_check";
			return key;
		}

		inline decltype(auto) base_class_cast_key() {
			static const auto& key = "class_cast";
			return key;
		}

		inline decltype(auto) base_class_index_propogation_key() {
			static const auto& key = u8"\xF0\x9F\x8C\xB2.index";
			return key;
		}

		inline decltype(auto) base_class_new_index_propogation_key() {
			static const auto& key = u8"\xF0\x9F\x8C\xB2.new_index";
			return key;
		}

		template <typename T>
		struct inheritance {
			typedef typename base<T>::type bases_t;

			static bool type_check_bases(types<>, const string_view&) {
				return false;
			}

			template <typename Base, typename... Args>
			static bool type_check_bases(types<Base, Args...>, const string_view& ti) {
				return ti == usertype_traits<Base>::qualified_name() || type_check_bases(types<Args...>(), ti);
			}

			static bool type_check(const string_view& ti) {
				return ti == usertype_traits<T>::qualified_name() || type_check_bases(bases_t(), ti);
			}

			template <typename... Bases>
			static bool type_check_with(const string_view& ti) {
				return ti == usertype_traits<T>::qualified_name() || type_check_bases(types<Bases...>(), ti);
			}

			static void* type_cast_bases(types<>, T*, const string_view&) {
				return nullptr;
			}

			template <typename Base, typename... Args>
			static void* type_cast_bases(types<Base, Args...>, T* data, const string_view& ti) {
				// Make sure to convert to T first, and then dynamic cast to the proper type
				return ti != usertype_traits<Base>::qualified_name() ? type_cast_bases(types<Args...>(), data, ti)
				                                                     : static_cast<void*>(static_cast<Base*>(data));
			}

			static void* type_cast(void* voiddata, const string_view& ti) {
				T* data = static_cast<T*>(voiddata);
				return static_cast<void*>(ti != usertype_traits<T>::qualified_name() ? type_cast_bases(bases_t(), data, ti) : data);
			}

			template <typename... Bases>
			static void* type_cast_with(void* voiddata, const string_view& ti) {
				T* data = static_cast<T*>(voiddata);
				return static_cast<void*>(ti != usertype_traits<T>::qualified_name() ? type_cast_bases(types<Bases...>(), data, ti) : data);
			}

			template <typename U>
			static bool type_unique_cast_bases(types<>, void*, void*, const string_view&) {
				return 0;
			}

			template <typename U, typename Base, typename... Args>
			static int type_unique_cast_bases(types<Base, Args...>, void* source_data, void* target_data, const string_view& ti) {
				using uu_traits = unique_usertype_traits<U>;
				using base_ptr = typename uu_traits::template rebind_actual_type<Base>;
				string_view base_ti = usertype_traits<Base>::qualified_name();
				if (base_ti == ti) {
					if (target_data != nullptr) {
						U* source = static_cast<U*>(source_data);
						base_ptr* target = static_cast<base_ptr*>(target_data);
						// perform proper derived -> base conversion
						*target = *source;
					}
					return 2;
				}
				return type_unique_cast_bases<U>(types<Args...>(), source_data, target_data, ti);
			}

			template <typename U>
			static int type_unique_cast(void* source_data, void* target_data, const string_view& ti, const string_view& rebind_ti) {
				if constexpr (is_actual_type_rebindable_for_v<U>) {
					using rebound_actual_type = unique_usertype_rebind_actual_t<U>;
					using maybe_bases_or_empty = meta::conditional_t<std::is_void_v<rebound_actual_type>, types<>, bases_t>;
					string_view this_rebind_ti = usertype_traits<rebound_actual_type>::qualified_name();
					if (rebind_ti != this_rebind_ti) {
						// this is not even of the same unique type
						return 0;
					}
					string_view this_ti = usertype_traits<T>::qualified_name();
					if (ti == this_ti) {
						// direct match, return 1
						return 1;
					}
					return type_unique_cast_bases<U>(maybe_bases_or_empty(), source_data, target_data, ti);
				}
				else {
					(void)rebind_ti;
					string_view this_ti = usertype_traits<T>::qualified_name();
					if (ti == this_ti) {
						// direct match, return 1
						return 1;
					}
					return type_unique_cast_bases<U>(types<>(), source_data, target_data, ti);
				}
			}

			template <typename U, typename... Bases>
			static int type_unique_cast_with(void* source_data, void* target_data, const string_view& ti, const string_view& rebind_ti) {
				using uc_bases_t = types<Bases...>;
				if constexpr (is_actual_type_rebindable_for_v<U>) {
					using rebound_actual_type = unique_usertype_rebind_actual_t<U>;
					using cond_bases_t = meta::conditional_t<std::is_void_v<rebound_actual_type>, types<>, uc_bases_t>;
					string_view this_rebind_ti = usertype_traits<rebound_actual_type>::qualified_name();
					if (rebind_ti != this_rebind_ti) {
						// this is not even of the same unique type
						return 0;
					}
					string_view this_ti = usertype_traits<T>::qualified_name();
					if (ti == this_ti) {
						// direct match, return 1
						return 1;
					}
					return type_unique_cast_bases<U>(cond_bases_t(), source_data, target_data, ti);
				}
				else {
					(void)rebind_ti;
					string_view this_ti = usertype_traits<T>::qualified_name();
					if (ti == this_ti) {
						// direct match, return 1
						return 1;
					}
					return type_unique_cast_bases<U>(types<>(), source_data, target_data, ti);
				}
			}
		};

		using inheritance_check_function = decltype(&inheritance<void>::type_check);
		using inheritance_cast_function = decltype(&inheritance<void>::type_cast);
		using inheritance_unique_cast_function = decltype(&inheritance<void>::type_unique_cast<void>);
	} // namespace detail
} // namespace sol

#endif // SOL_INHERITANCE_HPP
