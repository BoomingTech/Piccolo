// sol3

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

#ifndef SOL_TABLE_PROXY_HPP
#define SOL_TABLE_PROXY_HPP

#include <sol/traits.hpp>
#include <sol/function.hpp>
#include <sol/protected_function.hpp>
#include <sol/proxy_base.hpp>

namespace sol {

	template <typename Table, typename Key>
	struct table_proxy : public proxy_base<table_proxy<Table, Key>> {
	private:
		using key_type = detail::proxy_key_t<Key>;

		template <typename T, std::size_t... I>
		decltype(auto) tuple_get(std::index_sequence<I...>) const& {
			return tbl.template traverse_get<T>(std::get<I>(key)...);
		}

		template <typename T, std::size_t... I>
		decltype(auto) tuple_get(std::index_sequence<I...>) && {
			return tbl.template traverse_get<T>(std::get<I>(std::move(key))...);
		}

		template <std::size_t... I, typename T>
		void tuple_set(std::index_sequence<I...>, T&& value) & {
			tbl.traverse_set(std::get<I>(key)..., std::forward<T>(value));
		}

		template <std::size_t... I, typename T>
		void tuple_set(std::index_sequence<I...>, T&& value) && {
			tbl.traverse_set(std::get<I>(std::move(key))..., std::forward<T>(value));
		}

		auto setup_table(std::true_type) {
			auto p = stack::probe_get_field<std::is_same_v<meta::unqualified_t<Table>, global_table>>(lua_state(), key, tbl.stack_index());
			lua_pop(lua_state(), p.levels);
			return p;
		}

		bool is_valid(std::false_type) {
			auto pp = stack::push_pop(tbl);
			auto p = stack::probe_get_field<std::is_same_v<meta::unqualified_t<Table>, global_table>>(lua_state(), key, lua_gettop(lua_state()));
			lua_pop(lua_state(), p.levels);
			return p;
		}

	public:
		Table tbl;
		key_type key;

		template <typename T>
		table_proxy(Table table, T&& k) : tbl(table), key(std::forward<T>(k)) {
		}

		template <typename T>
		table_proxy& set(T&& item) & {
			tuple_set(std::make_index_sequence<std::tuple_size_v<meta::unqualified_t<key_type>>>(), std::forward<T>(item));
			return *this;
		}

		template <typename T>
		table_proxy&& set(T&& item) && {
			std::move(*this).tuple_set(std::make_index_sequence<std::tuple_size_v<meta::unqualified_t<key_type>>>(), std::forward<T>(item));
			return std::move(*this);
		}

		template <typename... Args>
		table_proxy& set_function(Args&&... args) & {
			tbl.set_function(key, std::forward<Args>(args)...);
			return *this;
		}

		template <typename... Args>
		table_proxy&& set_function(Args&&... args) && {
			tbl.set_function(std::move(key), std::forward<Args>(args)...);
			return std::move(*this);
		}

		template <typename T>
		table_proxy& operator=(T&& other) & {
			using Tu = meta::unwrap_unqualified_t<T>;
			if constexpr (!is_lua_reference_or_proxy_v<Tu> && meta::is_invocable_v<Tu>) {
				return set_function(std::forward<T>(other));
			}
			else {
				return set(std::forward<T>(other));
			}
		}

		template <typename T>
		table_proxy&& operator=(T&& other) && {
			using Tu = meta::unwrap_unqualified_t<T>;
			if constexpr (!is_lua_reference_or_proxy_v<Tu> && meta::is_invocable_v<Tu> && !detail::is_msvc_callable_rigged_v<T>) {
				return std::move(*this).set_function(std::forward<T>(other));
			}
			else {
				return std::move(*this).set(std::forward<T>(other));
			}
		}

		template <typename T>
		table_proxy& operator=(std::initializer_list<T> other) & {
			return set(std::move(other));
		}

		template <typename T>
		table_proxy&& operator=(std::initializer_list<T> other) && {
			return std::move(*this).set(std::move(other));
		}

		template <typename T>
		bool is() const {
			typedef decltype(get<T>()) U;
			optional<U> option = this->get<optional<U>>();
			return option.has_value();
		}

		template <typename T>
		decltype(auto) get() const& {
			using idx_seq = std::make_index_sequence<std::tuple_size_v<meta::unqualified_t<key_type>>>;
			return tuple_get<T>(idx_seq());
		}

		template <typename T>
		decltype(auto) get() && {
			using idx_seq = std::make_index_sequence<std::tuple_size_v<meta::unqualified_t<key_type>>>;
			return std::move(*this).template tuple_get<T>(idx_seq());
		}

		template <typename T>
		decltype(auto) get_or(T&& otherwise) const {
			typedef decltype(get<T>()) U;
			optional<U> option = get<optional<U>>();
			if (option) {
				return static_cast<U>(option.value());
			}
			return static_cast<U>(std::forward<T>(otherwise));
		}

		template <typename T, typename D>
		decltype(auto) get_or(D&& otherwise) const {
			optional<T> option = get<optional<T>>();
			if (option) {
				return static_cast<T>(option.value());
			}
			return static_cast<T>(std::forward<D>(otherwise));
		}


		template <typename T>
		decltype(auto) get_or_create() {
			return get_or_create<T>(new_table());
		}

		template <typename T, typename Otherwise>
		decltype(auto) get_or_create(Otherwise&& other) {
			if (!this->valid()) {
				this->set(std::forward<Otherwise>(other));
			}
			return get<T>();
		}

