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

#ifndef SOL_TABLE_CORE_HPP
#define SOL_TABLE_CORE_HPP

#include <sol/table_proxy.hpp>
#include <sol/stack.hpp>
#include <sol/function_types.hpp>
#include <sol/table_iterator.hpp>
#include <sol/pairs_iterator.hpp>
#include <sol/types.hpp>
#include <sol/object.hpp>
#include <sol/usertype.hpp>
#include <sol/optional.hpp>

namespace sol {
	namespace detail {
		template <std::size_t n>
		struct clean {
			lua_State* L;
			clean(lua_State* luastate) : L(luastate) {
			}
			~clean() {
				lua_pop(L, static_cast<int>(n));
			}
		};

		struct ref_clean {
			lua_State* L;
			int& pop_count;

			ref_clean(lua_State* L_, int& pop_count_) noexcept : L(L_), pop_count(pop_count_) {
			}
			~ref_clean() {
				lua_pop(L, static_cast<int>(pop_count));
			}
		};

		inline int fail_on_newindex(lua_State* L_) {
			return luaL_error(L_, "sol: cannot modify the elements of an enumeration table");
		}

	} // namespace detail

	template <bool top_level, typename ref_t>
	class basic_table_core : public basic_object<ref_t> {
	private:
		using base_t = basic_object<ref_t>;

		friend class state;
		friend class state_view;
		template <typename, typename>
		friend class basic_usertype;
		template <typename>
		friend class basic_metatable;

		template <typename T>
		using is_get_direct_tableless = meta::boolean<stack::stack_detail::is_get_direct_tableless_v<T, top_level, false>>;

		template <typename T>
		using is_raw_get_direct_tableless = std::false_type;

		template <typename T>
		using is_set_direct_tableless = meta::boolean<stack::stack_detail::is_set_direct_tableless_v<T, top_level, false>>;

		template <typename T>
		using is_raw_set_direct_tableless = std::false_type;

		template <bool raw, typename... Ret, typename... Keys>
		decltype(auto) tuple_get(int table_index, Keys&&... keys) const {
			if constexpr (sizeof...(Ret) < 2) {
				return traverse_get_single_maybe_tuple<raw, Ret...>(table_index, std::forward<Keys>(keys)...);
			}
			else {
				using multi_ret = decltype(stack::pop<std::tuple<Ret...>>(nullptr));
				return multi_ret(traverse_get_single_maybe_tuple<raw, Ret>(table_index, std::forward<Keys>(keys))...);
			}
		}

		template <bool raw, typename Ret, size_t... I, typename Key>
		decltype(auto) traverse_get_single_tuple(int table_index, std::index_sequence<I...>, Key&& key) const {
			return traverse_get_single<raw, Ret>(table_index, std::get<I>(std::forward<Key>(key))...);
		}

		template <bool raw, typename Ret, typename Key>
		decltype(auto) traverse_get_single_maybe_tuple(int table_index, Key&& key) const {
			if constexpr (meta::is_tuple_v<meta::unqualified_t<Key>>) {
				return traverse_get_single_tuple<raw, Ret>(
				     table_index, std::make_index_sequence<std::tuple_size_v<meta::unqualified_t<Key>>>(), std::forward<Key>(key));
			}
			else {
				return traverse_get_single<raw, Ret>(table_index, std::forward<Key>(key));
			}
		}

		template <bool raw, typename Ret, typename... Keys>
		decltype(auto) traverse_get_single(int table_index, Keys&&... keys) const {
			constexpr static bool global = (meta::count_for_to_pack_v < 1, is_get_direct_tableless, meta::unqualified_t<Keys>... >> 0);
			if constexpr (meta::is_optional_v<meta::unqualified_t<Ret>>) {
				int popcount = 0;
				detail::ref_clean c(base_t::lua_state(), popcount);
				return traverse_get_deep_optional<global, raw, detail::insert_mode::none, Ret>(popcount, table_index, std::forward<Keys>(keys)...);
			}
			else {
				detail::clean<sizeof...(Keys) - meta::count_for_pack_v<detail::is_insert_mode, meta::unqualified_t<Keys>...>> c(base_t::lua_state());
				return traverse_get_deep<global, raw, detail::insert_mode::none, Ret>(table_index, std::forward<Keys>(keys)...);
			}
		}

