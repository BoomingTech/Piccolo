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

#ifndef SOL_REFERENCE_HPP
#define SOL_REFERENCE_HPP

#include <sol/types.hpp>
#include <sol/stack_reference.hpp>

#include <functional>

namespace sol {
	namespace detail {
		inline const char (&default_main_thread_name())[9] {
			static const char name[9] = "sol.\xF0\x9F\x93\x8C";
			return name;
		}
	} // namespace detail

	namespace stack {
		inline void remove(lua_State* L_, int rawindex, int count) {
			if (count < 1)
				return;
			int top = lua_gettop(L_);
			if (top < 1) {
				return;
			}
			if (rawindex == -count || top == rawindex) {
				// Slice them right off the top
				lua_pop(L_, static_cast<int>(count));
				return;
			}

			// Remove each item one at a time using stack operations
			// Probably slower, maybe, haven't benchmarked,
			// but necessary
			int index = lua_absindex(L_, rawindex);
			if (index < 0) {
				index = lua_gettop(L_) + (index + 1);
			}
			int last = index + count;
			for (int i = index; i < last; ++i) {
				lua_remove(L_, index);
			}
		}

		struct push_popper_at {
			lua_State* L;
			int index;
			int count;
			push_popper_at(lua_State* L_, int index_ = -1, int count_ = 1) : L(L_), index(index_), count(count_) {
			}
			~push_popper_at() {
				remove(L, index, count);
			}
		};

		template <bool top_level>
		struct push_popper_n {
			lua_State* L;
			int pop_count;
			push_popper_n(lua_State* L_, int pop_count_) : L(L_), pop_count(pop_count_) {
			}
			push_popper_n(const push_popper_n&) = delete;
			push_popper_n(push_popper_n&&) = default;
			push_popper_n& operator=(const push_popper_n&) = delete;
			push_popper_n& operator=(push_popper_n&&) = default;
			~push_popper_n() {
				lua_pop(L, pop_count);
			}
		};

		template <>
		struct push_popper_n<true> {
			push_popper_n(lua_State*, int) {
			}
		};

		template <bool, typename T, typename = void>
		struct push_popper {
			using Tu = meta::unqualified_t<T>;
			T m_object;
			int m_index;

			push_popper(T object_) noexcept : m_object(object_), m_index(lua_absindex(m_object.lua_state(), -m_object.push())) {
			}

			int index_of(const Tu&) const noexcept {
				return m_index;
			}

			~push_popper() {
				m_object.pop();
			}
		};

		template <typename T, typename C>
		struct push_popper<true, T, C> {
			using Tu = meta::unqualified_t<T>;

			push_popper(T) noexcept {
			}

			int index_of(const Tu&) const noexcept {
				return -1;
			}

			~push_popper() {
			}
		};

		template <typename T>
		struct push_popper<false, T, std::enable_if_t<is_stack_based_v<meta::unqualified_t<T>>>> {
			using Tu = meta::unqualified_t<T>;

			push_popper(T) noexcept {
			}

			int index_of(const Tu& object_) const noexcept {
				return object_.stack_index();
			}

			~push_popper() {
			}
		};

		template <bool, typename T, typename = void>
		struct stateless_push_popper {
			using Tu = meta::unqualified_t<T>;
			lua_State* m_L;
			T m_object;
			int m_index;

			stateless_push_popper(lua_State* L_, T object_) noexcept : m_L(L_), m_object(object_), m_index(lua_absindex(m_L, -m_object.push(m_L))) {
			}

			int index_of(const Tu&) const noexcept {
				return m_index;
			}

			~stateless_push_popper() {
				m_object.pop(m_L);
			}
		};

		template <typename T, typename C>
		struct stateless_push_popper<true, T, C> {
			using Tu = meta::unqualified_t<T>;

			stateless_push_popper(lua_State*, T) noexcept {
			}

			int index_of(lua_State*, const Tu&) const noexcept {
				return -1;
			}

