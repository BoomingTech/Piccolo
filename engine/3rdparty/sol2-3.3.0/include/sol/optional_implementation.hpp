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

// Taken from: TartanLlama/optional on Github, because
// holy shit am I done dealing with C++11 constexpr

///
// optional - An implementation of std::optional with extensions
// Written in 2017 by Simon Brand (@TartanLlama)
//
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
///

#ifndef SOL_TL_OPTIONAL_HPP
#define SOL_TL_OPTIONAL_HPP

#include <sol/version.hpp>

#include <sol/in_place.hpp>

#define SOL_TL_OPTIONAL_VERSION_MAJOR 0
#define SOL_TL_OPTIONAL_VERSION_MINOR 5

#include <exception>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>
#include <cstdlib>
#include <optional>

#if (defined(_MSC_VER) && _MSC_VER == 1900)
#define SOL_TL_OPTIONAL_MSVC2015
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 && !defined(__clang__))
#define SOL_TL_OPTIONAL_GCC49
#endif

#if (defined(__GNUC__) && __GNUC__ == 5 && __GNUC_MINOR__ <= 4 && !defined(__clang__))
#define SOL_TL_OPTIONAL_GCC54
#endif

#if (defined(__GNUC__) && __GNUC__ == 5 && __GNUC_MINOR__ <= 5 && !defined(__clang__))
#define SOL_TL_OPTIONAL_GCC55
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 && !defined(__clang__))
// GCC < 5 doesn't support overloading on const&& for member functions
#define SOL_TL_OPTIONAL_NO_CONSTRR

// GCC < 5 doesn't support some standard C++11 type traits
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T) std::has_trivial_copy_constructor<T>::value
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_ASSIGNABLE(T) std::has_trivial_copy_assign<T>::value

// This one will be different for GCC 5.7 if it's ever supported
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_DESTRUCTIBLE(T) std::is_trivially_destructible<T>::value

// GCC 5 < v < 8 has a bug in is_trivially_copy_constructible which breaks std::vector
// for non-copyable types
#elif (defined(__GNUC__) && __GNUC__ < 8 && !defined(__clang__))
#ifndef SOL_TL_GCC_LESS_8_TRIVIALLY_COPY_CONSTRUCTIBLE_MUTEX
#define SOL_TL_GCC_LESS_8_TRIVIALLY_COPY_CONSTRUCTIBLE_MUTEX
namespace sol { namespace detail {
	template <class T>
	struct is_trivially_copy_constructible : std::is_trivially_copy_constructible<T> { };
#ifdef _GLIBCXX_VECTOR
	template <class T, class A>
	struct is_trivially_copy_constructible<std::vector<T, A>> : std::is_trivially_copy_constructible<T> { };
#endif
}} // namespace sol::detail
#endif

#define SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T) sol::detail::is_trivially_copy_constructible<T>::value
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_ASSIGNABLE(T) std::is_trivially_copy_assignable<T>::value
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_DESTRUCTIBLE(T) std::is_trivially_destructible<T>::value
#else
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T) std::is_trivially_copy_constructible<T>::value
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_ASSIGNABLE(T) std::is_trivially_copy_assignable<T>::value
#define SOL_TL_OPTIONAL_IS_TRIVIALLY_DESTRUCTIBLE(T) std::is_trivially_destructible<T>::value
#endif

#if __cplusplus > 201103L
#define SOL_TL_OPTIONAL_CXX14
#endif

// constexpr implies const in C++11, not C++14
#if (__cplusplus == 201103L || defined(SOL_TL_OPTIONAL_MSVC2015) || defined(SOL_TL_OPTIONAL_GCC49))
/// \exclude
#define SOL_TL_OPTIONAL_11_CONSTEXPR
#else
   /// \exclude
#define SOL_TL_OPTIONAL_11_CONSTEXPR constexpr
#endif

namespace sol {
#ifndef SOL_TL_MONOSTATE_INPLACE_MUTEX
#define SOL_TL_MONOSTATE_INPLACE_MUTEX
	/// \brief Used to represent an optional with no data; essentially a bool
	class monostate { };
#endif

	template <class T>
	class optional;

	/// \exclude
	namespace detail {
#ifndef SOL_TL_TRAITS_MUTEX
#define SOL_TL_TRAITS_MUTEX
		// C++14-style aliases for brevity
		template <class T>
		using remove_const_t = typename std::remove_const<T>::type;
		template <class T>
		using remove_reference_t = typename std::remove_reference<T>::type;
		template <class T>
		using decay_t = typename std::decay<T>::type;
		template <bool E, class T = void>
		using enable_if_t = typename std::enable_if<E, T>::type;
		template <bool B, class T, class F>
		using conditional_t = typename std::conditional<B, T, F>::type;

		// std::conjunction from C++17
		template <class...>
		struct conjunction : std::true_type { };
		template <class B>
		struct conjunction<B> : B { };
		template <class B, class... Bs>
		struct conjunction<B, Bs...> : std::conditional<bool(B::value), conjunction<Bs...>, B>::type { };

#if defined(_LIBCPP_VERSION) && __cplusplus == 201103L
#define SOL_TL_OPTIONAL_LIBCXX_MEM_FN_WORKAROUND
#endif

// In C++11 mode, there's an issue in libc++'s std::mem_fn
// which results in a hard-error when using it in a noexcept expression
// in some cases. This is a check to workaround the common failing case.
#ifdef SOL_TL_OPTIONAL_LIBCXX_MEM_FN_WORKAROUND
		template <class T>
		struct is_pointer_to_non_const_member_func : std::false_type { };
		template <class T, class Ret, class... Args>
		struct is_pointer_to_non_const_member_func<Ret (T::*)(Args...)> : std::true_type { };
		template <class T, class Ret, class... Args>
		struct is_pointer_to_non_const_member_func<Ret (T::*)(Args...)&> : std::true_type { };
		template <class T, class Ret, class... Args>
		struct is_pointer_to_non_const_member_func<Ret (T::*)(Args...) &&> : std::true_type { };
		template <class T, class Ret, class... Args>
		struct is_pointer_to_non_const_member_func<Ret (T::*)(Args...) volatile> : std::true_type { };
		template <class T, class Ret, class... Args>
		struct is_pointer_to_non_const_member_func<Ret (T::*)(Args...) volatile&> : std::true_type { };
		template <class T, class Ret, class... Args>
		struct is_pointer_to_non_const_member_func<Ret (T::*)(Args...) volatile&&> : std::true_type { };

		template <class T>
		struct is_const_or_const_ref : std::false_type { };
		template <class T>
		struct is_const_or_const_ref<T const&> : std::true_type { };
		template <class T>
		struct is_const_or_const_ref<T const> : std::true_type { };
#endif

		// std::invoke from C++17
		// https://stackoverflow.com/questions/38288042/c11-14-invoke-workaround
		template <typename Fn, typename... Args,
#ifdef SOL_TL_OPTIONAL_LIBCXX_MEM_FN_WORKAROUND
		     typename = enable_if_t<!(is_pointer_to_non_const_member_func<Fn>::value && is_const_or_const_ref<Args...>::value)>,
#endif
		     typename = enable_if_t<std::is_member_pointer<decay_t<Fn>>::value>, int = 0>
		constexpr auto invoke(Fn&& f, Args&&... args) noexcept(noexcept(std::mem_fn(f)(std::forward<Args>(args)...)))
		     -> decltype(std::mem_fn(f)(std::forward<Args>(args)...)) {
			return std::mem_fn(f)(std::forward<Args>(args)...);
		}

		template <typename Fn, typename... Args, typename = enable_if_t<!std::is_member_pointer<decay_t<Fn>>::value>>
		constexpr auto invoke(Fn&& f, Args&&... args) noexcept(noexcept(std::forward<Fn>(f)(std::forward<Args>(args)...)))
		     -> decltype(std::forward<Fn>(f)(std::forward<Args>(args)...)) {
			return std::forward<Fn>(f)(std::forward<Args>(args)...);
		}

		// std::invoke_result from C++17
		template <class F, class, class... Us>
		struct invoke_result_impl;

		template <class F, class... Us>
		struct invoke_result_impl<F, decltype(detail::invoke(std::declval<F>(), std::declval<Us>()...), void()), Us...> {
			using type = decltype(detail::invoke(std::declval<F>(), std::declval<Us>()...));
		};

		template <class F, class... Us>
		using invoke_result = invoke_result_impl<F, void, Us...>;

		template <class F, class... Us>
		using invoke_result_t = typename invoke_result<F, Us...>::type;
#endif

		// std::void_t from C++17
		template <class...>
		struct voider {
			using type = void;
		};
		template <class... Ts>
		using void_t = typename voider<Ts...>::type;

		// Trait for checking if a type is a sol::optional
		template <class T>
		struct is_optional_impl : std::false_type { };
		template <class T>
		struct is_optional_impl<optional<T>> : std::true_type { };
		template <class T>
		using is_optional = is_optional_impl<decay_t<T>>;