		template <bool raw, typename Pairs, std::size_t... I>
		void tuple_set(std::index_sequence<I...>, Pairs&& pairs) {
			constexpr static bool global = (meta::count_even_for_pack_v < is_set_direct_tableless,
			     meta::unqualified_t<decltype(std::get<I * 2>(std::forward<Pairs>(pairs)))>... >> 0);
			auto pp = stack::push_pop<global>(*this);
			int table_index = pp.index_of(*this);
			lua_State* L = base_t::lua_state();
			(void)table_index;
			(void)L;
			void(detail::swallow { (stack::set_field<(top_level), raw>(
			                             L, std::get<I * 2>(std::forward<Pairs>(pairs)), std::get<I * 2 + 1>(std::forward<Pairs>(pairs)), table_index),
			     0)... });
		}

		template <bool global, bool raw, detail::insert_mode mode, typename T, typename Key, typename... Keys>
		decltype(auto) traverse_get_deep(int table_index, Key&& key, Keys&&... keys) const {
			if constexpr (std::is_same_v<meta::unqualified_t<Key>, create_if_nil_t>) {
				(void)key;
				return traverse_get_deep<false, raw, static_cast<detail::insert_mode>(mode | detail::insert_mode::create_if_nil), T>(
				     table_index, std::forward<Keys>(keys)...);
			}
			else {
				lua_State* L = base_t::lua_state();
				stack::get_field<global, raw>(L, std::forward<Key>(key), table_index);
				if constexpr (sizeof...(Keys) > 0) {
					if constexpr ((mode & detail::insert_mode::create_if_nil) == detail::insert_mode::create_if_nil) {
						type t = type_of(L, -1);
						if (t == type::lua_nil || t == type::none) {
							lua_pop(L, 1);
							stack::push(L, new_table(0, 0));
						}
					}
					return traverse_get_deep<false, raw, mode, T>(lua_gettop(L), std::forward<Keys>(keys)...);
				}
				else {
					if constexpr ((mode & detail::insert_mode::create_if_nil) == detail::insert_mode::create_if_nil) {
						type t = type_of(L, -1);
						if ((t == type::lua_nil || t == type::none) && (is_table_like_v<T>)) {
							lua_pop(L, 1);
							stack::push(L, new_table(0, 0));
						}
					}
					return stack::get<T>(L);
				}
			}
		}

		template <bool global, bool raw, detail::insert_mode mode, typename T, typename Key, typename... Keys>
		decltype(auto) traverse_get_deep_optional(int& popcount, int table_index, Key&& key, Keys&&... keys) const {
			if constexpr (std::is_same_v<meta::unqualified_t<Key>, create_if_nil_t>) {
				constexpr detail::insert_mode new_mode = static_cast<detail::insert_mode>(mode | detail::insert_mode::create_if_nil);
				(void)key;
				return traverse_get_deep_optional<global, raw, new_mode, T>(popcount, table_index, std::forward<Keys>(keys)...);
			}
			else if constexpr (std::is_same_v<meta::unqualified_t<Key>, update_if_empty_t>) {
				constexpr detail::insert_mode new_mode = static_cast<detail::insert_mode>(mode | detail::insert_mode::update_if_empty);
				(void)key;
				return traverse_get_deep_optional<global, raw, new_mode, T>(popcount, table_index, std::forward<Keys>(keys)...);
			}
			else if constexpr (std::is_same_v<meta::unqualified_t<Key>, override_value_t>) {
				constexpr detail::insert_mode new_mode = static_cast<detail::insert_mode>(mode | detail::insert_mode::override_value);
				(void)key;
				return traverse_get_deep_optional<global, raw, new_mode, T>(popcount, table_index, std::forward<Keys>(keys)...);
			}
			else {
				if constexpr (sizeof...(Keys) > 0) {
					lua_State* L = base_t::lua_state();
					auto p = stack::probe_get_field<global, raw>(L, std::forward<Key>(key), table_index);
					popcount += p.levels;
					if (!p.success) {
						if constexpr ((mode & detail::insert_mode::create_if_nil) == detail::insert_mode::create_if_nil) {
							lua_pop(L, 1);
							constexpr bool is_seq = meta::count_for_to_pack_v < 1, std::is_integral, Keys... >> 0;
							stack::push(L, new_table(static_cast<int>(is_seq), static_cast<int>(!is_seq)));
							stack::set_field<global, raw>(L, std::forward<Key>(key), stack_reference(L, -1), table_index);
						}
						else {
							return T(nullopt);
						}
					}
					return traverse_get_deep_optional<false, raw, mode, T>(popcount, lua_gettop(L), std::forward<Keys>(keys)...);
				}
				else {
					using R = decltype(stack::get<T>(nullptr));
					using value_type = typename meta::unqualified_t<R>::value_type;
					lua_State* L = base_t::lua_state();
					auto p = stack::probe_get_field<global, raw, value_type>(L, key, table_index);
					popcount += p.levels;
					if (!p.success) {
						if constexpr ((mode & detail::insert_mode::create_if_nil) == detail::insert_mode::create_if_nil) {
							lua_pop(L, 1);
							stack::push(L, new_table(0, 0));
							stack::set_field<global, raw>(L, std::forward<Key>(key), stack_reference(L, -1), table_index);
							if (stack::check<value_type>(L, lua_gettop(L), &no_panic)) {
								return stack::get<T>(L);
							}
						}
						return R(nullopt);
					}
					return stack::get<T>(L);
				}
			}
		}