			~stateless_push_popper() {
			}
		};

		template <typename T>
		struct stateless_push_popper<false, T, std::enable_if_t<is_stack_based_v<meta::unqualified_t<T>>>> {
			using Tu = meta::unqualified_t<T>;
			lua_State* m_L;

			stateless_push_popper(lua_State* L_, T) noexcept : m_L(L_) {
			}

			int index_of(const Tu& object_) const noexcept {
				return object_.stack_index();
			}

			~stateless_push_popper() {
			}
		};

		template <bool top_level = false, typename T>
		push_popper<top_level, T> push_pop(T&& x) {
			return push_popper<top_level, T>(std::forward<T>(x));
		}

		template <bool top_level = false, typename T>
		stateless_push_popper<top_level, T> push_pop(lua_State* L_, T&& object_) {
			return stateless_push_popper<top_level, T>(L_, std::forward<T>(object_));
		}

		template <typename T>
		push_popper_at push_pop_at(T&& object_) {
			int push_count = object_.push();
			lua_State* L = object_.lua_state();
			return push_popper_at(L, lua_absindex(L, -push_count), push_count);
		}

		template <bool top_level = false>
		push_popper_n<top_level> pop_n(lua_State* L_, int pop_count_) {
			return push_popper_n<top_level>(L_, pop_count_);
		}
	} // namespace stack

	inline lua_State* main_thread(lua_State* L_, lua_State* backup_if_unsupported_ = nullptr) {
#if SOL_LUA_VERSION_I_ < 502
		if (L_ == nullptr)
			return backup_if_unsupported_;
		lua_getglobal(L_, detail::default_main_thread_name());
		auto pp = stack::pop_n(L_, 1);
		if (type_of(L_, -1) == type::thread) {
			return lua_tothread(L_, -1);
		}
		return backup_if_unsupported_;
#else
		if (L_ == nullptr)
			return backup_if_unsupported_;
		lua_rawgeti(L_, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
		lua_State* Lmain = lua_tothread(L_, -1);
		lua_pop(L_, 1);
		return Lmain;
#endif // Lua 5.2+ has the main thread unqualified_getter
	}

	namespace detail {
		struct no_safety_tag {
		} inline constexpr no_safety {};

		template <bool b>
		inline lua_State* pick_main_thread(lua_State* L_, lua_State* backup_if_unsupported = nullptr) {
			(void)L_;
			(void)backup_if_unsupported;
			if (b) {
				return main_thread(L_, backup_if_unsupported);
			}
			return L_;
		}
	} // namespace detail

	class stateless_reference {
	private:
		template <bool o_main_only>
		friend class basic_reference;

		int ref = LUA_NOREF;

		int copy_ref(lua_State* L_) const noexcept {
			if (ref == LUA_NOREF)
				return LUA_NOREF;
			push(L_);
			return luaL_ref(L_, LUA_REGISTRYINDEX);
		}

		lua_State* copy_assign_ref(lua_State* L_, lua_State* rL, const stateless_reference& r) {
			if (valid(L_)) {
				deref(L_);
			}
			ref = r.copy_ref(L_);
			return rL;
		}

		lua_State* move_assign(lua_State* L_, lua_State* rL, stateless_reference&& r) {
			if (valid(L_)) {
				deref(L_);
			}
			ref = r.ref;
			r.ref = LUA_NOREF;
			return rL;
		}

	protected:
		int stack_index() const noexcept {
			return -1;
		}

		stateless_reference(lua_State* L_, global_tag_t) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push this reference value");
#endif // make sure stack doesn't overflow
			lua_pushglobaltable(L_);
			ref = luaL_ref(L_, LUA_REGISTRYINDEX);
		}

		stateless_reference(int raw_ref_index) noexcept : ref(raw_ref_index) {
		}

	public:
		stateless_reference() noexcept = default;
		stateless_reference(lua_nil_t) noexcept : stateless_reference() {
		}
		stateless_reference(const stack_reference& r) noexcept : stateless_reference(r.lua_state(), r.stack_index()) {
		}
		stateless_reference(stack_reference&& r) noexcept : stateless_reference(r.lua_state(), r.stack_index()) {
		}
		stateless_reference(lua_State* L_, const stateless_reference& r) noexcept {
			if (r.ref == LUA_REFNIL) {
				ref = LUA_REFNIL;
				return;
			}
			if (r.ref == LUA_NOREF || L_ == nullptr) {
				ref = LUA_NOREF;
				return;
			}
			ref = r.copy_ref(L_);
		}

		stateless_reference(lua_State* L_, stateless_reference&& r) noexcept {
			if (r.ref == LUA_REFNIL) {
				ref = LUA_REFNIL;
				return;
			}
			if (r.ref == LUA_NOREF || L_ == nullptr) {
				ref = LUA_NOREF;
				return;
			}
			ref = r.ref;
			r.ref = LUA_NOREF;
		}

		stateless_reference(lua_State* L_, const stack_reference& r) noexcept {
			if (L_ == nullptr || r.lua_state() == nullptr || r.get_type() == type::none) {
				ref = LUA_NOREF;
				return;
			}
			if (r.get_type() == type::lua_nil) {
				ref = LUA_REFNIL;
				return;
			}
			if (L_ != r.lua_state() && !detail::xmovable(L_, r.lua_state())) {
				return;
			}
			r.push(L_);
			ref = luaL_ref(L_, LUA_REGISTRYINDEX);
		}

		stateless_reference(lua_State* L_, const stateless_stack_reference& r) noexcept : stateless_reference(L_, r.stack_index()) {
		}

		stateless_reference(lua_State* L_, int index = -1) noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push this reference value");
#endif // make sure stack doesn't overflow
			lua_pushvalue(L_, index);
			ref = luaL_ref(L_, LUA_REGISTRYINDEX);
		}
		stateless_reference(lua_State* L_, absolute_index index_) noexcept : stateless_reference(L_, index_.index) {
		}
		stateless_reference(lua_State* L_, ref_index index_) noexcept {
			lua_rawgeti(L_, LUA_REGISTRYINDEX, index_.index);
			ref = luaL_ref(L_, LUA_REGISTRYINDEX);
		}
		stateless_reference(lua_State*, lua_nil_t) noexcept {
		}