		template <typename K>
		decltype(auto) operator[](K&& k) const& {
			auto keys = meta::tuplefy(key, std::forward<K>(k));
			return table_proxy<Table, decltype(keys)>(tbl, std::move(keys));
		}

		template <typename K>
		decltype(auto) operator[](K&& k) & {
			auto keys = meta::tuplefy(key, std::forward<K>(k));
			return table_proxy<Table, decltype(keys)>(tbl, std::move(keys));
		}

		template <typename K>
		decltype(auto) operator[](K&& k) && {
			auto keys = meta::tuplefy(std::move(key), std::forward<K>(k));
			return table_proxy<Table, decltype(keys)>(tbl, std::move(keys));
		}

		template <typename... Ret, typename... Args>
		decltype(auto) call(Args&&... args) {
			lua_State* L = this->lua_state();
			push(L);
			int idx = lua_gettop(L);
			stack_aligned_function func(L, idx);
			return func.call<Ret...>(std::forward<Args>(args)...);
		}

		template <typename... Args>
		decltype(auto) operator()(Args&&... args) {
			return call<>(std::forward<Args>(args)...);
		}

		bool valid() const {
			auto pp = stack::push_pop(tbl);
			auto p = stack::probe_get_field<std::is_same<meta::unqualified_t<Table>, global_table>::value>(lua_state(), key, lua_gettop(lua_state()));
			lua_pop(lua_state(), p.levels);
			return p;
		}

		int push() const noexcept {
			return push(this->lua_state());
		}

		int push(lua_State* L) const noexcept {
			if constexpr (std::is_same_v<meta::unqualified_t<Table>, global_table> || is_stack_table_v<meta::unqualified_t<Table>>) {
				auto pp = stack::push_pop<true>(tbl);
				int tableindex = pp.index_of(tbl);
				int top_index = lua_gettop(L);
				stack::get_field<true>(lua_state(), key, tableindex);
				lua_replace(L, top_index + 1);
				lua_settop(L, top_index + 1);
			}
			else {
				auto pp = stack::push_pop<false>(tbl);
				int tableindex = pp.index_of(tbl);
				int aftertableindex = lua_gettop(L);
				stack::get_field<false>(lua_state(), key, tableindex);
				lua_replace(L, tableindex);
				lua_settop(L, aftertableindex + 1);
			}
			return 1;
		}

		type get_type() const {
			type t = type::none;
			auto pp = stack::push_pop(tbl);
			auto p = stack::probe_get_field<std::is_same<meta::unqualified_t<Table>, global_table>::value>(lua_state(), key, lua_gettop(lua_state()));
			if (p) {
				t = type_of(lua_state(), -1);
			}
			lua_pop(lua_state(), p.levels);
			return t;
		}

		lua_State* lua_state() const {
			return tbl.lua_state();
		}

		table_proxy& force() {
			if (!this->valid()) {
				this->set(new_table());
			}
			return *this;
		}
	};

	template <typename Table, typename Key, typename T>
	inline bool operator==(T&& left, const table_proxy<Table, Key>& right) {
		using G = decltype(stack::get<T>(nullptr, 0));
		return right.template get<optional<G>>() == left;
	}

	template <typename Table, typename Key, typename T>
	inline bool operator==(const table_proxy<Table, Key>& right, T&& left) {
		using G = decltype(stack::get<T>(nullptr, 0));
		return right.template get<optional<G>>() == left;
	}

	template <typename Table, typename Key, typename T>
	inline bool operator!=(T&& left, const table_proxy<Table, Key>& right) {
		using G = decltype(stack::get<T>(nullptr, 0));
		return right.template get<optional<G>>() != left;
	}

	template <typename Table, typename Key, typename T>
	inline bool operator!=(const table_proxy<Table, Key>& right, T&& left) {
		using G = decltype(stack::get<T>(nullptr, 0));
		return right.template get<optional<G>>() != left;
	}

	template <typename Table, typename Key>
	inline bool operator==(lua_nil_t, const table_proxy<Table, Key>& right) {
		return !right.valid();
	}

	template <typename Table, typename Key>
	inline bool operator==(const table_proxy<Table, Key>& right, lua_nil_t) {
		return !right.valid();
	}

	template <typename Table, typename Key>
	inline bool operator!=(lua_nil_t, const table_proxy<Table, Key>& right) {
		return right.valid();
	}

	template <typename Table, typename Key>
	inline bool operator!=(const table_proxy<Table, Key>& right, lua_nil_t) {
		return right.valid();
	}

	template <bool b>
	template <typename Super>
	basic_reference<b>& basic_reference<b>::operator=(proxy_base<Super>&& r) {
		basic_reference<b> v = r;
		this->operator=(std::move(v));
		return *this;
	}

	template <bool b>
	template <typename Super>
	basic_reference<b>& basic_reference<b>::operator=(const proxy_base<Super>& r) {
		basic_reference<b> v = r;
		this->operator=(std::move(v));
		return *this;
	}

	namespace stack {
		template <typename Table, typename Key>
		struct unqualified_pusher<table_proxy<Table, Key>> {
			static int push(lua_State* L, const table_proxy<Table, Key>& p) {
				return p.push(L);
			}
		};
	} // namespace stack
} // namespace sol

#endif // SOL_TABLE_PROXY_HPP