		template <bool global, bool raw, detail::insert_mode mode, typename Key, typename... Keys>
		void traverse_set_deep(int table_index, Key&& key, Keys&&... keys) const {
			using KeyU = meta::unqualified_t<Key>;
			if constexpr (std::is_same_v<KeyU, update_if_empty_t>) {
				(void)key;
				traverse_set_deep<global, raw, static_cast<detail::insert_mode>(mode | detail::insert_mode::update_if_empty)>(
				     table_index, std::forward<Keys>(keys)...);
			}
			else if constexpr (std::is_same_v<KeyU, create_if_nil_t>) {
				(void)key;
				traverse_set_deep<global, raw, static_cast<detail::insert_mode>(mode | detail::insert_mode::create_if_nil)>(
				     table_index, std::forward<Keys>(keys)...);
			}
			else if constexpr (std::is_same_v<KeyU, override_value_t>) {
				(void)key;
				traverse_set_deep<global, raw, static_cast<detail::insert_mode>(mode | detail::insert_mode::override_value)>(
				     table_index, std::forward<Keys>(keys)...);
			}
			else {
				lua_State* L = base_t::lua_state();
				if constexpr (sizeof...(Keys) == 1) {
					if constexpr ((mode & detail::insert_mode::update_if_empty) == detail::insert_mode::update_if_empty) {
						auto p = stack::probe_get_field<global, raw>(L, key, table_index);
						lua_pop(L, p.levels);
						if (!p.success) {
							stack::set_field<global, raw>(L, std::forward<Key>(key), std::forward<Keys>(keys)..., table_index);
						}
					}
					else {
						stack::set_field<global, raw>(L, std::forward<Key>(key), std::forward<Keys>(keys)..., table_index);
					}
				}
				else {
					if constexpr (mode != detail::insert_mode::none) {
						stack::get_field<global, raw>(L, key, table_index);
						type vt = type_of(L, -1);
						if constexpr ((mode & detail::insert_mode::update_if_empty) == detail::insert_mode::update_if_empty
						     || (mode & detail::insert_mode::create_if_nil) == detail::insert_mode::create_if_nil) {
							if (vt == type::lua_nil || vt == type::none) {
								constexpr bool is_seq = meta::count_for_to_pack_v < 1, std::is_integral, Keys... >> 0;
								lua_pop(L, 1);
								stack::push(L, new_table(static_cast<int>(is_seq), static_cast<int>(!is_seq)));
								stack::set_field<global, raw>(L, std::forward<Key>(key), stack_reference(L, -1), table_index);
							}
						}
						else {
							if (vt != type::table) {
								constexpr bool is_seq = meta::count_for_to_pack_v < 1, std::is_integral, Keys... >> 0;
								lua_pop(L, 1);
								stack::push(L, new_table(static_cast<int>(is_seq), static_cast<int>(!is_seq)));
								stack::set_field<global, raw>(L, std::forward<Key>(key), stack_reference(L, -1), table_index);
							}
						}
					}
					else {
						stack::get_field<global, raw>(L, std::forward<Key>(key), table_index);
					}
					traverse_set_deep<false, raw, mode>(lua_gettop(L), std::forward<Keys>(keys)...);
				}
			}
		}