		// Change void to sol::monostate
		template <class U>
		using fixup_void = conditional_t<std::is_void<U>::value, monostate, U>;

		template <class F, class U, class = invoke_result_t<F, U>>
		using get_map_return = optional<fixup_void<invoke_result_t<F, U>>>;

		// Check if invoking F for some Us returns void
		template <class F, class = void, class... U>
		struct returns_void_impl;
		template <class F, class... U>
		struct returns_void_impl<F, void_t<invoke_result_t<F, U...>>, U...> : std::is_void<invoke_result_t<F, U...>> { };
		template <class F, class... U>
		using returns_void = returns_void_impl<F, void, U...>;

		template <class T, class... U>
		using enable_if_ret_void = enable_if_t<returns_void<T&&, U...>::value>;

		template <class T, class... U>
		using disable_if_ret_void = enable_if_t<!returns_void<T&&, U...>::value>;

		template <class T, class U>
		using enable_forward_value = detail::enable_if_t<std::is_constructible<T, U&&>::value && !std::is_same<detail::decay_t<U>, in_place_t>::value
		     && !std::is_same<optional<T>, detail::decay_t<U>>::value>;

		template <class T, class U, class Other>
		using enable_from_other = detail::enable_if_t<std::is_constructible<T, Other>::value && !std::is_constructible<T, optional<U>&>::value
		     && !std::is_constructible<T, optional<U>&&>::value && !std::is_constructible<T, const optional<U>&>::value
		     && !std::is_constructible<T, const optional<U>&&>::value && !std::is_convertible<optional<U>&, T>::value
		     && !std::is_convertible<optional<U>&&, T>::value && !std::is_convertible<const optional<U>&, T>::value
		     && !std::is_convertible<const optional<U>&&, T>::value>;

		template <class T, class U>
		using enable_assign_forward = detail::enable_if_t<!std::is_same<optional<T>, detail::decay_t<U>>::value
		     && !detail::conjunction<std::is_scalar<T>, std::is_same<T, detail::decay_t<U>>>::value && std::is_constructible<T, U>::value
		     && std::is_assignable<T&, U>::value>;

		template <class T, class U, class Other>
		using enable_assign_from_other = detail::enable_if_t<std::is_constructible<T, Other>::value && std::is_assignable<T&, Other>::value
		     && !std::is_constructible<T, optional<U>&>::value && !std::is_constructible<T, optional<U>&&>::value
		     && !std::is_constructible<T, const optional<U>&>::value && !std::is_constructible<T, const optional<U>&&>::value
		     && !std::is_convertible<optional<U>&, T>::value && !std::is_convertible<optional<U>&&, T>::value
		     && !std::is_convertible<const optional<U>&, T>::value && !std::is_convertible<const optional<U>&&, T>::value
		     && !std::is_assignable<T&, optional<U>&>::value && !std::is_assignable<T&, optional<U>&&>::value
		     && !std::is_assignable<T&, const optional<U>&>::value && !std::is_assignable<T&, const optional<U>&&>::value>;

#ifdef _MSC_VER
		// TODO make a version which works with MSVC
		template <class T, class U = T>
		struct is_swappable : std::true_type { };

		template <class T, class U = T>
		struct is_nothrow_swappable : std::true_type { };
#else
		// https://stackoverflow.com/questions/26744589/what-is-a-proper-way-to-implement-is-swappable-to-test-for-the-swappable-concept
		namespace swap_adl_tests {
			// if swap ADL finds this then it would call std::swap otherwise (same
			// signature)
			struct tag { };

			template <class T>
			tag swap(T&, T&);
			template <class T, std::size_t N>
			tag swap(T (&a)[N], T (&b)[N]);

			// helper functions to test if an unqualified swap is possible, and if it
			// becomes std::swap
			template <class, class>
			std::false_type can_swap(...) noexcept(false);
			template <class T, class U, class = decltype(swap(std::declval<T&>(), std::declval<U&>()))>
			std::true_type can_swap(int) noexcept(noexcept(swap(std::declval<T&>(), std::declval<U&>())));

			template <class, class>
			std::false_type uses_std(...);
			template <class T, class U>
			std::is_same<decltype(swap(std::declval<T&>(), std::declval<U&>())), tag> uses_std(int);

			template <class T>
			struct is_std_swap_noexcept
			: std::integral_constant<bool, std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value> { };

			template <class T, std::size_t N>
			struct is_std_swap_noexcept<T[N]> : is_std_swap_noexcept<T> { };

			template <class T, class U>
			struct is_adl_swap_noexcept : std::integral_constant<bool, noexcept(can_swap<T, U>(0))> { };
		} // namespace swap_adl_tests

		template <class T, class U = T>
		struct is_swappable : std::integral_constant<bool,
		                           decltype(detail::swap_adl_tests::can_swap<T, U>(0))::value
		                                && (!decltype(detail::swap_adl_tests::uses_std<T, U>(0))::value
		                                     || (std::is_move_assignable<T>::value && std::is_move_constructible<T>::value))> { };

		template <class T, std::size_t N>
		struct is_swappable<T[N], T[N]> : std::integral_constant<bool,
		                                       decltype(detail::swap_adl_tests::can_swap<T[N], T[N]>(0))::value
		                                            && (!decltype(detail::swap_adl_tests::uses_std<T[N], T[N]>(0))::value || is_swappable<T, T>::value)> { };

		template <class T, class U = T>
		struct is_nothrow_swappable
		: std::integral_constant<bool,
		       is_swappable<T, U>::value
		            && ((decltype(detail::swap_adl_tests::uses_std<T, U>(0))::value&& detail::swap_adl_tests::is_std_swap_noexcept<T>::value)
		                 || (!decltype(detail::swap_adl_tests::uses_std<T, U>(0))::value&& detail::swap_adl_tests::is_adl_swap_noexcept<T, U>::value))> { };
#endif

		// The storage base manages the actual storage, and correctly propagates
		// trivial destroyion from T. This case is for when T is not trivially
		// destructible.
		template <class T, bool = ::std::is_trivially_destructible<T>::value>
		struct optional_storage_base {
			SOL_TL_OPTIONAL_11_CONSTEXPR optional_storage_base() noexcept : m_dummy(), m_has_value(false) {
			}

			template <class... U>
			SOL_TL_OPTIONAL_11_CONSTEXPR optional_storage_base(in_place_t, U&&... u) : m_value(std::forward<U>(u)...), m_has_value(true) {
			}

			~optional_storage_base() {
				if (m_has_value) {
					m_value.~T();
					m_has_value = false;
				}
			}

			struct dummy { };
			union {
				dummy m_dummy;
				T m_value;
			};

			bool m_has_value;
		};

		// This case is for when T is trivially destructible.
		template <class T>
		struct optional_storage_base<T, true> {
			SOL_TL_OPTIONAL_11_CONSTEXPR optional_storage_base() noexcept : m_dummy(), m_has_value(false) {
			}

			template <class... U>
			SOL_TL_OPTIONAL_11_CONSTEXPR optional_storage_base(in_place_t, U&&... u) : m_value(std::forward<U>(u)...), m_has_value(true) {
			}

			// No destructor, so this class is trivially destructible

			struct dummy { };
			union {
				dummy m_dummy;
				T m_value;
			};

			bool m_has_value = false;
		};

		// This base class provides some handy member functions which can be used in
		// further derived classes
		template <class T>
		struct optional_operations_base : optional_storage_base<T> {
			using optional_storage_base<T>::optional_storage_base;

			void hard_reset() noexcept {
				get().~T();
				this->m_has_value = false;
			}

			template <class... Args>
			void construct(Args&&... args) noexcept {
				new (std::addressof(this->m_value)) T(std::forward<Args>(args)...);
				this->m_has_value = true;
			}

			template <class Opt>
			void assign(Opt&& rhs) {
				if (this->has_value()) {
					if (rhs.has_value()) {
						this->m_value = std::forward<Opt>(rhs).get();
					}
					else {
						this->m_value.~T();
						this->m_has_value = false;
					}
				}

				else if (rhs.has_value()) {
					construct(std::forward<Opt>(rhs).get());
				}
			}

			bool has_value() const {
				return this->m_has_value;
			}

			SOL_TL_OPTIONAL_11_CONSTEXPR T& get() & {
				return this->m_value;
			}
			SOL_TL_OPTIONAL_11_CONSTEXPR const T& get() const& {
				return this->m_value;
			}
			SOL_TL_OPTIONAL_11_CONSTEXPR T&& get() && {
				return std::move(this->m_value);
			}
#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
			constexpr const T&& get() const&& {
				return std::move(this->m_value);
			}
#endif
		};

		// This class manages conditionally having a trivial copy constructor
		// This specialization is for when T is trivially copy constructible
		template <class T, bool = SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)>
		struct optional_copy_base : optional_operations_base<T> {
			using optional_operations_base<T>::optional_operations_base;
		};

		// This specialization is for when T is not trivially copy constructible
		template <class T>
		struct optional_copy_base<T, false> : optional_operations_base<T> {
			using base_t = optional_operations_base<T>;

