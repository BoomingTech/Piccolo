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

#ifndef SOL_VARIADIC_RESULTS_HPP
#define SOL_VARIADIC_RESULTS_HPP

#include <sol/stack.hpp>
#include <sol/object.hpp>
#include <sol/as_returns.hpp>
#include <sol/function_result.hpp>
#include <sol/protected_function_result.hpp>

#include <vector>

namespace sol {

	template <typename Al = typename std::allocator<object>>
	struct basic_variadic_results : public std::vector<object, Al> {
	private:
		using base_t = std::vector<object, Al>;

	public:
		basic_variadic_results() : base_t() {
		}

		basic_variadic_results(unsafe_function_result fr) : base_t() {
			this->reserve(fr.return_count());
			this->insert(this->cend(), fr.begin(), fr.end());
		}

		basic_variadic_results(protected_function_result fr) : base_t() {
			this->reserve(fr.return_count());
			this->insert(this->cend(), fr.begin(), fr.end());
		}

		template <typename Arg0, typename... Args,
		     meta::disable_any<std::is_same<meta::unqualified_t<Arg0>, basic_variadic_results>, std::is_same<meta::unqualified_t<Arg0>, function_result>,
		          std::is_same<meta::unqualified_t<Arg0>, protected_function_result>> = meta::enabler>
		basic_variadic_results(Arg0&& arg0, Args&&... args) : base_t(std::forward<Arg0>(arg0), std::forward<Args>(args)...) {
		}

		basic_variadic_results(const basic_variadic_results&) = default;
		basic_variadic_results(basic_variadic_results&&) = default;
	};

	struct variadic_results : public basic_variadic_results<> {
	private:
		using base_t = basic_variadic_results<>;

	public:
		using base_t::base_t;
	};

	template <typename Al>
	struct is_container<basic_variadic_results<Al>> : std::false_type { };

	template <>
	struct is_container<variadic_results> : std::false_type { };

	namespace stack {
		template <typename Al>
		struct unqualified_pusher<basic_variadic_results<Al>> {
			int push(lua_State* L, const basic_variadic_results<Al>& e) {
				int p = 0;
				for (const auto& i : e) {
					p += stack::push(L, i);
				}
				return p;
			}
		};

		template <>
		struct unqualified_pusher<variadic_results> {
			int push(lua_State* L, const variadic_results& r) {
				using base_t = basic_variadic_results<>;
				return stack::push(L, static_cast<const base_t&>(r));
			}
		};
	} // namespace stack

} // namespace sol

#endif // SOL_VARIADIC_RESULTS_HPP
