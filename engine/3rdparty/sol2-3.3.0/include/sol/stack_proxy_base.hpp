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

#ifndef SOL_STACK_PROXY_BASE_HPP
#define SOL_STACK_PROXY_BASE_HPP

#include <sol/stack.hpp>
#include <sol/proxy_base.hpp>

namespace sol {
	struct stack_proxy_base : public proxy_base<stack_proxy_base> {
	private:
		lua_State* m_L;
		int m_index;

	public:
		stack_proxy_base() : m_L(nullptr), m_index(0) {
		}
		stack_proxy_base(lua_State* L_, int index_) : m_L(L_), m_index(index_) {
		}

		template <typename T>
		decltype(auto) get() const {
			return stack::get<T>(m_L, stack_index());
		}

		template <typename T>
		bool is() const {
			return stack::check<T>(m_L, stack_index());
		}

		template <typename T>
		decltype(auto) as() const {
			return get<T>();
		}

		type get_type() const noexcept {
			return type_of(lua_state(), stack_index());
		}

		int push() const {
			return push(m_L);
		}

		int push(lua_State* L_) const {
			lua_pushvalue(L_, m_index);
			return 1;
		}

		lua_State* lua_state() const {
			return m_L;
		}
		int stack_index() const {
			return m_index;
		}
	};

	namespace stack {
		template <>
		struct unqualified_getter<stack_proxy_base> {
			static stack_proxy_base get(lua_State* L_, int index_ = -1) {
				return stack_proxy_base(L_, index_);
			}
		};

		template <>
		struct unqualified_pusher<stack_proxy_base> {
			static int push(lua_State*, const stack_proxy_base& proxy_reference) {
				return proxy_reference.push();
			}
		};
	} // namespace stack

} // namespace sol

#endif // SOL_STACK_PROXY_BASE_HPP
