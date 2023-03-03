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

#ifndef SOL_STACK_REFERENCE_HPP
#define SOL_STACK_REFERENCE_HPP

#include <sol/types.hpp>

namespace sol {
	namespace detail {
		inline bool xmovable(lua_State* leftL, lua_State* rightL) {
			if (rightL == nullptr || leftL == nullptr || leftL == rightL) {
				return false;
			}
			const void* leftregistry = lua_topointer(leftL, LUA_REGISTRYINDEX);
			const void* rightregistry = lua_topointer(rightL, LUA_REGISTRYINDEX);
			return leftregistry == rightregistry;
		}
	} // namespace detail

	class stateless_stack_reference {
	private:
		friend class stack_reference;

		int m_index = 0;

		int registry_index() const noexcept {
			return LUA_NOREF;
		}

	public:
		stateless_stack_reference() noexcept = default;
		stateless_stack_reference(lua_nil_t) noexcept : stateless_stack_reference() {};
		stateless_stack_reference(lua_State* L_, int index_) noexcept : stateless_stack_reference(absolute_index(L_, index_)) {
		}
		stateless_stack_reference(lua_State*, absolute_index index_) noexcept : stateless_stack_reference(index_) {
		}
		stateless_stack_reference(lua_State*, raw_index index_) noexcept : stateless_stack_reference(index_) {
		}
		stateless_stack_reference(absolute_index index_) noexcept : m_index(index_) {
		}
		stateless_stack_reference(raw_index index_) noexcept : m_index(index_) {
		}
		stateless_stack_reference(lua_State*, ref_index) noexcept = delete;
		stateless_stack_reference(ref_index) noexcept = delete;
		stateless_stack_reference(const reference&) noexcept = delete;
		stateless_stack_reference(const stateless_stack_reference&) noexcept = default;
		stateless_stack_reference(stateless_stack_reference&& o) noexcept = default;
		stateless_stack_reference& operator=(stateless_stack_reference&&) noexcept = default;
		stateless_stack_reference& operator=(const stateless_stack_reference&) noexcept = default;

		int push(lua_State* L_) const noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push a single reference value");
#endif // make sure stack doesn't overflow
			lua_pushvalue(L_, m_index);
			return 1;
		}

		void pop(lua_State* L_, int pop_count = 1) const noexcept {
			lua_pop(L_, pop_count);
		}

		int stack_index() const noexcept {
			return m_index;
		}

		const void* pointer(lua_State* L_) const noexcept {
			const void* pointer_id = lua_topointer(L_, stack_index());
			return pointer_id;
		}

		type get_type(lua_State* L_) const noexcept {
			int untyped_value = lua_type(L_, stack_index());
			return static_cast<type>(untyped_value);
		}

		bool valid(lua_State* L) const noexcept {
			type t = get_type(L);
			return t != type::lua_nil && t != type::none;
		}

		void reset(lua_State*) noexcept {
			m_index = 0;
		}

		void reset(lua_State* L_, int index_) noexcept {
			m_index = absolute_index(L_, index_);
		}

		void abandon(lua_State* = nullptr) noexcept {
			m_index = 0;
		}

		stateless_stack_reference copy(lua_State* L_) const noexcept {
			return stateless_stack_reference(L_, raw_index(m_index));
		}

		void copy_assign(lua_State*, const stateless_stack_reference& right) noexcept {
			m_index = right.m_index;
		}

		bool equals(lua_State* L_, const stateless_stack_reference& r) const noexcept {
			return lua_compare(L_, this->stack_index(), r.stack_index(), LUA_OPEQ) == 1;
		}

