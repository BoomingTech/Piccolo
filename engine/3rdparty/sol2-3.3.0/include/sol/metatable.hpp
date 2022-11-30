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

#ifndef SOL_METATABLE_HPP
#define SOL_METATABLE_HPP

#include <sol/table_core.hpp>
#include <sol/usertype.hpp>

namespace sol {

	template <typename base_type>
	class basic_metatable : public basic_table<base_type> {
		typedef basic_table<base_type> base_t;
		friend class state;
		friend class state_view;

	protected:
		basic_metatable(detail::no_safety_tag, lua_nil_t n) : base_t(n) {
		}
		basic_metatable(detail::no_safety_tag, lua_State* L, int index) : base_t(L, index) {
		}
		basic_metatable(detail::no_safety_tag, lua_State* L, ref_index index) : base_t(L, index) {
		}
		template <typename T,
		     meta::enable<meta::neg<meta::any_same<meta::unqualified_t<T>, basic_metatable>>, meta::neg<std::is_same<base_type, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_metatable(detail::no_safety_tag, T&& r) noexcept : base_t(std::forward<T>(r)) {
		}
		template <typename T, meta::enable<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_metatable(detail::no_safety_tag, lua_State* L, T&& r) noexcept : base_t(L, std::forward<T>(r)) {
		}

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
		using base_t::lua_state;

		basic_metatable() noexcept = default;
		basic_metatable(const basic_metatable&) = default;
		basic_metatable(basic_metatable&&) = default;
		basic_metatable& operator=(const basic_metatable&) = default;
		basic_metatable& operator=(basic_metatable&&) = default;
		basic_metatable(const stack_reference& r) : basic_metatable(r.lua_state(), r.stack_index()) {
		}
		basic_metatable(stack_reference&& r) : basic_metatable(r.lua_state(), r.stack_index()) {
		}
		template <typename T, meta::enable_any<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_metatable(lua_State* L, T&& r) : base_t(L, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_metatable>(lua_state(), -1, handler);
#endif // Safety
		}
		basic_metatable(lua_State* L, int index = -1) : basic_metatable(detail::no_safety, L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_metatable>(L, index, handler);
#endif // Safety
		}
		basic_metatable(lua_State* L, ref_index index) : basic_metatable(detail::no_safety, L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_metatable>(lua_state(), -1, handler);
#endif // Safety
		}
		template <typename T,
		     meta::enable<meta::neg<meta::any_same<meta::unqualified_t<T>, basic_metatable>>, meta::neg<std::is_same<base_type, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_metatable(T&& r) noexcept : basic_metatable(detail::no_safety, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			if (!is_table<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				constructor_handler handler {};
				stack::check<basic_metatable>(base_t::lua_state(), -1, handler);
			}
#endif // Safety
		}
		basic_metatable(lua_nil_t r) noexcept : basic_metatable(detail::no_safety, r) {
		}

		template <typename Key, typename Value>
		basic_metatable<base_type>& set(Key&& key, Value&& value);

		template <typename Sig, typename Key, typename... Args>
		basic_metatable& set_function(Key&& key, Args&&... args) {
			set_fx(types<Sig>(), std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template <typename Key, typename... Args>
		basic_metatable& set_function(Key&& key, Args&&... args) {
			set_fx(types<>(), std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		void unregister() {
			using ustorage_base = u_detail::usertype_storage_base;

			lua_State* L = this->lua_state();

			auto pp = stack::push_pop(*this);
			int top = lua_gettop(L);

			stack_reference mt(L, -1);
			stack::get_field(L, meta_function::gc_names, mt.stack_index());
			if (type_of(L, -1) != type::table) {
				lua_settop(L, top);
				return;
			}
			stack_reference gc_names_table(L, -1);
			stack::get_field(L, meta_function::storage, mt.stack_index());
			if (type_of(L, -1) != type::lightuserdata) {
				lua_settop(L, top);
				return;
			}
			ustorage_base& base_storage = *static_cast<ustorage_base*>(stack::get<void*>(L, -1));
			std::array<string_view, 6> registry_traits;
			for (std::size_t i = 0; i < registry_traits.size(); ++i) {
				u_detail::submetatable_type smt = static_cast<u_detail::submetatable_type>(i);
				stack::get_field<false, true>(L, smt, gc_names_table.stack_index());
				registry_traits[i] = stack::get<string_view>(L, -1);
			}

			// get the registry
			stack_reference registry(L, raw_index(LUA_REGISTRYINDEX));
			registry.push();
			// eliminate all named entries for this usertype
			// in the registry (luaL_newmetatable does
			// [name] = new table
			// in registry upon creation)
			for (std::size_t i = 0; i < registry_traits.size(); ++i) {
				u_detail::submetatable_type smt = static_cast<u_detail::submetatable_type>(i);
				const string_view& gcmetakey = registry_traits[i];
				if (smt == u_detail::submetatable_type::named) {
					// use .data() to make it treat it like a c string,
					// which it is...
					stack::set_field<true>(L, gcmetakey.data(), lua_nil);
				}
				else {
					// do not change the values in the registry: they need to be present
					// no matter what, for safety's sake
					// stack::set_field(L, gcmetakey, lua_nil, registry.stack_index());
				}
			}

			// destroy all storage and tables
			base_storage.clear();

			// 6 strings from gc_names table,
			// + 1 registry,
			// + 1 gc_names table
			// + 1 light userdata of storage
			// + 1 registry
			// 10 total, 4 left since popping off 6 gc_names tables
			lua_settop(L, top);
		}
	};

} // namespace sol

#endif // SOL_METATABLE_HPP