			using base_t::base_t;

			optional_copy_base() = default;
			optional_copy_base(const optional_copy_base& rhs) : base_t() {
				if (rhs.has_value()) {
					this->construct(rhs.get());
				}
				else {
					this->m_has_value = false;
				}
			}

			optional_copy_base(optional_copy_base&& rhs) = default;
			optional_copy_base& operator=(const optional_copy_base& rhs) = default;
			optional_copy_base& operator=(optional_copy_base&& rhs) = default;
		};

// This class manages conditionally having a trivial move constructor
// Unfortunately there's no way to achieve this in GCC < 5 AFAIK, since it
// doesn't implement an analogue to std::is_trivially_move_constructible. We
// have to make do with a non-trivial move constructor even if T is trivially
// move constructible
#ifndef SOL_TL_OPTIONAL_GCC49
		template <class T, bool = std::is_trivially_move_constructible<T>::value>
		struct optional_move_base : optional_copy_base<T> {
			using optional_copy_base<T>::optional_copy_base;
		};
#else
		template <class T, bool = false>
		struct optional_move_base;
#endif
		template <class T>
		struct optional_move_base<T, false> : optional_copy_base<T> {
			using optional_copy_base<T>::optional_copy_base;

			optional_move_base() = default;
			optional_move_base(const optional_move_base& rhs) = default;

			optional_move_base(optional_move_base&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
				if (rhs.has_value()) {
					this->construct(std::move(rhs.get()));
				}
				else {
					this->m_has_value = false;
				}
			}
			optional_move_base& operator=(const optional_move_base& rhs) = default;
			optional_move_base& operator=(optional_move_base&& rhs) = default;
		};

		// This class manages conditionally having a trivial copy assignment operator
		template <class T,
		     bool = SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_ASSIGNABLE(T) && SOL_TL_OPTIONAL_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)
		          && SOL_TL_OPTIONAL_IS_TRIVIALLY_DESTRUCTIBLE(T)>
		struct optional_copy_assign_base : optional_move_base<T> {
			using optional_move_base<T>::optional_move_base;
		};

		template <class T>
		struct optional_copy_assign_base<T, false> : optional_move_base<T> {
			using optional_move_base<T>::optional_move_base;

			optional_copy_assign_base() = default;
			optional_copy_assign_base(const optional_copy_assign_base& rhs) = default;

			optional_copy_assign_base(optional_copy_assign_base&& rhs) = default;
			optional_copy_assign_base& operator=(const optional_copy_assign_base& rhs) {
				this->assign(rhs);
				return *this;
			}
			optional_copy_assign_base& operator=(optional_copy_assign_base&& rhs) = default;
		};

// This class manages conditionally having a trivial move assignment operator
// Unfortunately there's no way to achieve this in GCC < 5 AFAIK, since it
// doesn't implement an analogue to std::is_trivially_move_assignable. We have
// to make do with a non-trivial move assignment operator even if T is trivially
// move assignable
#ifndef SOL_TL_OPTIONAL_GCC49
		template <class T,
		     bool = std::is_trivially_destructible<T>::value&& std::is_trivially_move_constructible<T>::value&& std::is_trivially_move_assignable<T>::value>
		struct optional_move_assign_base : optional_copy_assign_base<T> {
			using optional_copy_assign_base<T>::optional_copy_assign_base;
		};
#else
		template <class T, bool = false>
		struct optional_move_assign_base;