		bool equals(lua_State* L_, lua_nil_t) const noexcept {
			return valid(L_);
		}
	};

	class stack_reference : public stateless_stack_reference {
	private:
		lua_State* luastate = nullptr;

	public:
		stack_reference() noexcept = default;
		stack_reference(lua_nil_t) noexcept : stack_reference() {};
		stack_reference(lua_State* L, lua_nil_t) noexcept : stateless_stack_reference(L, 0), luastate(L) {
		}
		stack_reference(lua_State* L, int i) noexcept : stateless_stack_reference(L, i), luastate(L) {
		}
		stack_reference(lua_State* L, absolute_index i) noexcept : stateless_stack_reference(L, i), luastate(L) {
		}
		stack_reference(lua_State* L, raw_index i) noexcept : stateless_stack_reference(L, i), luastate(L) {
		}
		stack_reference(lua_State* L, ref_index i) noexcept = delete;
		stack_reference(lua_State* L, const reference& r) noexcept = delete;
		stack_reference(lua_State* L, const stack_reference& r) noexcept : luastate(L) {
			if (!r.valid()) {
				m_index = 0;
				return;
			}
			int i = r.stack_index();
			if (detail::xmovable(lua_state(), r.lua_state())) {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
				luaL_checkstack(L, 1, "not enough Lua stack space to push a single reference value");
#endif // make sure stack doesn't overflow
				lua_pushvalue(r.lua_state(), r.stack_index());
				lua_xmove(r.lua_state(), luastate, 1);
				i = absolute_index(luastate, -1);
			}
			m_index = i;
		}
		stack_reference(stack_reference&& o) noexcept = default;
		stack_reference& operator=(stack_reference&&) noexcept = default;
		stack_reference(const stack_reference&) noexcept = default;
		stack_reference& operator=(const stack_reference&) noexcept = default;

		int push() const noexcept {
			return push(lua_state());
		}

		int push(lua_State* L_) const noexcept {
			return stateless_stack_reference::push(L_);
		}

		void pop() const noexcept {
			pop(lua_state());
		}

		void pop(lua_State* L_, int pop_count_ = 1) const noexcept {
			stateless_stack_reference::pop(L_, pop_count_);
		}

		const void* pointer() const noexcept {
			return stateless_stack_reference::pointer(lua_state());
		}

		type get_type() const noexcept {
			return stateless_stack_reference::get_type(lua_state());
		}

		lua_State* lua_state() const noexcept {
			return luastate;
		}

		bool valid() const noexcept {
			return stateless_stack_reference::valid(lua_state());
		}

		void abandon() {
			stateless_stack_reference::abandon(lua_state());
		}
	};

	inline bool operator==(const stack_reference& l, const stack_reference& r) {
		return lua_compare(l.lua_state(), l.stack_index(), r.stack_index(), LUA_OPEQ) == 1;
	}

	inline bool operator!=(const stack_reference& l, const stack_reference& r) {
		return !operator==(l, r);
	}

	inline bool operator==(const stack_reference& lhs, const lua_nil_t&) {
		return !lhs.valid();
	}

	inline bool operator==(const lua_nil_t&, const stack_reference& rhs) {
		return !rhs.valid();
	}

	inline bool operator!=(const stack_reference& lhs, const lua_nil_t&) {
		return lhs.valid();
	}

	inline bool operator!=(const lua_nil_t&, const stack_reference& rhs) {
		return rhs.valid();
	}

	inline bool operator==(const stateless_stack_reference& l, const stateless_stack_reference& r) {
		return l.stack_index() == r.stack_index();
	}

	inline bool operator!=(const stateless_stack_reference& l, const stateless_stack_reference& r) {
		return l.stack_index() != r.stack_index();
	}

	struct stateless_stack_reference_equals {
		using is_transparent = std::true_type;

		stateless_stack_reference_equals(lua_State* L_) noexcept : m_L(L_) {
		}

		lua_State* lua_state() const noexcept {
			return m_L;
		}

		bool operator()(const stateless_stack_reference& lhs, const stateless_stack_reference& rhs) const {
			return lhs.equals(lua_state(), rhs);
		}

		bool operator()(lua_nil_t lhs, const stateless_stack_reference& rhs) const {
			return rhs.equals(lua_state(), lhs);
		}

		bool operator()(const stateless_stack_reference& lhs, lua_nil_t rhs) const {
			return lhs.equals(lua_state(), rhs);
		}

	private:
		lua_State* m_L;
	};

	struct stack_reference_equals {
		using is_transparent = std::true_type;

		bool operator()(const lua_nil_t& lhs, const stack_reference& rhs) const {
			return lhs == rhs;
		}

		bool operator()(const stack_reference& lhs, const lua_nil_t& rhs) const {
			return lhs == rhs;
		}

		bool operator()(const stack_reference& lhs, const stack_reference& rhs) const {
			return lhs == rhs;
		}
	};

	struct stateless_stack_reference_hash {
		using argument_type = stateless_stack_reference;
		using result_type = std::size_t;
		using is_transparent = std::true_type;

		stateless_stack_reference_hash(lua_State* L_) noexcept : m_L(L_) {
		}

		lua_State* lua_state() const noexcept {
			return m_L;
		}

		result_type operator()(const argument_type& lhs) const noexcept {
			std::hash<const void*> h;
			return h(lhs.pointer(lua_state()));
		}

	private:
		lua_State* m_L;
	};

	struct stack_reference_hash {
		using argument_type = stack_reference;
		using result_type = std::size_t;
		using is_transparent = std::true_type;

		result_type operator()(const argument_type& lhs) const noexcept {
			std::hash<const void*> h;
			return h(lhs.pointer());
		}
	};
} // namespace sol

#endif // SOL_STACK_REFERENCE_HPP