	protected:
		basic_table_core(detail::no_safety_tag, lua_nil_t n) : base_t(n) {
		}
		basic_table_core(detail::no_safety_tag, lua_State* L, int index) : base_t(L, index) {
		}
		basic_table_core(detail::no_safety_tag, lua_State* L, ref_index index) : base_t(L, index) {
		}
		template <typename T,
		     meta::enable<meta::neg<meta::any_same<meta::unqualified_t<T>, basic_table_core>>, meta::neg<std::is_same<ref_t, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_table_core(detail::no_safety_tag, T&& r) noexcept : base_t(std::forward<T>(r)) {
		}
		template <typename T, meta::enable<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_table_core(detail::no_safety_tag, lua_State* L, T&& r) noexcept : base_t(L, std::forward<T>(r)) {
		}

	public:
		using iterator = basic_table_iterator<ref_t>;
		using const_iterator = iterator;

		using base_t::lua_state;

		basic_table_core() noexcept = default;
		basic_table_core(const basic_table_core&) = default;
		basic_table_core(basic_table_core&&) = default;
		basic_table_core& operator=(const basic_table_core&) = default;
		basic_table_core& operator=(basic_table_core&&) = default;

		basic_table_core(const stack_reference& r) : basic_table_core(r.lua_state(), r.stack_index()) {
		}

		basic_table_core(stack_reference&& r) : basic_table_core(r.lua_state(), r.stack_index()) {
		}

		template <typename T, meta::enable_any<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_table_core(lua_State* L, T&& r) : base_t(L, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			int table_index = pp.index_of(*this);
			constructor_handler handler {};
			stack::check<basic_table_core>(lua_state(), table_index, handler);
#endif // Safety
		}

		basic_table_core(lua_State* L, const new_table& nt) : base_t(L, -stack::push(L, nt)) {
			if (!is_stack_based<meta::unqualified_t<ref_t>>::value) {
				lua_pop(L, 1);
			}
		}

		basic_table_core(lua_State* L, int index = -1) : basic_table_core(detail::no_safety, L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_table_core>(L, index, handler);
#endif // Safety
		}

		basic_table_core(lua_State* L, ref_index index) : basic_table_core(detail::no_safety, L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			int table_index = pp.index_of(*this);
			constructor_handler handler {};
			stack::check<basic_table_core>(lua_state(), table_index, handler);
#endif // Safety
		}

		template <typename T,
		     meta::enable<meta::neg<meta::any_same<meta::unqualified_t<T>, basic_table_core>>, meta::neg<std::is_same<ref_t, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_table_core(T&& r) noexcept : basic_table_core(detail::no_safety, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			if (!is_table<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				int table_index = pp.index_of(*this);
				constructor_handler handler {};
				stack::check<basic_table_core>(lua_state(), table_index, handler);
			}
#endif // Safety
		}

		basic_table_core(lua_nil_t r) noexcept : basic_table_core(detail::no_safety, r) {
		}

		basic_table_core(lua_State* L, global_tag_t t) noexcept : base_t(L, t) {
		}

		iterator begin() const {
			if (this->get_type() == type::table) {
				return iterator(*this);
			}
			return iterator();
		}

		iterator end() const {
			return iterator();
		}

		const_iterator cbegin() const {
			return begin();
		}

		const_iterator cend() const {
			return end();
		}

		basic_pairs_range<basic_table_core> pairs() noexcept {
			return basic_pairs_range<basic_table_core>(*this);
		}

		basic_pairs_range<const basic_table_core> pairs() const noexcept {
			return basic_pairs_range<const basic_table_core>(*this);
		}

		void clear() {
			auto pp = stack::push_pop<false>(*this);
			int table_index = pp.index_of(*this);
			stack::clear(lua_state(), table_index);
		}

		template <typename... Ret, typename... Keys>
		decltype(auto) get(Keys&&... keys) const {
			static_assert(sizeof...(Keys) == sizeof...(Ret), "number of keys and number of return types do not match");
			constexpr static bool global = meta::all<meta::boolean<top_level>, is_get_direct_tableless<meta::unqualified_t<Keys>>...>::value;
			auto pp = stack::push_pop<global>(*this);
			int table_index = pp.index_of(*this);
			return tuple_get<false, Ret...>(table_index, std::forward<Keys>(keys)...);
		}

