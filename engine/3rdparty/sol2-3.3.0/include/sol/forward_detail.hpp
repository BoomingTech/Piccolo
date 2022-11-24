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

#ifndef SOL_FORWARD_DETAIL_HPP
#define SOL_FORWARD_DETAIL_HPP

#include <sol/version.hpp>

#include <sol/forward.hpp>
#include <sol/traits.hpp>

namespace sol {
	namespace detail {
		const bool default_safe_function_calls =
#if SOL_IS_ON(SOL_SAFE_FUNCTION_CALLS)
		     true;
#else
		     false;
#endif
	} // namespace detail


	namespace meta { namespace meta_detail {
	}} // namespace meta::meta_detail

	namespace stack { namespace stack_detail {
		using undefined_method_func = void (*)(stack_reference);

		template <typename T>
		void set_undefined_methods_on(stack_reference);

		struct undefined_metatable;
	}} // namespace stack::stack_detail
} // namespace sol

#endif // SOL_FORWARD_DETAIL_HPP
