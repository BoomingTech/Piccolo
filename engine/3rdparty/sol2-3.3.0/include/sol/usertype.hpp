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

#ifndef SOL_USERTYPE_HPP
#define SOL_USERTYPE_HPP

#include <sol/usertype_core.hpp>
#include <sol/usertype_container_launch.hpp>
#include <sol/usertype_storage.hpp>
#include <sol/usertype_proxy.hpp>
#include <sol/metatable.hpp>

namespace sol {

	template <typename T, typename base_type>
	class basic_usertype : private basic_metatable<base_type> {
	private:
		using base_t = basic_metatable<base_type>;
		using table_base_t = basic_table<base_type>;

		template <typename>
		friend class basic_metatable;

		template <bool, typename>
		friend class basic_table_core;

		template <std::size_t... I, typename... Args>
		void tuple_set(std::index_sequence<I...>, std::tuple<Args...>&& args) {
			(void)args;
			(void)detail::swallow { 0, (this->set(std::get<I * 2>(std::move(args)), std::get<I * 2 + 1>(std::move(args))), 0)... };
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
		using base_t::base_t;

		using base_t::get;
		using base_t::lua_state;
		using base_t::pop;
		using base_t::push;
		using base_t::traverse_get;
		using base_t::traverse_set;
		using base_t::unregister;

		template <typename Key, typename Value>
		basic_usertype& set(Key&& key, Value&& value) {
			optional<u_detail::usertype_storage<T>&> maybe_uts = u_detail::maybe_get_usertype_storage<T>(this->lua_state());
			if (maybe_uts) {
				u_detail::usertype_storage<T>& uts = *maybe_uts;
				uts.set(this->lua_state(), std::forward<Key>(key), std::forward<Value>(value));
			}
			else {
				using ValueU = meta::unqualified_t<Value>;
				// cannot get metatable: try regular table set?
				if constexpr (detail::is_non_factory_constructor_v<ValueU> || detail::is_policy_v<ValueU>) {
					// tag constructors so we don't get destroyed by lack of info
					table_base_t::set(std::forward<Key>(key), detail::tagged<T, Value>(std::forward<Value>(value)));
				}
				else {
					table_base_t::set(std::forward<Key>(key), std::forward<Value>(value));
				}
			}
			return *this;
		}

		template <typename Sig, typename Key, typename... Args>
		basic_usertype& set_function(Key&& key, Args&&... args) {
			set_fx(types<Sig>(), std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template <typename Key, typename... Args>
		basic_usertype& set_function(Key&& key, Args&&... args) {
			set_fx(types<>(), std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template <typename Key>
		usertype_proxy<basic_usertype&, std::decay_t<Key>> operator[](Key&& key) {
			return usertype_proxy<basic_usertype&, std::decay_t<Key>>(*this, std::forward<Key>(key));
		}

		template <typename Key>
		usertype_proxy<const basic_usertype&, std::decay_t<Key>> operator[](Key&& key) const {
			return usertype_proxy<const basic_usertype&, std::decay_t<Key>>(*this, std::forward<Key>(key));
		}
	};

} // namespace sol

#endif // SOL_USERTYPE_HPP