		template <typename T, typename Key>
		decltype(auto) get_or(Key&& key, T&& otherwise) const {
			typedef decltype(get<T>("")) U;
			optional<U> option = get<optional<U>>(std::forward<Key>(key));
			if (option) {
				return static_cast<U>(option.value());
			}
			return static_cast<U>(std::forward<T>(otherwise));
		}

		template <typename T, typename Key, typename D>
		decltype(auto) get_or(Key&& key, D&& otherwise) const {
			optional<T> option = get<optional<T>>(std::forward<Key>(key));
			if (option) {
				return static_cast<T>(option.value());
			}
			return static_cast<T>(std::forward<D>(otherwise));
		}

		template <typename T, typename... Keys>
		decltype(auto) traverse_get(Keys&&... keys) const {
			static_assert(sizeof...(Keys) > 0, "must pass at least 1 key to get");
			constexpr static bool global = (meta::count_for_to_pack_v < 1, is_get_direct_tableless, meta::unqualified_t<Keys>... >> 0);
			auto pp = stack::push_pop<global>(*this);
			int table_index = pp.index_of(*this);
			return traverse_get_single<false, T>(table_index, std::forward<Keys>(keys)...);
		}

		template <typename... Keys>
		basic_table_core& traverse_set(Keys&&... keys) {
			static_assert(sizeof...(Keys) > 1, "must pass at least 1 key and 1 value to set");
			constexpr static bool global
			     = (meta::count_when_for_to_pack_v < detail::is_not_insert_mode, 1, is_set_direct_tableless, meta::unqualified_t<Keys>... >> 0);
			auto pp = stack::push_pop<global>(*this);
			int table_index = pp.index_of(*this);
			lua_State* L = base_t::lua_state();
			auto pn = stack::pop_n(L, static_cast<int>(sizeof...(Keys) - 2 - meta::count_for_pack_v<detail::is_insert_mode, meta::unqualified_t<Keys>...>));
			traverse_set_deep<top_level, false, detail::insert_mode::none>(table_index, std::forward<Keys>(keys)...);
			return *this;
		}

		template <typename... Args>
		basic_table_core& set(Args&&... args) {
			if constexpr (sizeof...(Args) == 2) {
				traverse_set(std::forward<Args>(args)...);
			}
			else {
				tuple_set<false>(std::make_index_sequence<sizeof...(Args) / 2>(), std::forward_as_tuple(std::forward<Args>(args)...));
			}
			return *this;
		}

		template <typename... Ret, typename... Keys>
		decltype(auto) raw_get(Keys&&... keys) const {
			static_assert(sizeof...(Keys) == sizeof...(Ret), "number of keys and number of return types do not match");
			constexpr static bool global = (meta::count_for_to_pack_v < 1, is_raw_get_direct_tableless, meta::unqualified_t<Keys>... >> 0);
			auto pp = stack::push_pop<global>(*this);
			int table_index = pp.index_of(*this);
			return tuple_get<true, Ret...>(table_index, std::forward<Keys>(keys)...);
		}

		template <typename T, typename Key>
		decltype(auto) raw_get_or(Key&& key, T&& otherwise) const {
			typedef decltype(raw_get<T>("")) U;
			optional<U> option = raw_get<optional<U>>(std::forward<Key>(key));
			if (option) {
				return static_cast<U>(option.value());
			}
			return static_cast<U>(std::forward<T>(otherwise));
		}

		template <typename T, typename Key, typename D>
		decltype(auto) raw_get_or(Key&& key, D&& otherwise) const {
			optional<T> option = raw_get<optional<T>>(std::forward<Key>(key));
			if (option) {
				return static_cast<T>(option.value());
			}
			return static_cast<T>(std::forward<D>(otherwise));
		}

		template <typename T, typename... Keys>
		decltype(auto) traverse_raw_get(Keys&&... keys) const {
			constexpr static bool global = (meta::count_for_to_pack_v < 1, is_raw_get_direct_tableless, meta::unqualified_t<Keys>... >> 0);
			auto pp = stack::push_pop<global>(*this);
			int table_index = pp.index_of(*this);
			return traverse_get_single<true, T>(table_index, std::forward<Keys>(keys)...);
		}

