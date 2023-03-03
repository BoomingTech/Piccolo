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

#ifndef SOL_UNIQUE_USERTYPE_TRAITS_HPP
#define SOL_UNIQUE_USERTYPE_TRAITS_HPP

#include <sol/base_traits.hpp>
#include <sol/pointer_like.hpp>

#include <sol/forward.hpp>

#include <memory>

namespace sol {

	namespace detail {
		template <typename T>
		struct unique_fallback {
			using SOL_INTERNAL_UNSPECIALIZED_MARKER_ = int;
		};

		template <typename T>
		struct unique_fallback<std::shared_ptr<T>> {
		private:
			using pointer = typename std::pointer_traits<std::shared_ptr<T>>::element_type*;

		public:
			// rebind is non-void
			// if and only if unique usertype
			// is cast-capable
			template <typename X>
			using rebind_actual_type = std::shared_ptr<X>;

			static bool is_null(lua_State*, const std::shared_ptr<T>& p) noexcept {
				return p == nullptr;
			}

			static pointer get(lua_State*, const std::shared_ptr<T>& p) noexcept {
				return p.get();
			}
		};

		template <typename T, typename D>
		struct unique_fallback<std::unique_ptr<T, D>> {
		private:
			using pointer = typename std::unique_ptr<T, D>::pointer;

		public:
			static bool is_null(lua_State*, const std::unique_ptr<T, D>& p) noexcept {
				return p == nullptr;
			}

			static pointer get(lua_State*, const std::unique_ptr<T, D>& p) noexcept {
				return p.get();
			}
		};
	} // namespace detail

	namespace meta { namespace meta_detail {
		template <typename T, typename = void>
		struct unique_actual_type;

		template <typename T>
		struct unique_actual_type<T, meta::void_t<typename T::actual_type>> {
			using type = typename T::actual_type;
		};

		template <typename T, typename... Rest, template <typename...> class Templ>
		struct unique_actual_type<Templ<T, Rest...>> {
			using type = T;
		};

	}} // namespace meta::meta_detail

	template <typename T>
	using unique_usertype_actual_t = typename meta::meta_detail::unique_actual_type<unique_usertype_traits<T>>::type;

	namespace meta { namespace meta_detail {
		template <typename T>
		using value_test_t = decltype(T::value);

		template <typename T>
		using type_test_t = typename T::type;

		template <typename T>
		using type_element_type_t = typename T::element_type;

		template <typename T, typename = void>
		struct unique_element_type {
			using type = typename std::pointer_traits<typename unique_actual_type<T>::type>::element_type;
		};

		template <typename T>
		struct unique_element_type<T, std::enable_if_t<meta::is_detected_v<type_element_type_t, T>>> {
			using type = typename T::element_type;
		};

		template <typename T>
		struct unique_element_type<T, std::enable_if_t<meta::is_detected_v<type_test_t, T>>> {
			using type = typename T::type;
		};

		template <typename T, typename = void>
		struct unique_valid : std::integral_constant<bool, !has_internal_marker_v<T>> { };

		template <typename T>
		struct unique_valid<T, meta::void_t<decltype(T::value)>> : std::integral_constant<bool, T::value> { };
	}} // namespace meta::meta_detail

	template <typename T>
	using unique_usertype_element_t = typename meta::meta_detail::unique_element_type<unique_usertype_traits<T>>::type;

	template <typename T, typename Element = void>
	using unique_usertype_rebind_actual_t = typename unique_usertype_traits<T>::template rebind_actual_type<Element>;

	template <typename T>
	struct unique_usertype_traits : public detail::unique_fallback<T> { };

	template <typename T>
	struct is_unique_usertype : std::integral_constant<bool, meta::meta_detail::unique_valid<unique_usertype_traits<T>>::value> { };

	template <typename T>
	inline constexpr bool is_unique_usertype_v = is_unique_usertype<T>::value;

	namespace meta { namespace meta_detail {
		template <typename T>
		using adl_sol_lua_check_access_test_t
			= decltype(sol_lua_check_access(types<T>(), static_cast<lua_State*>(nullptr), -1, std::declval<stack::record&>()));

		template <typename T>
		inline constexpr bool is_adl_sol_lua_check_access_v = meta::is_detected_v<adl_sol_lua_check_access_test_t, T>;