		~stateless_reference() noexcept = default;

		stateless_reference(const stateless_reference& o) noexcept = delete;
		stateless_reference& operator=(const stateless_reference& r) noexcept = delete;

		stateless_reference(stateless_reference&& o) noexcept : ref(o.ref) {
			o.ref = LUA_NOREF;
		}

		stateless_reference& operator=(stateless_reference&& o) noexcept {
			ref = o.ref;
			o.ref = LUA_NOREF;
			return *this;
		}

		int push(lua_State* L_) const noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push this reference value");
#endif // make sure stack doesn't overflow
			lua_rawgeti(L_, LUA_REGISTRYINDEX, ref);
			return 1;
		}

		void pop(lua_State* L_, int n = 1) const noexcept {
			lua_pop(L_, n);
		}

		int registry_index() const noexcept {
			return ref;
		}

		void reset(lua_State* L_) noexcept {
			if (valid(L_)) {
				deref(L_);
			}
			ref = LUA_NOREF;
		}

		void reset(lua_State* L_, int index_) noexcept {
			reset(L_);
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push this reference value");
#endif // make sure stack doesn't overflow
			lua_pushvalue(L_, index_);
			ref = luaL_ref(L_, LUA_REGISTRYINDEX);
		}

		bool valid(lua_State*) const noexcept {
			return !(ref == LUA_NOREF || ref == LUA_REFNIL);
		}

		const void* pointer(lua_State* L_) const noexcept {
			int si = push(L_);
			const void* vp = lua_topointer(L_, -si);
			lua_pop(L_, si);
			return vp;
		}

		type get_type(lua_State* L_) const noexcept {
			int p = push(L_);
			int result = lua_type(L_, -1);
			pop(L_, p);
			return static_cast<type>(result);
		}

		void abandon(lua_State* = nullptr) {
			ref = LUA_NOREF;
		}

		void deref(lua_State* L_) const noexcept {
			luaL_unref(L_, LUA_REGISTRYINDEX, ref);
		}

		stateless_reference copy(lua_State* L_) const noexcept {
			if (!valid(L_)) {
				return {};
			}
			return stateless_reference(copy_ref(L_));
		}

		void copy_assign(lua_State* L_, const stateless_reference& right) noexcept {
			if (valid(L_)) {
				deref(L_);
			}
			if (!right.valid(L_)) {
				return;
			}
			ref = right.copy_ref(L_);
		}

		bool equals(lua_State* L_, const stateless_reference& r) const noexcept {
			auto ppl = stack::push_pop(L_, *this);
			auto ppr = stack::push_pop(L_, r);
			return lua_compare(L_, -1, -2, LUA_OPEQ) == 1;
		}

		bool equals(lua_State* L_, const stateless_stack_reference& r) const noexcept {
			auto ppl = stack::push_pop(L_, *this);
			return lua_compare(L_, -1, r.stack_index(), LUA_OPEQ) == 1;
		}

		bool equals(lua_State* L_, lua_nil_t) const noexcept {
			return valid(L_);
		}
	};

	template <bool main_only = false>
	class basic_reference : public stateless_reference {
	private:
		template <bool o_main_only>
		friend class basic_reference;
		lua_State* luastate = nullptr; // non-owning

		template <bool r_main_only>
		void copy_assign_complex(const basic_reference<r_main_only>& r) {
			if (valid()) {
				deref();
			}
			if (r.ref == LUA_REFNIL) {
				luastate = detail::pick_main_thread < main_only && !r_main_only > (r.lua_state(), r.lua_state());
				ref = LUA_REFNIL;
				return;
			}
			if (r.ref == LUA_NOREF) {
				luastate = r.luastate;
				ref = LUA_NOREF;
				return;
			}
			if (detail::xmovable(lua_state(), r.lua_state())) {
				r.push(lua_state());
				ref = luaL_ref(lua_state(), LUA_REGISTRYINDEX);
				return;
			}
			luastate = detail::pick_main_thread < main_only && !r_main_only > (r.lua_state(), r.lua_state());
			ref = r.copy_ref();
		}

		template <bool r_main_only>
		void move_assign(basic_reference<r_main_only>&& r) {
			if (valid()) {
				deref();
			}
			if (r.ref == LUA_REFNIL) {
				luastate = detail::pick_main_thread < main_only && !r_main_only > (r.lua_state(), r.lua_state());
				ref = LUA_REFNIL;
				return;
			}
			if (r.ref == LUA_NOREF) {
				luastate = r.luastate;
				ref = LUA_NOREF;
				return;
			}
			if (detail::xmovable(lua_state(), r.lua_state())) {
				r.push(lua_state());
				ref = luaL_ref(lua_state(), LUA_REGISTRYINDEX);
				return;
			}

			luastate = detail::pick_main_thread < main_only && !r_main_only > (r.lua_state(), r.lua_state());
			ref = r.ref;
			r.ref = LUA_NOREF;
			r.luastate = nullptr;
		}

	protected:
		basic_reference(lua_State* L_, global_tag_t) noexcept : basic_reference(detail::pick_main_thread<main_only>(L_, L_), global_tag, global_tag) {
		}

		basic_reference(lua_State* L_, global_tag_t, global_tag_t) noexcept : stateless_reference(L_, global_tag), luastate(L_) {
		}

		basic_reference(lua_State* oL, const basic_reference<!main_only>& o) noexcept : stateless_reference(oL, o), luastate(oL) {
		}

		void deref() const noexcept {
			return stateless_reference::deref(lua_state());
		}

		int copy_ref() const noexcept {
			return copy_ref(lua_state());
		}

		int copy_ref(lua_State* L_) const noexcept {
			return stateless_reference::copy_ref(L_);
		}

	public:
		basic_reference() noexcept = default;
		basic_reference(lua_nil_t) noexcept : basic_reference() {
		}
		basic_reference(const stack_reference& r) noexcept : basic_reference(r.lua_state(), r.stack_index()) {
		}
		basic_reference(stack_reference&& r) noexcept : basic_reference(r.lua_state(), r.stack_index()) {
		}
		template <bool r_main_only>
		basic_reference(lua_State* L_, const basic_reference<r_main_only>& r) noexcept : luastate(detail::pick_main_thread<main_only>(L_, L_)) {
			if (r.ref == LUA_REFNIL) {
				ref = LUA_REFNIL;
				return;
			}
			if (r.ref == LUA_NOREF || lua_state() == nullptr) {
				ref = LUA_NOREF;
				return;
			}
			if (detail::xmovable(lua_state(), r.lua_state())) {
				r.push(lua_state());
				ref = luaL_ref(lua_state(), LUA_REGISTRYINDEX);
				return;
			}
			ref = r.copy_ref();
		}

		template <bool r_main_only>
		basic_reference(lua_State* L_, basic_reference<r_main_only>&& r) noexcept : luastate(detail::pick_main_thread<main_only>(L_, L_)) {
			if (r.ref == LUA_REFNIL) {
				ref = LUA_REFNIL;
				return;
			}
			if (r.ref == LUA_NOREF || lua_state() == nullptr) {
				ref = LUA_NOREF;
				return;
			}
			if (detail::xmovable(lua_state(), r.lua_state())) {
				r.push(lua_state());
				ref = luaL_ref(lua_state(), LUA_REGISTRYINDEX);
				return;
			}
			ref = r.ref;
			r.ref = LUA_NOREF;
			r.luastate = nullptr;
		}

		basic_reference(lua_State* L_, const stack_reference& r) noexcept : luastate(detail::pick_main_thread<main_only>(L_, L_)) {
			if (lua_state() == nullptr || r.lua_state() == nullptr || r.get_type() == type::none) {
				ref = LUA_NOREF;
				return;
			}
			if (r.get_type() == type::lua_nil) {
				ref = LUA_REFNIL;
				return;
			}
			if (lua_state() != r.lua_state() && !detail::xmovable(lua_state(), r.lua_state())) {
				return;
			}
			r.push(lua_state());
			ref = luaL_ref(lua_state(), LUA_REGISTRYINDEX);
		}
		basic_reference(lua_State* L_, int index = -1) noexcept : luastate(detail::pick_main_thread<main_only>(L_, L_)) {
			// use L_ to stick with that state's execution stack
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push this reference value");
#endif // make sure stack doesn't overflow
			lua_pushvalue(L_, index);
			ref = luaL_ref(L_, LUA_REGISTRYINDEX);
		}
		basic_reference(lua_State* L_, ref_index index) noexcept : luastate(detail::pick_main_thread<main_only>(L_, L_)) {
			lua_rawgeti(lua_state(), LUA_REGISTRYINDEX, index.index);
			ref = luaL_ref(lua_state(), LUA_REGISTRYINDEX);
		}
		basic_reference(lua_State* L_, lua_nil_t) noexcept : luastate(detail::pick_main_thread<main_only>(L_, L_)) {
		}

		~basic_reference() noexcept {
			if (lua_state() == nullptr || ref == LUA_NOREF)
				return;
			deref();
		}

		basic_reference(const basic_reference& o) noexcept : stateless_reference(o.copy_ref()), luastate(o.lua_state()) {
		}

		basic_reference(basic_reference&& o) noexcept : stateless_reference(std::move(o)), luastate(o.lua_state()) {
			o.luastate = nullptr;
		}

		basic_reference(const basic_reference<!main_only>& o) noexcept
		: basic_reference(detail::pick_main_thread<main_only>(o.lua_state(), o.lua_state()), o) {
		}

		basic_reference(basic_reference<!main_only>&& o) noexcept
		: stateless_reference(std::move(o)), luastate(detail::pick_main_thread<main_only>(o.lua_state(), o.lua_state())) {
			o.luastate = nullptr;
			o.ref = LUA_NOREF;
		}

		basic_reference& operator=(basic_reference&& r) noexcept {
			move_assign(std::move(r));
			return *this;
		}

		basic_reference& operator=(const basic_reference& r) noexcept {
			copy_assign_complex(r);
			return *this;
		}

		basic_reference& operator=(basic_reference<!main_only>&& r) noexcept {
			move_assign(std::move(r));
			return *this;
		}

		basic_reference& operator=(const basic_reference<!main_only>& r) noexcept {
			copy_assign_complex(r);
			return *this;
		}

		basic_reference& operator=(const lua_nil_t&) noexcept {
			reset();
			return *this;
		}

		template <typename Super>
		basic_reference& operator=(proxy_base<Super>&& r);

		template <typename Super>
		basic_reference& operator=(const proxy_base<Super>& r);

		int push() const noexcept {
			return push(lua_state());
		}

		void reset() noexcept {
			stateless_reference::reset(luastate);
			luastate = nullptr;
		}

		int push(lua_State* L_) const noexcept {
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L_, 1, "not enough Lua stack space to push this reference value");
#endif // make sure stack doesn't overflow
			if (lua_state() == nullptr) {
				lua_pushnil(L_);
				return 1;
			}
			lua_rawgeti(lua_state(), LUA_REGISTRYINDEX, ref);
			if (L_ != lua_state()) {
				lua_xmove(lua_state(), L_, 1);
			}
			return 1;
		}

		void pop() const noexcept {
			pop(lua_state());
		}

		void pop(lua_State* L_, int n = 1) const noexcept {
			stateless_reference::pop(L_, n);
		}

		int registry_index() const noexcept {
			return stateless_reference::registry_index();
		}

		bool valid() const noexcept {
			return stateless_reference::valid(lua_state());
		}

		bool valid(lua_State* L_) const noexcept {
			return stateless_reference::valid(L_);
		}

		const void* pointer() const noexcept {
			return stateless_reference::pointer(lua_state());
		}

		explicit operator bool() const noexcept {
			return valid();
		}

		type get_type() const noexcept {
			return stateless_reference::get_type(lua_state());
		}

		lua_State* lua_state() const noexcept {
			return luastate;
		}
	};

	template <bool lb, bool rb>
	inline bool operator==(const basic_reference<lb>& l, const basic_reference<rb>& r) noexcept {
		auto ppl = stack::push_pop(l);
		auto ppr = stack::push_pop(r);
		return lua_compare(l.lua_state(), -1, -2, LUA_OPEQ) == 1;
	}

	template <bool lb, bool rb>
	inline bool operator!=(const basic_reference<lb>& l, const basic_reference<rb>& r) noexcept {
		return !operator==(l, r);
	}

	template <bool lb>
	inline bool operator==(const basic_reference<lb>& l, const stack_reference& r) noexcept {
		auto ppl = stack::push_pop(l);
		return lua_compare(l.lua_state(), -1, r.stack_index(), LUA_OPEQ) == 1;
	}

	template <bool lb>
	inline bool operator!=(const basic_reference<lb>& l, const stack_reference& r) noexcept {
		return !operator==(l, r);
	}

	template <bool rb>
	inline bool operator==(const stack_reference& l, const basic_reference<rb>& r) noexcept {
		auto ppr = stack::push_pop(r);
		return lua_compare(l.lua_state(), -1, r.stack_index(), LUA_OPEQ) == 1;
	}

	template <bool rb>
	inline bool operator!=(const stack_reference& l, const basic_reference<rb>& r) noexcept {
		return !operator==(l, r);
	}

	template <bool lb>
	inline bool operator==(const basic_reference<lb>& lhs, const lua_nil_t&) noexcept {
		return !lhs.valid();
	}

	template <bool rb>
	inline bool operator==(const lua_nil_t&, const basic_reference<rb>& rhs) noexcept {
		return !rhs.valid();
	}

	template <bool lb>
	inline bool operator!=(const basic_reference<lb>& lhs, const lua_nil_t&) noexcept {
		return lhs.valid();
	}

	template <bool rb>
	inline bool operator!=(const lua_nil_t&, const basic_reference<rb>& rhs) noexcept {
		return rhs.valid();
	}

	inline bool operator==(const stateless_reference& l, const stateless_reference& r) noexcept {
		return l.registry_index() == r.registry_index();
	}

	inline bool operator!=(const stateless_reference& l, const stateless_reference& r) noexcept {
		return l.registry_index() != r.registry_index();
	}

	inline bool operator==(const stateless_reference& lhs, const lua_nil_t&) noexcept {
		return lhs.registry_index() == LUA_REFNIL;
	}

	inline bool operator==(const lua_nil_t&, const stateless_reference& rhs) noexcept {
		return rhs.registry_index() == LUA_REFNIL;
	}

	inline bool operator!=(const stateless_reference& lhs, const lua_nil_t&) noexcept {
		return lhs.registry_index() != LUA_REFNIL;
	}

	inline bool operator!=(const lua_nil_t&, const stateless_reference& rhs) noexcept {
		return rhs.registry_index() != LUA_REFNIL;
	}

	struct stateless_reference_equals : public stateless_stack_reference_equals {
		using is_transparent = std::true_type;

		stateless_reference_equals(lua_State* L_) noexcept : stateless_stack_reference_equals(L_) {
		}

		bool operator()(const lua_nil_t& lhs, const stateless_reference& rhs) const noexcept {
			return rhs.equals(lua_state(), lhs);
		}

		bool operator()(const stateless_reference& lhs, const lua_nil_t& rhs) const noexcept {
			return lhs.equals(lua_state(), rhs);
		}

		bool operator()(const stateless_reference& lhs, const stateless_reference& rhs) const noexcept {
			return lhs.equals(lua_state(), rhs);
		}
	};

	struct reference_equals : public stack_reference_equals {
		using is_transparent = std::true_type;

		template <bool rb>
		bool operator()(const lua_nil_t& lhs, const basic_reference<rb>& rhs) const noexcept {
			return lhs == rhs;
		}

		template <bool lb>
		bool operator()(const basic_reference<lb>& lhs, const lua_nil_t& rhs) const noexcept {
			return lhs == rhs;
		}

		template <bool lb, bool rb>
		bool operator()(const basic_reference<lb>& lhs, const basic_reference<rb>& rhs) const noexcept {
			return lhs == rhs;
		}

		template <bool lb>
		bool operator()(const basic_reference<lb>& lhs, const stack_reference& rhs) const noexcept {
			return lhs == rhs;
		}

		template <bool rb>
		bool operator()(const stack_reference& lhs, const basic_reference<rb>& rhs) const noexcept {
			return lhs == rhs;
		}
	};

	struct stateless_reference_hash : public stateless_stack_reference_hash {
		using argument_type = stateless_reference;
		using result_type = std::size_t;
		using is_transparent = std::true_type;

		stateless_reference_hash(lua_State* L_) noexcept : stateless_stack_reference_hash(L_) {
		}

		result_type operator()(const stateless_reference& lhs) const noexcept {
			std::hash<const void*> h;
			return h(lhs.pointer(lua_state()));
		}
	};

	struct reference_hash : public stack_reference_hash {
		using argument_type = reference;
		using result_type = std::size_t;
		using is_transparent = std::true_type;

		template <bool lb>
		result_type operator()(const basic_reference<lb>& lhs) const noexcept {
			std::hash<const void*> h;
			return h(lhs.pointer());
		}
	};
} // namespace sol

#endif // SOL_REFERENCE_HPP
