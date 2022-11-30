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

#ifndef SOL_PROPERTY_HPP
#define SOL_PROPERTY_HPP

#include <sol/types.hpp>
#include <sol/ebco.hpp>
#include <type_traits>
#include <utility>

namespace sol {
	namespace detail {
		struct no_prop { };
	} // namespace detail

	template <typename R, typename W>
	struct property_wrapper : detail::ebco<R, 0>, detail::ebco<W, 1> {
	private:
		using read_base_t = detail::ebco<R, 0>;
		using write_base_t = detail::ebco<W, 1>;

	public:
		template <typename Rx, typename Wx>
		property_wrapper(Rx&& r, Wx&& w) : read_base_t(std::forward<Rx>(r)), write_base_t(std::forward<Wx>(w)) {
		}

		W& write() {
			return write_base_t::value();
		}

		const W& write() const {
			return write_base_t::value();
		}

		R& read() {
			return read_base_t::value();
		}

		const R& read() const {
			return read_base_t::value();
		}
	};

	template <typename F, typename G>
	inline decltype(auto) property(F&& f, G&& g) {
		typedef lua_bind_traits<meta::unqualified_t<F>> left_traits;
		typedef lua_bind_traits<meta::unqualified_t<G>> right_traits;
		if constexpr (left_traits::free_arity < right_traits::free_arity) {
			return property_wrapper<std::decay_t<F>, std::decay_t<G>>(std::forward<F>(f), std::forward<G>(g));
		}
		else {
			return property_wrapper<std::decay_t<G>, std::decay_t<F>>(std::forward<G>(g), std::forward<F>(f));
		}
	}

	template <typename F>
	inline decltype(auto) property(F&& f) {
		typedef lua_bind_traits<meta::unqualified_t<F>> left_traits;
		if constexpr (left_traits::free_arity < 2) {
			return property_wrapper<std::decay_t<F>, detail::no_prop>(std::forward<F>(f), detail::no_prop());
		}
		else {
			return property_wrapper<detail::no_prop, std::decay_t<F>>(detail::no_prop(), std::forward<F>(f));
		}
	}

	template <typename F>
	inline decltype(auto) readonly_property(F&& f) {
		return property_wrapper<std::decay_t<F>, detail::no_prop>(std::forward<F>(f), detail::no_prop());
	}

	template <typename F>
	inline decltype(auto) writeonly_property(F&& f) {
		return property_wrapper<detail::no_prop, std::decay_t<F>>(detail::no_prop(), std::forward<F>(f));
	}

	template <typename T>
	struct readonly_wrapper : detail::ebco<T> {
	private:
		using base_t = detail::ebco<T>;

	public:
		using base_t::base_t;

		operator T&() {
			return base_t::value();
		}
		operator const T&() const {
			return base_t::value();
		}
	};

	// Allow someone to make a member variable readonly (const)
	template <typename R, typename T>
	inline auto readonly(R T::*v) {
		return readonly_wrapper<meta::unqualified_t<decltype(v)>>(v);
	}

	template <typename T>
	struct var_wrapper : detail::ebco<T> {
	private:
		using base_t = detail::ebco<T>;

	public:
		using base_t::base_t;
	};

	template <typename V>
	inline auto var(V&& v) {
		typedef std::decay_t<V> T;
		return var_wrapper<T>(std::forward<V>(v));
	}

	namespace meta {
		template <typename T>
		using is_member_object = std::integral_constant<bool, std::is_member_object_pointer_v<T> || is_specialization_of_v<T, readonly_wrapper>>;

		template <typename T>
		inline constexpr bool is_member_object_v = is_member_object<T>::value;

		template <typename T>
		using is_member_object_or_function = std::integral_constant<bool, is_member_object_v<T> || std::is_member_pointer_v<T>>;

		template <typename T>
		inline constexpr bool is_member_object_or_function_v = is_member_object_or_function<T>::value;
	} // namespace meta

} // namespace sol

#endif // SOL_PROPERTY_HPP