		template <typename... Keys>
		basic_table_core& traverse_raw_set(Keys&&... keys) {
			constexpr static bool global = (meta::count_for_to_pack_v < 1, is_raw_set_direct_tableless, meta::unqualified_t<Keys>... >> 0);
			auto pp = stack::push_pop<global>(*this);
			lua_State* L = base_t::lua_state();
			auto pn = stack::pop_n(L, static_cast<int>(sizeof...(Keys) - 2 - meta::count_for_pack_v<detail::is_insert_mode, meta::unqualified_t<Keys>...>));
			traverse_set_deep<top_level, true, false>(std::forward<Keys>(keys)...);
			return *this;
		}

		template <typename... Args>
		basic_table_core& raw_set(Args&&... args) {
			tuple_set<true>(std::make_index_sequence<sizeof...(Args) / 2>(), std::forward_as_tuple(std::forward<Args>(args)...));
			return *this;
		}

		template <typename Class, typename Key>
		usertype<Class> new_usertype(Key&& key);

		template <typename Class, typename Key, automagic_flags enrollment_flags>
		usertype<Class> new_usertype(Key&& key, constant_automagic_enrollments<enrollment_flags> enrollment);

		template <typename Class, typename Key>
		usertype<Class> new_usertype(Key&& key, automagic_enrollments enrollment);

		template <typename Class, typename Key, typename Arg, typename... Args,
		     typename = std::enable_if_t<!std::is_base_of_v<automagic_enrollments, meta::unqualified_t<Arg>>>>
		usertype<Class> new_usertype(Key&& key, Arg&& arg, Args&&... args);

		template <bool read_only = true, typename... Args>
		table new_enum(const string_view& name, Args&&... args) {
			table target = create_with(std::forward<Args>(args)...);
			if constexpr (read_only) {
				// Need to create a special iterator to handle this
				table x
				     = create_with(meta_function::new_index, detail::fail_on_newindex, meta_function::index, target, meta_function::pairs, stack::stack_detail::readonly_pairs);
				table shim = create_named(name, metatable_key, x);
				return shim;
			}
			else {
				set(name, target);
				return target;
			}
		}

		template <typename T, bool read_only = true>
		table new_enum(const string_view& name, std::initializer_list<std::pair<string_view, T>> items) {
			table target = create(static_cast<int>(items.size()), static_cast<int>(0));
			for (const auto& kvp : items) {
				target.set(kvp.first, kvp.second);
			}
			if constexpr (read_only) {
				table x = create_with(meta_function::new_index, detail::fail_on_newindex, meta_function::index, target);
				table shim = create_named(name, metatable_key, x);
				return shim;
			}
			else {
				set(name, target);
				return target;
			}
		}

		template <typename Key = object, typename Value = object, typename Fx>
		void for_each(Fx&& fx) const {
			lua_State* L = base_t::lua_state();
			if constexpr (std::is_invocable_v<Fx, Key, Value>) {
				auto pp = stack::push_pop(*this);
				int table_index = pp.index_of(*this);
				stack::push(L, lua_nil);
				while (lua_next(L, table_index)) {
					Key key(L, -2);
					Value value(L, -1);
					auto pn = stack::pop_n(L, 1);
					fx(key, value);
				}
			}
			else {
				auto pp = stack::push_pop(*this);
				int table_index = pp.index_of(*this);
				stack::push(L, lua_nil);
				while (lua_next(L, table_index)) {
					Key key(L, -2);
					Value value(L, -1);
					auto pn = stack::pop_n(L, 1);
					std::pair<Key&, Value&> keyvalue(key, value);
					fx(keyvalue);
				}
			}
		}

		size_t size() const {
			auto pp = stack::push_pop(*this);
			int table_index = pp.index_of(*this);
			lua_State* L = base_t::lua_state();
			lua_len(L, table_index);
			return stack::pop<size_t>(L);
		}

		bool empty() const {
			return cbegin() == cend();
		}

		template <typename T>
		auto operator[](T&& key) & {
			return table_proxy<basic_table_core&, detail::proxy_key_t<T>>(*this, std::forward<T>(key));
		}

		template <typename T>
		auto operator[](T&& key) const& {
			return table_proxy<const basic_table_core&, detail::proxy_key_t<T>>(*this, std::forward<T>(key));
		}