#endif

		template <class T>
		struct optional_move_assign_base<T, false> : optional_copy_assign_base<T> {
			using optional_copy_assign_base<T>::optional_copy_assign_base;

			optional_move_assign_base() = default;
			optional_move_assign_base(const optional_move_assign_base& rhs) = default;

			optional_move_assign_base(optional_move_assign_base&& rhs) = default;

			optional_move_assign_base& operator=(const optional_move_assign_base& rhs) = default;

			optional_move_assign_base& operator=(optional_move_assign_base&& rhs) noexcept(
			     std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value) {
				this->assign(std::move(rhs));
				return *this;
			}
		};

		// optional_delete_ctor_base will conditionally delete copy and move
		// constructors depending on whether T is copy/move constructible
		template <class T, bool EnableCopy = std::is_copy_constructible<T>::value, bool EnableMove = std::is_move_constructible<T>::value>
		struct optional_delete_ctor_base {
			optional_delete_ctor_base() = default;
			optional_delete_ctor_base(const optional_delete_ctor_base&) = default;
			optional_delete_ctor_base(optional_delete_ctor_base&&) noexcept = default;
			optional_delete_ctor_base& operator=(const optional_delete_ctor_base&) = default;
			optional_delete_ctor_base& operator=(optional_delete_ctor_base&&) noexcept = default;
		};

		template <class T>
		struct optional_delete_ctor_base<T, true, false> {
			optional_delete_ctor_base() = default;
			optional_delete_ctor_base(const optional_delete_ctor_base&) = default;
			optional_delete_ctor_base(optional_delete_ctor_base&&) noexcept = delete;
			optional_delete_ctor_base& operator=(const optional_delete_ctor_base&) = default;
			optional_delete_ctor_base& operator=(optional_delete_ctor_base&&) noexcept = default;
		};

		template <class T>
		struct optional_delete_ctor_base<T, false, true> {
			optional_delete_ctor_base() = default;
			optional_delete_ctor_base(const optional_delete_ctor_base&) = delete;
			optional_delete_ctor_base(optional_delete_ctor_base&&) noexcept = default;
			optional_delete_ctor_base& operator=(const optional_delete_ctor_base&) = default;
			optional_delete_ctor_base& operator=(optional_delete_ctor_base&&) noexcept = default;
		};

		template <class T>
		struct optional_delete_ctor_base<T, false, false> {
			optional_delete_ctor_base() = default;
			optional_delete_ctor_base(const optional_delete_ctor_base&) = delete;
			optional_delete_ctor_base(optional_delete_ctor_base&&) noexcept = delete;
			optional_delete_ctor_base& operator=(const optional_delete_ctor_base&) = default;
			optional_delete_ctor_base& operator=(optional_delete_ctor_base&&) noexcept = default;
		};

		// optional_delete_assign_base will conditionally delete copy and move
		// constructors depending on whether T is copy/move constructible + assignable
		template <class T, bool EnableCopy = (std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value),
		     bool EnableMove = (std::is_move_constructible<T>::value && std::is_move_assignable<T>::value)>
		struct optional_delete_assign_base {
			optional_delete_assign_base() = default;
			optional_delete_assign_base(const optional_delete_assign_base&) = default;
			optional_delete_assign_base(optional_delete_assign_base&&) noexcept = default;
			optional_delete_assign_base& operator=(const optional_delete_assign_base&) = default;
			optional_delete_assign_base& operator=(optional_delete_assign_base&&) noexcept = default;
		};

		template <class T>
		struct optional_delete_assign_base<T, true, false> {
			optional_delete_assign_base() = default;
			optional_delete_assign_base(const optional_delete_assign_base&) = default;
			optional_delete_assign_base(optional_delete_assign_base&&) noexcept = default;
			optional_delete_assign_base& operator=(const optional_delete_assign_base&) = default;
			optional_delete_assign_base& operator=(optional_delete_assign_base&&) noexcept = delete;
		};

		template <class T>
		struct optional_delete_assign_base<T, false, true> {
			optional_delete_assign_base() = default;
			optional_delete_assign_base(const optional_delete_assign_base&) = default;
			optional_delete_assign_base(optional_delete_assign_base&&) noexcept = default;
			optional_delete_assign_base& operator=(const optional_delete_assign_base&) = delete;
			optional_delete_assign_base& operator=(optional_delete_assign_base&&) noexcept = default;
		};

		template <class T>
		struct optional_delete_assign_base<T, false, false> {
			optional_delete_assign_base() = default;
			optional_delete_assign_base(const optional_delete_assign_base&) = default;
			optional_delete_assign_base(optional_delete_assign_base&&) noexcept = default;
			optional_delete_assign_base& operator=(const optional_delete_assign_base&) = delete;
			optional_delete_assign_base& operator=(optional_delete_assign_base&&) noexcept = delete;
		};

	} // namespace detail

	/// \brief A tag type to represent an empty optional
	using nullopt_t = std::nullopt_t;

	/// \brief Represents an empty optional
	/// \synopsis static constexpr nullopt_t nullopt;
	///
	/// *Examples*:
	/// ```
	/// sol::optional<int> a = sol::nullopt;
	/// void foo (sol::optional<int>);
	/// foo(sol::nullopt); //pass an empty optional
	/// ```
	using std::nullopt;

	/// @brief An exception for when an optional is accessed through specific methods while it is not engaged.
	class bad_optional_access : public std::exception {
	public:
		/// @brief Default-constructs an optional exception.
		bad_optional_access() = default;
		/// @brief Returns a pointer to a null-terminated string containing the reason for the exception.
		const char* what() const noexcept override {
			return "Optional has no value";
		}
	};

	/// An optional object is an object that contains the storage for another
	/// object and manages the lifetime of this contained object, if any. The
	/// contained object may be initialized after the optional object has been
	/// initialized, and may be destroyed before the optional object has been
	/// destroyed. The initialization state of the contained object is tracked by
	/// the optional object.
	template <class T>
	class optional : private detail::optional_move_assign_base<T>,
	                 private detail::optional_delete_ctor_base<T>,
	                 private detail::optional_delete_assign_base<T> {
		using base = detail::optional_move_assign_base<T>;

		static_assert(!std::is_same<T, in_place_t>::value, "instantiation of optional with in_place_t is ill-formed");
		static_assert(!std::is_same<detail::decay_t<T>, nullopt_t>::value, "instantiation of optional with nullopt_t is ill-formed");

	public:
// The different versions for C++14 and 11 are needed because deduced return
// types are not SFINAE-safe. This provides better support for things like
// generic lambdas. C.f.
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0826r0.html
#if defined(SOL_TL_OPTIONAL_CXX14) && !defined(SOL_TL_OPTIONAL_GCC49) && !defined(SOL_TL_OPTIONAL_GCC54) && !defined(SOL_TL_OPTIONAL_GCC55)
		/// \group and_then
		/// Carries out some operation which returns an optional on the stored
		/// object if there is one. \requires `std::invoke(std::forward<F>(f),
		/// value())` returns a `std::optional<U>` for some `U`. \returns Let `U` be
		/// the result of `std::invoke(std::forward<F>(f), value())`. Returns a
		/// `std::optional<U>`. The return value is empty if `*this` is empty,
		/// otherwise the return value of `std::invoke(std::forward<F>(f), value())`
		/// is returned.
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto and_then(F&& f) & {
			using result = detail::invoke_result_t<F, T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto and_then(F&& f) && {
			using result = detail::invoke_result_t<F, T&&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &;
		template <class F>
		constexpr auto and_then(F&& f) const& {
			using result = detail::invoke_result_t<F, const T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &&;
		template <class F>
		constexpr auto and_then(F&& f) const&& {
			using result = detail::invoke_result_t<F, const T&&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : result(nullopt);
		}
#endif
#else
		/// \group and_then
		/// Carries out some operation which returns an optional on the stored
		/// object if there is one. \requires `std::invoke(std::forward<F>(f),
		/// value())` returns a `std::optional<U>` for some `U`.
		/// \returns Let `U` be the result of `std::invoke(std::forward<F>(f),
		/// value())`. Returns a `std::optional<U>`. The return value is empty if
		/// `*this` is empty, otherwise the return value of
		/// `std::invoke(std::forward<F>(f), value())` is returned.
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR detail::invoke_result_t<F, T&> and_then(F&& f) & {
			using result = detail::invoke_result_t<F, T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR detail::invoke_result_t<F, T&&> and_then(F&& f) && {
			using result = detail::invoke_result_t<F, T&&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &;
		template <class F>
		constexpr detail::invoke_result_t<F, const T&> and_then(F&& f) const& {
			using result = detail::invoke_result_t<F, const T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &&;
		template <class F>
		constexpr detail::invoke_result_t<F, const T&&> and_then(F&& f) const&& {
			using result = detail::invoke_result_t<F, const T&&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : result(nullopt);
		}
#endif
#endif

#if defined(SOL_TL_OPTIONAL_CXX14) && !defined(SOL_TL_OPTIONAL_GCC49) && !defined(SOL_TL_OPTIONAL_GCC54) && !defined(SOL_TL_OPTIONAL_GCC55)
		/// \brief Carries out some operation on the stored object if there is one.
		/// \returns Let `U` be the result of `std::invoke(std::forward<F>(f),
		/// value())`. Returns a `std::optional<U>`. The return value is empty if
		/// `*this` is empty, otherwise an `optional<U>` is constructed from the
		/// return value of `std::invoke(std::forward<F>(f), value())` and is
		/// returned.
		///
		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto map(F&& f) & {
			return optional_map_impl(*this, std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto map(F&& f) && {
			return optional_map_impl(std::move(*this), std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) const&;
		template <class F>
		constexpr auto map(F&& f) const& {
			return optional_map_impl(*this, std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) const&&;
		template <class F>
		constexpr auto map(F&& f) const&& {
			return optional_map_impl(std::move(*this), std::forward<F>(f));
		}
#else
		/// \brief Carries out some operation on the stored object if there is one.
		/// \returns Let `U` be the result of `std::invoke(std::forward<F>(f),
		/// value())`. Returns a `std::optional<U>`. The return value is empty if
		/// `*this` is empty, otherwise an `optional<U>` is constructed from the
		/// return value of `std::invoke(std::forward<F>(f), value())` and is
		/// returned.
		///
		/// \group map
		/// \synopsis template <class F> auto map(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR decltype(optional_map_impl(std::declval<optional&>(), std::declval<F&&>())) map(F&& f) & {
			return optional_map_impl(*this, std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> auto map(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR decltype(optional_map_impl(std::declval<optional&&>(), std::declval<F&&>())) map(F&& f) && {
			return optional_map_impl(std::move(*this), std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> auto map(F &&f) const&;
		template <class F>
		constexpr decltype(optional_map_impl(std::declval<const optional&>(), std::declval<F&&>())) map(F&& f) const& {
			return optional_map_impl(*this, std::forward<F>(f));
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group map
		/// \synopsis template <class F> auto map(F &&f) const&&;
		template <class F>
		constexpr decltype(optional_map_impl(std::declval<const optional&&>(), std::declval<F&&>())) map(F&& f) const&& {
			return optional_map_impl(std::move(*this), std::forward<F>(f));
		}
#endif
#endif

		/// \brief Calls `f` if the optional is empty
		/// \requires `std::invoke_result_t<F>` must be void or convertible to
		/// `optional<T>`.
		/// \effects If `*this` has a value, returns `*this`.
		/// Otherwise, if `f` returns `void`, calls `std::forward<F>(f)` and returns
		/// `std::nullopt`. Otherwise, returns `std::forward<F>(f)()`.
		///
		/// \group or_else
		/// \synopsis template <class F> optional<T> or_else (F &&f) &;
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) & {
			if (has_value())
				return *this;

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) & {
			return has_value() ? *this : std::forward<F>(f)();
		}

		/// \group or_else
		/// \synopsis template <class F> optional<T> or_else (F &&f) &&;
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) && {
			if (has_value())
				return std::move(*this);

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) && {
			return has_value() ? std::move(*this) : std::forward<F>(f)();
		}

		/// \group or_else
		/// \synopsis template <class F> optional<T> or_else (F &&f) const &;
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) const& {
			if (has_value())
				return *this;

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) const& {
			return has_value() ? *this : std::forward<F>(f)();
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \exclude
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) const&& {
			if (has_value())
				return std::move(*this);

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) const&& {
			return has_value() ? std::move(*this) : std::forward<F>(f)();
		}
#endif

		/// \brief Maps the stored value with `f` if there is one, otherwise returns
		/// `u`.
		///
		/// \details If there is a value stored, then `f` is called with `**this`
		/// and the value is returned. Otherwise `u` is returned.
		///
		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) & {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u);
		}

		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) && {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u);
		}

		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) const& {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) const&& {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u);
		}
#endif

		/// \brief Maps the stored value with `f` if there is one, otherwise calls
		/// `u` and returns the result.
		///
		/// \details If there is a value stored, then `f` is
		/// called with `**this` and the value is returned. Otherwise
		/// `std::forward<U>(u)()` is returned.
		///
		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u) &;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) & {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u)();
		}

		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u)
		/// &&;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) && {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u)();
		}

		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u)
		/// const &;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) const& {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u)();
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u)
		/// const &&;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) const&& {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u)();
		}
