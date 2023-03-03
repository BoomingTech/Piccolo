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

#ifndef SOL_STACK_POP_HPP
#define SOL_STACK_POP_HPP

#include <sol/stack_core.hpp>
#include <sol/stack_get.hpp>
#include <utility>
#include <tuple>

namespace sol { namespace stack {
	template <typename T, typename>
	struct popper {
		inline static decltype(auto) pop(lua_State* L) {
			if constexpr (is_stack_based_v<meta::unqualified_t<T>>) {
				static_assert(!is_stack_based_v<meta::unqualified_t<T>>,
					"You cannot pop something that lives solely on the stack: it will not remain on the stack when popped and thusly will go out of "
					"scope!");
			}
			else {
				record tracking {};
				decltype(auto) r = get<T>(L, -lua_size<T>::value, tracking);
				lua_pop(L, tracking.used);
				return r;
			}
		}
	};
}} // namespace sol::stack

#endif // SOL_STACK_POP_HPP