		template <typename T>
		auto operator[](T&& key) && {
			return table_proxy<basic_table_core, detail::proxy_key_t<T>>(std::move(*this), std::forward<T>(key));
		}

		template <typename Sig, typename Key, typename... Args>
		basic_table_core& set_function(Key&& key, Args&&... args) {
			set_fx(types<Sig>(), std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template <typename Key, typename... Args>
		basic_table_core& set_function(Key&& key, Args&&... args) {
			set_fx(types<>(), std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template <typename... Args>
		basic_table_core& add(Args&&... args) {
			auto pp = stack::push_pop(*this);
			int table_index = pp.index_of(*this);
			lua_State* L = base_t::lua_state();
			(void)detail::swallow { 0, (stack::stack_detail::raw_table_set(L, std::forward<Args>(args), table_index), 0)... };
			return *this;
		}

	private:
		template <typename R, typename... Args, typename Fx, typename Key, typename = std::invoke_result_t<Fx, Args...>>
		void set_fx(types<R(Args...)>, Key&& key, Fx&& fx) {
			set_resolved_function<R(Args...)>(std::forward<Key>(key), std::forward<Fx>(fx));
		}

		template <typename Fx, typename Key, meta::enable<meta::is_specialization_of<meta::unqualified_t<Fx>, overload_set>> = meta::enabler>
		void set_fx(types<>, Key&& key, Fx&& fx) {
			set(std::forward<Key>(key), std::forward<Fx>(fx));
		}

		template <typename Fx, typename Key, typename... Args,
		     meta::disable<meta::is_specialization_of<meta::unqualified_t<Fx>, overload_set>> = meta::enabler>
		void set_fx(types<>, Key&& key, Fx&& fx, Args&&... args) {
			set(std::forward<Key>(key), as_function_reference(std::forward<Fx>(fx), std::forward<Args>(args)...));
		}

		template <typename... Sig, typename... Args, typename Key>
		void set_resolved_function(Key&& key, Args&&... args) {
			set(std::forward<Key>(key), as_function_reference<function_sig<Sig...>>(std::forward<Args>(args)...));
		}

	public:
		static inline table create(lua_State* L, int narr = 0, int nrec = 0) {
			lua_createtable(L, narr, nrec);
			table result(L);
			lua_pop(L, 1);
			return result;
		}

		template <typename Key, typename Value, typename... Args>
		static inline table create(lua_State* L, int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
			lua_createtable(L, narr, nrec);
			table result(L);
			result.set(std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
			lua_pop(L, 1);
			return result;
		}

		template <typename... Args>
		static inline table create_with(lua_State* L, Args&&... args) {
			static_assert(sizeof...(Args) % 2 == 0, "You must have an even number of arguments for a key, value ... list.");
			constexpr int narr = static_cast<int>(meta::count_odd_for_pack_v<std::is_integral, Args...>);
			return create(L, narr, static_cast<int>((sizeof...(Args) / 2) - narr), std::forward<Args>(args)...);
		}

		table create(int narr = 0, int nrec = 0) {
			return create(base_t::lua_state(), narr, nrec);
		}

		template <typename Key, typename Value, typename... Args>
		table create(int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
			return create(base_t::lua_state(), narr, nrec, std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
		}

		template <typename Name>
		table create(Name&& name, int narr = 0, int nrec = 0) {
			table x = create(base_t::lua_state(), narr, nrec);
			this->set(std::forward<Name>(name), x);
			return x;
		}

		template <typename Name, typename Key, typename Value, typename... Args>
		table create(Name&& name, int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
			table x = create(base_t::lua_state(), narr, nrec, std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
			this->set(std::forward<Name>(name), x);
			return x;
		}

		template <typename... Args>
		table create_with(Args&&... args) {
			return create_with(base_t::lua_state(), std::forward<Args>(args)...);
		}

		template <typename Name, typename... Args>
		table create_named(Name&& name, Args&&... args) {
			static const int narr = static_cast<int>(meta::count_even_for_pack_v<std::is_integral, Args...>);
			return create(std::forward<Name>(name), narr, (sizeof...(Args) / 2) - narr, std::forward<Args>(args)...);
		}
	};
} // namespace sol

#endif // SOL_TABLE_CORE_HPP
