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

#ifndef SOL_FUNCTION_TYPES_OVERLOAD_HPP
#define SOL_FUNCTION_TYPES_OVERLOAD_HPP

#include <sol/overload.hpp>
#include <sol/call.hpp>
#include <sol/function_types_core.hpp>

namespace sol { namespace function_detail {
	template <int start_skew, typename... Functions>
	struct overloaded_function {
		typedef std::tuple<Functions...> overload_list;
		typedef std::make_index_sequence<sizeof...(Functions)> indices;
		overload_list overloads;

		overloaded_function(overload_list set) : overloads(std::move(set)) {
		}

		overloaded_function(Functions... fxs) : overloads(fxs...) {
		}

		template <typename Fx, std::size_t I, typename... R, typename... Args>
		static int call(types<Fx>, meta::index_value<I>, types<R...>, types<Args...>, lua_State* L, int, int, overload_list& ol) {
			auto& func = std::get<I>(ol);
			int nr = call_detail::call_wrapped<void, true, false, start_skew>(L, func);
			return nr;
		}

		struct on_success {
			template <typename... Args>
			int operator()(Args&&... args) const {
				return call(std::forward<Args>(args)...);
			}
		};

		int operator()(lua_State* L) {
			on_success call_obj {};
			return call_detail::overload_match<Functions...>(call_obj, L, 1 + start_skew, overloads);
		}
	};
}} // namespace sol::function_detail

#endif // SOL_FUNCTION_TYPES_OVERLOAD_HPP