		template <typename T>
		using unique_usertype_get_with_state_test_t
			= decltype(unique_usertype_traits<T>::get(static_cast<lua_State*>(nullptr), std::declval<unique_usertype_actual_t<T>>()));

		template <typename T>
		inline constexpr bool unique_usertype_get_with_state_v = meta::is_detected_v<unique_usertype_get_with_state_test_t, T>;

		template <typename T>
		using unique_usertype_is_null_with_state_test_t
			= decltype(unique_usertype_traits<T>::is_null(static_cast<lua_State*>(nullptr), std::declval<unique_usertype_actual_t<T>>()));

		template <typename T>
		inline constexpr bool unique_usertype_is_null_with_state_v = meta::is_detected_v<unique_usertype_is_null_with_state_test_t, T>;
	}} // namespace meta::meta_detail

	namespace detail {
		template <typename T>
		constexpr bool unique_is_null_noexcept() noexcept {
			if constexpr (meta::meta_detail::unique_usertype_is_null_with_state_v<std::remove_cv_t<T>>) {
				return noexcept(
				     unique_usertype_traits<T>::is_null(static_cast<lua_State*>(nullptr), std::declval<unique_usertype_actual_t<std::remove_cv_t<T>>>()));
			}
			else {
				return noexcept(unique_usertype_traits<T>::is_null(std::declval<unique_usertype_actual_t<std::remove_cv_t<T>>>()));
			}
		}

		template <typename T>
		bool unique_is_null(lua_State* L_, T& value_) noexcept(unique_is_null_noexcept<std::remove_cv_t<T>>()) {
			using Tu = std::remove_cv_t<T>;
			if constexpr (meta::meta_detail::unique_usertype_is_null_with_state_v<Tu>) {
				return unique_usertype_traits<Tu>::is_null(L_, value_);
			}
			else {
				return unique_usertype_traits<Tu>::is_null(value_);
			}
		}

		template <typename T>
		constexpr bool unique_get_noexcept() noexcept {
			if constexpr (meta::meta_detail::unique_usertype_get_with_state_v<std::remove_cv_t<T>>) {
				return noexcept(
				     unique_usertype_traits<T>::get(static_cast<lua_State*>(nullptr), std::declval<unique_usertype_actual_t<std::remove_cv_t<T>>>()));
			}
			else {
				return noexcept(unique_usertype_traits<T>::get(std::declval<unique_usertype_actual_t<std::remove_cv_t<T>>>()));
			}
		}

		template <typename T>
		auto unique_get(lua_State* L_, T& value_) noexcept(unique_get_noexcept<std::remove_cv_t<T>>()) {
			using Tu = std::remove_cv_t<T>;
			if constexpr (meta::meta_detail::unique_usertype_get_with_state_v<Tu>) {
				return unique_usertype_traits<Tu>::get(L_, value_);
			}
			else {
				return unique_usertype_traits<Tu>::get(value_);
			}
		}
	} // namespace detail

	namespace meta { namespace meta_detail {
		template <typename T, typename Element = void>
		using is_rebind_actual_type_test_t = typename T::template rebind_actual_type<Element>;

		template <typename T, typename Element = void>
		using is_rebind_actual_type = meta::is_detected<is_rebind_actual_type_test_t, T, Element>;

		template <typename T, typename Element = void>
		inline constexpr bool is_rebind_actual_type_v = is_rebind_actual_type<T, Element>::value;

		template <typename T, typename Element, bool = is_rebind_actual_type_v<T, Element>>
		struct is_actual_type_rebindable_for_test : std::false_type { };

		template <typename T, typename Element>
		struct is_actual_type_rebindable_for_test<T, Element, true>
		: std::integral_constant<bool, !std::is_void_v<typename T::template rebind_actual_type<Element>>> { };
	}} // namespace meta::meta_detail

	template <typename T, typename Element = void>
	using is_actual_type_rebindable_for = typename meta::meta_detail::is_actual_type_rebindable_for_test<unique_usertype_traits<T>, Element>::type;

	template <typename T, typename Element = void>
	inline constexpr bool is_actual_type_rebindable_for_v = is_actual_type_rebindable_for<T, Element>::value;

} // namespace sol

#endif // SOL_UNIQUE_USERTYPE_TRAITS_HPP
