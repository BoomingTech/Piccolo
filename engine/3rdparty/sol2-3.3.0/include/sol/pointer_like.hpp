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

#ifndef SOL_POINTER_LIKE_HPP
#define SOL_POINTER_LIKE_HPP

#include <sol/base_traits.hpp>

#include <utility>
#include <type_traits>
#include <memory>

namespace sol {

	namespace meta {
		namespace meta_detail {
			template <typename T>
			using is_dereferenceable_test = decltype(*std::declval<T>());

			template <typename T>
			using is_explicitly_dereferenceable_test = decltype(std::declval<T>().operator*());
		} // namespace meta_detail

		template <typename T>
		using is_pointer_like = std::integral_constant<bool,
		     !std::is_array_v<T> && (std::is_pointer_v<T> || is_detected_v<meta_detail::is_explicitly_dereferenceable_test, T>)>;

		template <typename T>
		constexpr inline bool is_pointer_like_v = is_pointer_like<T>::value;
	} // namespace meta

	namespace detail {

		template <typename T>
		auto unwrap(T&& item) -> decltype(std::forward<T>(item)) {
			return std::forward<T>(item);
		}

		template <typename T>
		T& unwrap(std::reference_wrapper<T> arg) {
			return arg.get();
		}

		template <typename T>
		inline decltype(auto) deref(T&& item) {
			using Tu = meta::unqualified_t<T>;
			if constexpr (meta::is_pointer_like_v<Tu>) {
				return *std::forward<T>(item);
			}
			else {
				return std::forward<T>(item);
			}
		}

		template <typename T>
		inline decltype(auto) deref_move_only(T&& item) {
			using Tu = meta::unqualified_t<T>;
			if constexpr (meta::is_pointer_like_v<Tu> && !std::is_pointer_v<Tu> && !std::is_copy_constructible_v<Tu>) {
				return *std::forward<T>(item);
			}
			else {
				return std::forward<T>(item);
			}
		}

		template <typename T>
		inline T* ptr(T& val) {
			return std::addressof(val);
		}

		template <typename T>
		inline T* ptr(std::reference_wrapper<T> val) {
			return std::addressof(val.get());
		}

		template <typename T>
		inline T* ptr(T* val) {
			return val;
		}
	} // namespace detail
} // namespace sol

#endif // SOL_POINTER_LIKE_HPP