#endif

		/// \returns `u` if `*this` has a value, otherwise an empty optional.
		template <class U>
		constexpr optional<typename std::decay<U>::type> conjunction(U&& u) const {
			using result = optional<detail::decay_t<U>>;
			return has_value() ? result { u } : result { nullopt };
		}

		/// \returns `rhs` if `*this` is empty, otherwise the current value.
		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(const optional& rhs) & {
			return has_value() ? *this : rhs;
		}

		/// \group disjunction
		constexpr optional disjunction(const optional& rhs) const& {
			return has_value() ? *this : rhs;
		}

		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(const optional& rhs) && {
			return has_value() ? std::move(*this) : rhs;
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group disjunction
		constexpr optional disjunction(const optional& rhs) const&& {
			return has_value() ? std::move(*this) : rhs;
		}
#endif

		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(optional&& rhs) & {
			return has_value() ? *this : std::move(rhs);
		}

		/// \group disjunction
		constexpr optional disjunction(optional&& rhs) const& {
			return has_value() ? *this : std::move(rhs);
		}

		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(optional&& rhs) && {
			return has_value() ? std::move(*this) : std::move(rhs);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group disjunction
		constexpr optional disjunction(optional&& rhs) const&& {
			return has_value() ? std::move(*this) : std::move(rhs);
		}
#endif

		/// Takes the value out of the optional, leaving it empty
		/// \group take
		optional take() & {
			optional ret = *this;
			reset();
			return ret;
		}

		/// \group take
		optional take() const& {
			optional ret = *this;
			reset();
			return ret;
		}

		/// \group take
		optional take() && {
			optional ret = std::move(*this);
			reset();
			return ret;
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group take
		optional take() const&& {
			optional ret = std::move(*this);
			reset();
			return ret;
		}
#endif

		using value_type = T;

		/// Constructs an optional that does not contain a value.
		/// \group ctor_empty
		constexpr optional() noexcept = default;

		/// \group ctor_empty
		constexpr optional(nullopt_t) noexcept {
		}

		/// Copy constructor
		///
		/// If `rhs` contains a value, the stored value is direct-initialized with
		/// it. Otherwise, the constructed optional is empty.
		SOL_TL_OPTIONAL_11_CONSTEXPR optional(const optional& rhs) = default;

		/// Move constructor
		///
		/// If `rhs` contains a value, the stored value is direct-initialized with
		/// it. Otherwise, the constructed optional is empty.
		SOL_TL_OPTIONAL_11_CONSTEXPR optional(optional&& rhs) = default;

		/// Constructs the stored value in-place using the given arguments.
		/// \group in_place
		/// \synopsis template <class... Args> constexpr explicit optional(in_place_t, Args&&... args);
		template <class... Args>
		constexpr explicit optional(detail::enable_if_t<std::is_constructible<T, Args...>::value, in_place_t>, Args&&... args)
		: base(in_place, std::forward<Args>(args)...) {
		}

		/// \group in_place
		/// \synopsis template <class U, class... Args> \n constexpr explicit optional(in_place_t, std::initializer_list<U>&, Args&&... args);
		template <class U, class... Args>
		SOL_TL_OPTIONAL_11_CONSTEXPR explicit optional(detail::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value, in_place_t>,
		     std::initializer_list<U> il, Args&&... args) {
			this->construct(il, std::forward<Args>(args)...);
		}

#if 0 // SOL_MODIFICATION
      /// Constructs the stored value with `u`.
      /// \synopsis template <class U=T> constexpr optional(U &&u);
		template <class U = T, detail::enable_if_t<std::is_convertible<U&&, T>::value>* = nullptr, detail::enable_forward_value<T, U>* = nullptr>
		constexpr optional(U&& u) : base(in_place, std::forward<U>(u)) {
		}

		/// \exclude
		template <class U = T, detail::enable_if_t<!std::is_convertible<U&&, T>::value>* = nullptr, detail::enable_forward_value<T, U>* = nullptr>
		constexpr explicit optional(U&& u) : base(in_place, std::forward<U>(u)) {
		}
#else
		/// Constructs the stored value with `u`.
		/// \synopsis template <class U=T> constexpr optional(U &&u);
		constexpr optional(T&& u) : base(in_place, std::move(u)) {
		}

		/// \exclude
		constexpr optional(const T& u) : base(in_place, u) {
		}
#endif // sol2 modification

		/// Converting copy constructor.
		/// \synopsis template <class U> optional(const optional<U> &rhs);
		template <class U, detail::enable_from_other<T, U, const U&>* = nullptr, detail::enable_if_t<std::is_convertible<const U&, T>::value>* = nullptr>
		optional(const optional<U>& rhs) {
			if (rhs.has_value()) {
				this->construct(*rhs);
			}
		}

		/// \exclude
		template <class U, detail::enable_from_other<T, U, const U&>* = nullptr, detail::enable_if_t<!std::is_convertible<const U&, T>::value>* = nullptr>
		explicit optional(const optional<U>& rhs) {
			if (rhs.has_value()) {
				this->construct(*rhs);
			}
		}

		/// Converting move constructor.
		/// \synopsis template <class U> optional(optional<U> &&rhs);
		template <class U, detail::enable_from_other<T, U, U&&>* = nullptr, detail::enable_if_t<std::is_convertible<U&&, T>::value>* = nullptr>
		optional(optional<U>&& rhs) {
			if (rhs.has_value()) {
				this->construct(std::move(*rhs));
			}
		}

		/// \exclude
		template <class U, detail::enable_from_other<T, U, U&&>* = nullptr, detail::enable_if_t<!std::is_convertible<U&&, T>::value>* = nullptr>
		explicit optional(optional<U>&& rhs) {
			this->construct(std::move(*rhs));
		}

		/// Destroys the stored value if there is one.
		~optional() = default;

		/// Assignment to empty.
		///
		/// Destroys the current value if there is one.
		optional& operator=(nullopt_t) noexcept {
			if (has_value()) {
				this->m_value.~T();
				this->m_has_value = false;
			}

			return *this;
		}

		/// Copy assignment.
		///
		/// Copies the value from `rhs` if there is one. Otherwise resets the stored
		/// value in `*this`.
		optional& operator=(const optional& rhs) = default;

		/// Move assignment.
		///
		/// Moves the value from `rhs` if there is one. Otherwise resets the stored
		/// value in `*this`.
		optional& operator=(optional&& rhs) = default;

		/// Assigns the stored value from `u`, destroying the old value if there was
		/// one.
		/// \synopsis optional &operator=(U &&u);
		template <class U = T, detail::enable_assign_forward<T, U>* = nullptr>
		optional& operator=(U&& u) {
			if (has_value()) {
				this->m_value = std::forward<U>(u);
			}
			else {
				this->construct(std::forward<U>(u));
			}

			return *this;
		}

		/// Converting copy assignment operator.
		///
		/// Copies the value from `rhs` if there is one. Otherwise resets the stored
		/// value in `*this`.
		/// \synopsis optional &operator=(const optional<U> & rhs);
		template <class U, detail::enable_assign_from_other<T, U, const U&>* = nullptr>
		optional& operator=(const optional<U>& rhs) {
			if (has_value()) {
				if (rhs.has_value()) {
					this->m_value = *rhs;
				}
				else {
					this->hard_reset();
				}
			}

			if (rhs.has_value()) {
				this->construct(*rhs);
			}

			return *this;
		}

		// TODO check exception guarantee
		/// Converting move assignment operator.
		///
		/// Moves the value from `rhs` if there is one. Otherwise resets the stored
		/// value in `*this`.
		/// \synopsis optional &operator=(optional<U> && rhs);
		template <class U, detail::enable_assign_from_other<T, U, U>* = nullptr>
		optional& operator=(optional<U>&& rhs) {
			if (has_value()) {
				if (rhs.has_value()) {
					this->m_value = std::move(*rhs);
				}
				else {
					this->hard_reset();
				}
			}

			if (rhs.has_value()) {
				this->construct(std::move(*rhs));
			}

			return *this;
		}

		/// Constructs the value in-place, destroying the current one if there is
		/// one.
		/// \group emplace
		template <class... Args>
		T& emplace(Args&&... args) {
			static_assert(std::is_constructible<T, Args&&...>::value, "T must be constructible with Args");

			*this = nullopt;
			this->construct(std::forward<Args>(args)...);
			return value();
		}

		/// \group emplace
		/// \synopsis template <class U, class... Args> \n T& emplace(std::initializer_list<U> il, Args &&... args);
		template <class U, class... Args>
		detail::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value, T&> emplace(std::initializer_list<U> il, Args&&... args) {
			*this = nullopt;
			this->construct(il, std::forward<Args>(args)...);
			return value();
		}

		/// Swaps this optional with the other.
		///
		/// If neither optionals have a value, nothing happens.
		/// If both have a value, the values are swapped.
		/// If one has a value, it is moved to the other and the movee is left
		/// valueless.
		void swap(optional& rhs) noexcept(std::is_nothrow_move_constructible<T>::value&& detail::is_nothrow_swappable<T>::value) {
			if (has_value()) {
				if (rhs.has_value()) {
					using std::swap;
					swap(**this, *rhs);
				}
				else {
					new (std::addressof(rhs.m_value)) T(std::move(this->m_value));
					this->m_value.T::~T();
				}
			}
			else if (rhs.has_value()) {
				new (std::addressof(this->m_value)) T(std::move(rhs.m_value));
				rhs.m_value.T::~T();
			}
		}

		/// \returns a pointer to the stored value
		/// \requires a value is stored
		/// \group pointer
		/// \synopsis constexpr const T *operator->() const;
		constexpr const T* operator->() const {
			return std::addressof(this->m_value);
		}

		/// \group pointer
		/// \synopsis constexpr T *operator->();
		SOL_TL_OPTIONAL_11_CONSTEXPR T* operator->() {
			return std::addressof(this->m_value);
		}

		/// \returns the stored value
		/// \requires a value is stored
		/// \group deref
		/// \synopsis constexpr T &operator*();
		SOL_TL_OPTIONAL_11_CONSTEXPR T& operator*() & {
			return this->m_value;
		}

		/// \group deref
		/// \synopsis constexpr const T &operator*() const;
		constexpr const T& operator*() const& {
			return this->m_value;
		}

		/// \exclude
		SOL_TL_OPTIONAL_11_CONSTEXPR T&& operator*() && {
			return std::move(this->m_value);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \exclude
		constexpr const T&& operator*() const&& {
			return std::move(this->m_value);
		}
#endif

		/// \returns whether or not the optional has a value
		/// \group has_value
		constexpr bool has_value() const noexcept {
			return this->m_has_value;
		}

		/// \group has_value
		constexpr explicit operator bool() const noexcept {
			return this->m_has_value;
		}

		/// \returns the contained value if there is one, otherwise throws
		/// [bad_optional_access]
		/// \group value
		/// \synopsis constexpr T &value();
		SOL_TL_OPTIONAL_11_CONSTEXPR T& value() & {
			if (has_value())
				return this->m_value;
#if SOL_IS_OFF(SOL_EXCEPTIONS)
			std::abort();
#else
			throw bad_optional_access();
#endif // No exceptions allowed
		}
		/// \group value
		/// \synopsis constexpr const T &value() const;
		SOL_TL_OPTIONAL_11_CONSTEXPR const T& value() const& {
			if (has_value())
				return this->m_value;
#if SOL_IS_OFF(SOL_EXCEPTIONS)
			std::abort();
#else
			throw bad_optional_access();
#endif // No exceptions allowed
		}
		/// \exclude
		SOL_TL_OPTIONAL_11_CONSTEXPR T&& value() && {
			if (has_value())
				return std::move(this->m_value);
#if SOL_IS_OFF(SOL_EXCEPTIONS)
			std::abort();
#else
			throw bad_optional_access();
#endif // No exceptions allowed
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \exclude
		SOL_TL_OPTIONAL_11_CONSTEXPR const T&& value() const&& {
			if (has_value())
				return std::move(this->m_value);
#if SOL_IS_OFF(SOL_EXCEPTIONS)
			std::abort();
#else
			throw bad_optional_access();
#endif // No exceptions allowed
		}
#endif

		/// \returns the stored value if there is one, otherwise returns `u`
		/// \group value_or
		template <class U>
		constexpr T value_or(U&& u) const& {
			static_assert(std::is_copy_constructible<T>::value && std::is_convertible<U&&, T>::value, "T must be copy constructible and convertible from U");
			return has_value() ? **this : static_cast<T>(std::forward<U>(u));
		}

		/// \group value_or
		template <class U>
		SOL_TL_OPTIONAL_11_CONSTEXPR T value_or(U&& u) && {
			static_assert(std::is_move_constructible<T>::value && std::is_convertible<U&&, T>::value, "T must be move constructible and convertible from U");
			return has_value() ? **this : static_cast<T>(std::forward<U>(u));
		}

		/// Destroys the stored value if one exists, making the optional empty
		void reset() noexcept {
			if (has_value()) {
				this->m_value.~T();
				this->m_has_value = false;
			}
		}
	}; // namespace sol

	/// \group relop
	/// \brief Compares two optional objects
	/// \details If both optionals contain a value, they are compared with `T`s
	/// relational operators. Otherwise `lhs` and `rhs` are equal only if they are
	/// both empty, and `lhs` is less than `rhs` only if `rhs` is empty and `lhs`
	/// is not.
	template <class T, class U>
	inline constexpr bool operator==(const optional<T>& lhs, const optional<U>& rhs) {
		return lhs.has_value() == rhs.has_value() && (!lhs.has_value() || *lhs == *rhs);
	}
	/// \group relop
	template <class T, class U>
	inline constexpr bool operator!=(const optional<T>& lhs, const optional<U>& rhs) {
		return lhs.has_value() != rhs.has_value() || (lhs.has_value() && *lhs != *rhs);
	}
	/// \group relop
	template <class T, class U>
	inline constexpr bool operator<(const optional<T>& lhs, const optional<U>& rhs) {
		return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
	}
	/// \group relop
	template <class T, class U>
	inline constexpr bool operator>(const optional<T>& lhs, const optional<U>& rhs) {
		return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
	}
	/// \group relop
	template <class T, class U>
	inline constexpr bool operator<=(const optional<T>& lhs, const optional<U>& rhs) {
		return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
	}
	/// \group relop
	template <class T, class U>
	inline constexpr bool operator>=(const optional<T>& lhs, const optional<U>& rhs) {
		return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
	}

	/// \group relop_nullopt
	/// \brief Compares an optional to a `nullopt`
	/// \details Equivalent to comparing the optional to an empty optional
	template <class T>
	inline constexpr bool operator==(const optional<T>& lhs, nullopt_t) noexcept {
		return !lhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator==(nullopt_t, const optional<T>& rhs) noexcept {
		return !rhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator!=(const optional<T>& lhs, nullopt_t) noexcept {
		return lhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator!=(nullopt_t, const optional<T>& rhs) noexcept {
		return rhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator<(const optional<T>&, nullopt_t) noexcept {
		return false;
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator<(nullopt_t, const optional<T>& rhs) noexcept {
		return rhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator<=(const optional<T>& lhs, nullopt_t) noexcept {
		return !lhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept {
		return true;
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator>(const optional<T>& lhs, nullopt_t) noexcept {
		return lhs.has_value();
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator>(nullopt_t, const optional<T>&) noexcept {
		return false;
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept {
		return true;
	}
	/// \group relop_nullopt
	template <class T>
	inline constexpr bool operator>=(nullopt_t, const optional<T>& rhs) noexcept {
		return !rhs.has_value();
	}

	/// \group relop_t
	/// \brief Compares the optional with a value.
	/// \details If the optional has a value, it is compared with the other value
	/// using `T`s relational operators. Otherwise, the optional is considered
	/// less than the value.
	template <class T, class U>
	inline constexpr bool operator==(const optional<T>& lhs, const U& rhs) {
		return lhs.has_value() ? *lhs == rhs : false;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator==(const U& lhs, const optional<T>& rhs) {
		return rhs.has_value() ? lhs == *rhs : false;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator!=(const optional<T>& lhs, const U& rhs) {
		return lhs.has_value() ? *lhs != rhs : true;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator!=(const U& lhs, const optional<T>& rhs) {
		return rhs.has_value() ? lhs != *rhs : true;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator<(const optional<T>& lhs, const U& rhs) {
		return lhs.has_value() ? *lhs < rhs : true;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator<(const U& lhs, const optional<T>& rhs) {
		return rhs.has_value() ? lhs < *rhs : false;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator<=(const optional<T>& lhs, const U& rhs) {
		return lhs.has_value() ? *lhs <= rhs : true;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator<=(const U& lhs, const optional<T>& rhs) {
		return rhs.has_value() ? lhs <= *rhs : false;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator>(const optional<T>& lhs, const U& rhs) {
		return lhs.has_value() ? *lhs > rhs : false;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator>(const U& lhs, const optional<T>& rhs) {
		return rhs.has_value() ? lhs > *rhs : true;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator>=(const optional<T>& lhs, const U& rhs) {
		return lhs.has_value() ? *lhs >= rhs : false;
	}
	/// \group relop_t
	template <class T, class U>
	inline constexpr bool operator>=(const U& lhs, const optional<T>& rhs) {
		return rhs.has_value() ? lhs >= *rhs : true;
	}

	/// \synopsis template <class T>  \n  void swap(optional<T> &lhs, optional<T> &rhs);
	template <class T, detail::enable_if_t<std::is_move_constructible<T>::value>* = nullptr, detail::enable_if_t<detail::is_swappable<T>::value>* = nullptr>
	void swap(optional<T>& lhs, optional<T>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
		return lhs.swap(rhs);
	}

	namespace detail {
		struct i_am_secret { };
	} // namespace detail

	template <class T = detail::i_am_secret, class U, class Ret = detail::conditional_t<std::is_same<T, detail::i_am_secret>::value, detail::decay_t<U>, T>>
	inline constexpr optional<Ret> make_optional(U&& v) {
		return optional<Ret>(std::forward<U>(v));
	}

	template <class T, class... Args>
	inline constexpr optional<T> make_optional(Args&&... args) {
		return optional<T>(in_place, std::forward<Args>(args)...);
	}
	template <class T, class U, class... Args>
	inline constexpr optional<T> make_optional(std::initializer_list<U> il, Args&&... args) {
		return optional<T>(in_place, il, std::forward<Args>(args)...);
	}

#if __cplusplus >= 201703L
	template <class T>
	optional(T) -> optional<T>;
#endif

	/// \exclude
	namespace detail {
#ifdef SOL_TL_OPTIONAL_CXX14
		template <class Opt, class F, class Ret = decltype(detail::invoke(std::declval<F>(), *std::declval<Opt>())),
		     detail::enable_if_t<!std::is_void<Ret>::value>* = nullptr>
		constexpr auto optional_map_impl(Opt&& opt, F&& f) {
			return opt.has_value() ? detail::invoke(std::forward<F>(f), *std::forward<Opt>(opt)) : optional<Ret>(nullopt);
		}

		template <class Opt, class F, class Ret = decltype(detail::invoke(std::declval<F>(), *std::declval<Opt>())),
		     detail::enable_if_t<std::is_void<Ret>::value>* = nullptr>
		auto optional_map_impl(Opt&& opt, F&& f) {
			if (opt.has_value()) {
				detail::invoke(std::forward<F>(f), *std::forward<Opt>(opt));
				return make_optional(monostate {});
			}

			return optional<monostate>(nullopt);
		}
#else
		template <class Opt, class F, class Ret = decltype(detail::invoke(std::declval<F>(), *std::declval<Opt>())),
		     detail::enable_if_t<!std::is_void<Ret>::value>* = nullptr>

		constexpr auto optional_map_impl(Opt&& opt, F&& f) -> optional<Ret> {
			return opt.has_value() ? detail::invoke(std::forward<F>(f), *std::forward<Opt>(opt)) : optional<Ret>(nullopt);
		}

		template <class Opt, class F, class Ret = decltype(detail::invoke(std::declval<F>(), *std::declval<Opt>())),
		     detail::enable_if_t<std::is_void<Ret>::value>* = nullptr>

		auto optional_map_impl(Opt&& opt, F&& f) -> optional<monostate> {
			if (opt.has_value()) {
				detail::invoke(std::forward<F>(f), *std::forward<Opt>(opt));
				return monostate {};
			}

			return nullopt;
		}
#endif
	} // namespace detail

	/// Specialization for when `T` is a reference. `optional<T&>` acts similarly
	/// to a `T*`, but provides more operations and shows intent more clearly.
	///
	/// *Examples*:
	///
	/// ```
	/// int i = 42;
	/// sol::optional<int&> o = i;
	/// *o == 42; //true
	/// i = 12;
	/// *o = 12; //true
	/// &*o == &i; //true
	/// ```
	///
	/// Assignment has rebind semantics rather than assign-through semantics:
	///
	/// ```
	/// int j = 8;
	/// o = j;
	///
	/// &*o == &j; //true
	/// ```
	template <class T>
	class optional<T&> {
	public:
// The different versions for C++14 and 11 are needed because deduced return
// types are not SFINAE-safe. This provides better support for things like
// generic lambdas. C.f.
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0826r0.html
#if defined(SOL_TL_OPTIONAL_CXX14) && !defined(SOL_TL_OPTIONAL_GCC49) && !defined(SOL_TL_OPTIONAL_GCC54) && !defined(SOL_TL_OPTIONAL_GCC55)
		/// \group and_then
		/// Carries out some operation which returns an optional on the stored
		/// object if there is one. \requires `std::invoke(std::forward<F>(f),
		/// value())` returns a `std::optional<U>` for some `U`. \returns Let `U` be
		/// the result of `std::invoke(std::forward<F>(f), value())`. Returns a
		/// `std::optional<U>`. The return value is empty if `*this` is empty,
		/// otherwise the return value of `std::invoke(std::forward<F>(f), value())`
		/// is returned.
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto and_then(F&& f) & {
			using result = detail::invoke_result_t<F, T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto and_then(F&& f) && {
			using result = detail::invoke_result_t<F, T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &;
		template <class F>
		constexpr auto and_then(F&& f) const& {
			using result = detail::invoke_result_t<F, const T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &&;
		template <class F>
		constexpr auto and_then(F&& f) const&& {
			using result = detail::invoke_result_t<F, const T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}
#endif
#else
		/// \group and_then
		/// Carries out some operation which returns an optional on the stored
		/// object if there is one. \requires `std::invoke(std::forward<F>(f),
		/// value())` returns a `std::optional<U>` for some `U`. \returns Let `U` be
		/// the result of `std::invoke(std::forward<F>(f), value())`. Returns a
		/// `std::optional<U>`. The return value is empty if `*this` is empty,
		/// otherwise the return value of `std::invoke(std::forward<F>(f), value())`
		/// is returned.
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR detail::invoke_result_t<F, T&> and_then(F&& f) & {
			using result = detail::invoke_result_t<F, T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR detail::invoke_result_t<F, T&> and_then(F&& f) && {
			using result = detail::invoke_result_t<F, T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &;
		template <class F>
		constexpr detail::invoke_result_t<F, const T&> and_then(F&& f) const& {
			using result = detail::invoke_result_t<F, const T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group and_then
		/// \synopsis template <class F> \n constexpr auto and_then(F &&f) const &&;
		template <class F>
		constexpr detail::invoke_result_t<F, const T&> and_then(F&& f) const&& {
			using result = detail::invoke_result_t<F, const T&>;
			static_assert(detail::is_optional<result>::value, "F must return an optional");

			return has_value() ? detail::invoke(std::forward<F>(f), **this) : result(nullopt);
		}
#endif
#endif

#if defined(SOL_TL_OPTIONAL_CXX14) && !defined(SOL_TL_OPTIONAL_GCC49) && !defined(SOL_TL_OPTIONAL_GCC54) && !defined(SOL_TL_OPTIONAL_GCC55)
		/// \brief Carries out some operation on the stored object if there is one.
		/// \returns Let `U` be the result of `std::invoke(std::forward<F>(f),
		/// value())`. Returns a `std::optional<U>`. The return value is empty if
		/// `*this` is empty, otherwise an `optional<U>` is constructed from the
		/// return value of `std::invoke(std::forward<F>(f), value())` and is
		/// returned.
		///
		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto map(F&& f) & {
			return detail::optional_map_impl(*this, std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR auto map(F&& f) && {
			return detail::optional_map_impl(std::move(*this), std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) const&;
		template <class F>
		constexpr auto map(F&& f) const& {
			return detail::optional_map_impl(*this, std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> constexpr auto map(F &&f) const&&;
		template <class F>
		constexpr auto map(F&& f) const&& {
			return detail::optional_map_impl(std::move(*this), std::forward<F>(f));
		}
#else
		/// \brief Carries out some operation on the stored object if there is one.
		/// \returns Let `U` be the result of `std::invoke(std::forward<F>(f),
		/// value())`. Returns a `std::optional<U>`. The return value is empty if
		/// `*this` is empty, otherwise an `optional<U>` is constructed from the
		/// return value of `std::invoke(std::forward<F>(f), value())` and is
		/// returned.
		///
		/// \group map
		/// \synopsis template <class F> auto map(F &&f) &;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR decltype(detail::optional_map_impl(std::declval<optional&>(), std::declval<F&&>())) map(F&& f) & {
			return detail::optional_map_impl(*this, std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> auto map(F &&f) &&;
		template <class F>
		SOL_TL_OPTIONAL_11_CONSTEXPR decltype(detail::optional_map_impl(std::declval<optional&&>(), std::declval<F&&>())) map(F&& f) && {
			return detail::optional_map_impl(std::move(*this), std::forward<F>(f));
		}

		/// \group map
		/// \synopsis template <class F> auto map(F &&f) const&;
		template <class F>
		constexpr decltype(detail::optional_map_impl(std::declval<const optional&>(), std::declval<F&&>())) map(F&& f) const& {
			return detail::optional_map_impl(*this, std::forward<F>(f));
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group map
		/// \synopsis template <class F> auto map(F &&f) const&&;
		template <class F>
		constexpr decltype(detail::optional_map_impl(std::declval<const optional&&>(), std::declval<F&&>())) map(F&& f) const&& {
			return detail::optional_map_impl(std::move(*this), std::forward<F>(f));
		}
#endif
#endif

		/// \brief Calls `f` if the optional is empty
		/// \requires `std::invoke_result_t<F>` must be void or convertible to
		/// `optional<T>`. \effects If `*this` has a value, returns `*this`.
		/// Otherwise, if `f` returns `void`, calls `std::forward<F>(f)` and returns
		/// `std::nullopt`. Otherwise, returns `std::forward<F>(f)()`.
		///
		/// \group or_else
		/// \synopsis template <class F> optional<T> or_else (F &&f) &;
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) & {
			if (has_value())
				return *this;

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) & {
			return has_value() ? *this : std::forward<F>(f)();
		}

		/// \group or_else
		/// \synopsis template <class F> optional<T> or_else (F &&f) &&;
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) && {
			if (has_value())
				return std::move(*this);

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) && {
			return has_value() ? std::move(*this) : std::forward<F>(f)();
		}

		/// \group or_else
		/// \synopsis template <class F> optional<T> or_else (F &&f) const &;
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) const& {
			if (has_value())
				return *this;

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> SOL_TL_OPTIONAL_11_CONSTEXPR or_else(F&& f) const& {
			return has_value() ? *this : std::forward<F>(f)();
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \exclude
		template <class F, detail::enable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) const&& {
			if (has_value())
				return std::move(*this);

			std::forward<F>(f)();
			return nullopt;
		}

		/// \exclude
		template <class F, detail::disable_if_ret_void<F>* = nullptr>
		optional<T> or_else(F&& f) const&& {
			return has_value() ? std::move(*this) : std::forward<F>(f)();
		}
#endif

		/// \brief Maps the stored value with `f` if there is one, otherwise returns
		/// `u`.
		///
		/// \details If there is a value stored, then `f` is called with `**this`
		/// and the value is returned. Otherwise `u` is returned.
		///
		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) & {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u);
		}

		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) && {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u);
		}

		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) const& {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group map_or
		template <class F, class U>
		U map_or(F&& f, U&& u) const&& {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u);
		}
#endif

		/// \brief Maps the stored value with `f` if there is one, otherwise calls
		/// `u` and returns the result.
		///
		/// \details If there is a value stored, then `f` is
		/// called with `**this` and the value is returned. Otherwise
		/// `std::forward<U>(u)()` is returned.
		///
		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u) &;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) & {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u)();
		}

		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u)
		/// &&;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) && {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u)();
		}

		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u)
		/// const &;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) const& {
			return has_value() ? detail::invoke(std::forward<F>(f), **this) : std::forward<U>(u)();
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group map_or_else
		/// \synopsis template <class F, class U> \n auto map_or_else(F &&f, U &&u)
		/// const &&;
		template <class F, class U>
		detail::invoke_result_t<U> map_or_else(F&& f, U&& u) const&& {
			return has_value() ? detail::invoke(std::forward<F>(f), std::move(**this)) : std::forward<U>(u)();
		}
#endif

		/// \returns `u` if `*this` has a value, otherwise an empty optional.
		template <class U>
		constexpr optional<typename std::decay<U>::type> conjunction(U&& u) const {
			using result = optional<detail::decay_t<U>>;
			return has_value() ? result { u } : result { nullopt };
		}

		/// \returns `rhs` if `*this` is empty, otherwise the current value.
		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(const optional& rhs) & {
			return has_value() ? *this : rhs;
		}

		/// \group disjunction
		constexpr optional disjunction(const optional& rhs) const& {
			return has_value() ? *this : rhs;
		}

		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(const optional& rhs) && {
			return has_value() ? std::move(*this) : rhs;
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group disjunction
		constexpr optional disjunction(const optional& rhs) const&& {
			return has_value() ? std::move(*this) : rhs;
		}
#endif

		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(optional&& rhs) & {
			return has_value() ? *this : std::move(rhs);
		}

		/// \group disjunction
		constexpr optional disjunction(optional&& rhs) const& {
			return has_value() ? *this : std::move(rhs);
		}

		/// \group disjunction
		SOL_TL_OPTIONAL_11_CONSTEXPR optional disjunction(optional&& rhs) && {
			return has_value() ? std::move(*this) : std::move(rhs);
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group disjunction
		constexpr optional disjunction(optional&& rhs) const&& {
			return has_value() ? std::move(*this) : std::move(rhs);
		}
#endif

		/// Takes the value out of the optional, leaving it empty
		/// \group take
		optional take() & {
			optional ret = *this;
			reset();
			return ret;
		}

		/// \group take
		optional take() const& {
			optional ret = *this;
			reset();
			return ret;
		}

		/// \group take
		optional take() && {
			optional ret = std::move(*this);
			reset();
			return ret;
		}

#ifndef SOL_TL_OPTIONAL_NO_CONSTRR
		/// \group take
		optional take() const&& {
			optional ret = std::move(*this);
			reset();
			return ret;
		}
#endif

		using value_type = T&;

		/// Constructs an optional that does not contain a value.
		/// \group ctor_empty
		constexpr optional() noexcept : m_value(nullptr) {
		}

		/// \group ctor_empty
		constexpr optional(nullopt_t) noexcept : m_value(nullptr) {
		}

		/// Copy constructor
		///
		/// If `rhs` contains a value, the stored value is direct-initialized with
		/// it. Otherwise, the constructed optional is empty.
		SOL_TL_OPTIONAL_11_CONSTEXPR optional(const optional& rhs) noexcept = default;

		/// Move constructor
		///
		/// If `rhs` contains a value, the stored value is direct-initialized with
		/// it. Otherwise, the constructed optional is empty.
		SOL_TL_OPTIONAL_11_CONSTEXPR optional(optional&& rhs) = default;

		/// Constructs the stored value with `u`.
		/// \synopsis template <class U=T> constexpr optional(U &&u);
		template <class U = T, detail::enable_if_t<!detail::is_optional<detail::decay_t<U>>::value>* = nullptr>
		constexpr optional(U&& u) : m_value(std::addressof(u)) {
			static_assert(std::is_lvalue_reference<U>::value, "U must be an lvalue");
		}

		/// \exclude
		template <class U>
		constexpr explicit optional(const optional<U>& rhs) : optional(*rhs) {
		}

		/// No-op
		~optional() = default;

		/// Assignment to empty.
		///
		/// Destroys the current value if there is one.
		optional& operator=(nullopt_t) noexcept {
			m_value = nullptr;
			return *this;
		}

		/// Copy assignment.
		///
		/// Rebinds this optional to the referee of `rhs` if there is one. Otherwise
		/// resets the stored value in `*this`.
		optional& operator=(const optional& rhs) = default;

		/// Rebinds this optional to `u`.
		///
		/// \requires `U` must be an lvalue reference.
		/// \synopsis optional &operator=(U &&u);
		template <class U = T, detail::enable_if_t<!detail::is_optional<detail::decay_t<U>>::value>* = nullptr>
		optional& operator=(U&& u) {
			static_assert(std::is_lvalue_reference<U>::value, "U must be an lvalue");
			m_value = std::addressof(u);
			return *this;
		}

		/// Converting copy assignment operator.
		///
		/// Rebinds this optional to the referee of `rhs` if there is one. Otherwise
		/// resets the stored value in `*this`.
		template <class U>
		optional& operator=(const optional<U>& rhs) {
			m_value = std::addressof(rhs.value());
			return *this;
		}

		/// Constructs the value in-place, destroying the current one if there is
		/// one.
		///
		/// \group emplace
		template <class... Args>
		T& emplace(Args&&... args) noexcept {
			static_assert(std::is_constructible<T, Args&&...>::value, "T must be constructible with Args");

			*this = nullopt;
			this->construct(std::forward<Args>(args)...);
		}

		/// Swaps this optional with the other.
		///
		/// If neither optionals have a value, nothing happens.
		/// If both have a value, the values are swapped.
		/// If one has a value, it is moved to the other and the movee is left
		/// valueless.
		void swap(optional& rhs) noexcept {
			std::swap(m_value, rhs.m_value);
		}

		/// \returns a pointer to the stored value
		/// \requires a value is stored
		/// \group pointer
		/// \synopsis constexpr const T *operator->() const;
		constexpr const T* operator->() const {
			return m_value;
		}

		/// \group pointer
		/// \synopsis constexpr T *operator->();
		SOL_TL_OPTIONAL_11_CONSTEXPR T* operator->() {
			return m_value;
		}

		/// \returns the stored value
		/// \requires a value is stored
		/// \group deref
		/// \synopsis constexpr T &operator*();
		SOL_TL_OPTIONAL_11_CONSTEXPR T& operator*() {
			return *m_value;
		}

		/// \group deref
		/// \synopsis constexpr const T &operator*() const;
		constexpr const T& operator*() const {
			return *m_value;
		}

		/// \returns whether or not the optional has a value
		/// \group has_value
		constexpr bool has_value() const noexcept {
			return m_value != nullptr;
		}

		/// \group has_value
		constexpr explicit operator bool() const noexcept {
			return m_value != nullptr;
		}

		/// \returns the contained value if there is one, otherwise throws
		/// [bad_optional_access]
		/// \group value
		/// synopsis constexpr T &value();
		SOL_TL_OPTIONAL_11_CONSTEXPR T& value() {
			if (has_value())
				return *m_value;
#if SOL_IS_OFF(SOL_EXCEPTIONS)
			std::abort();
#else
			throw bad_optional_access();
#endif // No exceptions allowed
		}
		/// \group value
		/// \synopsis constexpr const T &value() const;
		SOL_TL_OPTIONAL_11_CONSTEXPR const T& value() const {
			if (has_value())
				return *m_value;
#if SOL_IS_OFF(SOL_EXCEPTIONS)
			std::abort();
#else
			throw bad_optional_access();
#endif // No exceptions allowed
		}

		/// \returns the stored value if there is one, otherwise returns `u`
		/// \group value_or
		template <class U>
		constexpr T& value_or(U&& u) const {
			static_assert(std::is_convertible<U&&, T&>::value, "T must be convertible from U");
			return has_value() ? const_cast<T&>(**this) : static_cast<T&>(std::forward<U>(u));
		}

		/// Destroys the stored value if one exists, making the optional empty
		void reset() noexcept {
			m_value = nullptr;
		}

	private:
		T* m_value;
	};

} // namespace sol

namespace std {
	// TODO SFINAE
	template <class T>
	struct hash<::sol::optional<T>> {
		::std::size_t operator()(const ::sol::optional<T>& o) const {
			if (!o.has_value())
				return 0;

			return ::std::hash<::sol::detail::remove_const_t<T>>()(*o);
		}
	};
} // namespace std

#endif // SOL_TL_OPTIONAL_HPP
