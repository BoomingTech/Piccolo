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

#ifndef SOL_PAIRS_ITERATOR_HPP
#define SOL_PAIRS_ITERATOR_HPP

#include <sol/version.hpp>

#include <sol/reference.hpp>
#include <sol/stack_reference.hpp>
#include <sol/table_iterator.hpp>
#include <sol/protected_function.hpp>

#include <sol/stack/detail/pairs.hpp>

namespace sol {

	struct pairs_sentinel { };

	class pairs_iterator {
	private:
		inline static constexpr int empty_key_index = -1;

	public:
		using key_type = object;
		using mapped_type = object;
		using value_type = std::pair<object, object>;
		using iterator_category = std::input_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using const_pointer = value_type const*;
		using reference = value_type&;
		using const_reference = const value_type&;

		pairs_iterator() noexcept
		: m_L(nullptr)
		, m_next_function_ref(lua_nil)
		, m_table_ref(lua_nil)
		, m_cached_key_value_pair({ lua_nil, lua_nil })
		, m_key_index(empty_key_index)
		, m_iteration_index(0) {
		}

		pairs_iterator(const pairs_iterator&) = delete;
		pairs_iterator& operator=(const pairs_iterator&) = delete;

		pairs_iterator(pairs_iterator&& right) noexcept
		: m_L(right.m_L)
		, m_next_function_ref(std::move(right.m_next_function_ref))
		, m_table_ref(std::move(right.m_table_ref))
		, m_cached_key_value_pair(std::move(right.m_cached_key_value_pair))
		, m_key_index(right.m_key_index)
		, m_iteration_index(right.m_iteration_index) {
			right.m_key_index = empty_key_index;
		}

		pairs_iterator& operator=(pairs_iterator&& right) noexcept {
			m_L = right.m_L;
			m_next_function_ref = std::move(right.m_next_function_ref);
			m_table_ref = std::move(right.m_table_ref);
			m_cached_key_value_pair = std::move(right.m_cached_key_value_pair);
			m_key_index = right.m_key_index;
			m_iteration_index = right.m_iteration_index;
			right.m_key_index = empty_key_index;
			return *this;
		}

		template <typename Source>
		pairs_iterator(const Source& source_) noexcept : m_L(source_.lua_state()), m_key_index(empty_key_index), m_iteration_index(0) {
			if (m_L == nullptr || !source_.valid()) {
				m_key_index = empty_key_index;
				return;
			}
			int source_index = -source_.push(m_L);
			int abs_source_index = lua_absindex(m_L, source_index);
			int metatable_exists = lua_getmetatable(m_L, abs_source_index);
			lua_remove(m_L, abs_source_index);
			if (metatable_exists == 1) {
				// just has a metatable, but does it have __pairs ?
				stack_reference metatable(m_L, raw_index(abs_source_index));
				stack::get_field<is_global_table_v<Source>, true>(m_L, meta_function::pairs, metatable.stack_index());
				optional<protected_function> maybe_pairs_function = stack::pop<optional<protected_function>>(m_L);
				if (maybe_pairs_function.has_value()) {
					protected_function& pairs_function = *maybe_pairs_function;
					protected_function_result next_fn_and_table_and_first_key = pairs_function(source_);
					if (next_fn_and_table_and_first_key.valid()) {
						m_next_function_ref = next_fn_and_table_and_first_key.get<protected_function>(0);
						m_table_ref = next_fn_and_table_and_first_key.get<sol::reference>(1);
						m_key_index = next_fn_and_table_and_first_key.stack_index() - 1;
						// remove next function and table
						lua_remove(m_L, m_key_index);
						lua_remove(m_L, m_key_index);
						next_fn_and_table_and_first_key.abandon();
						lua_remove(m_L, abs_source_index);
						this->operator++();
						m_iteration_index = 0;
						return;
					}
				}
			}

			{
				auto maybe_next = stack::stack_detail::find_lua_next_function(m_L);
				if (maybe_next.has_value()) {
					m_next_function_ref = std::move(*maybe_next);
					m_table_ref = source_;

					stack::push(m_L, lua_nil);
					m_key_index = lua_gettop(m_L);
					this->operator++();
					m_iteration_index = 0;
					return;
				}
			}

			// okay, so none of the above worked and now we need to create
			// a shim / polyfill instead
			stack::push(m_L, &stack::stack_detail::c_lua_next);
			m_next_function_ref = stack::pop<protected_function>(m_L);
			m_table_ref = source_;
			stack::push(m_L, lua_nil);
			m_key_index = lua_gettop(m_L);
			this->operator++();
			m_iteration_index = 0;
		}

		pairs_iterator& operator++() {
			if (m_key_index == empty_key_index) {
				return *this;
			}
			{
				sol::protected_function_result next_results = m_next_function_ref(m_table_ref, stack_reference(m_L, m_key_index));
				if (!next_results.valid()) {
					// TODO: abort, or throw an error?
					m_clear();
					m_key_index = empty_key_index;
					return *this;
				}
				int next_results_count = next_results.return_count();
				if (next_results_count < 2) {
					// iteration is over!
					next_results.abandon();
					lua_settop(m_L, m_key_index - 1);
					m_key_index = empty_key_index;
					++m_iteration_index;
					return *this;
				}
				else {
					lua_remove(m_L, m_key_index);
					m_key_index = next_results.stack_index() - 1;
					m_cached_key_value_pair.first = stack::get<object>(m_L, m_key_index);
					m_cached_key_value_pair.second = stack::get<object>(m_L, m_key_index + 1);
					lua_settop(m_L, m_key_index);
					next_results.abandon();
				}
			}
			++m_iteration_index;
			return *this;
		}

		std::ptrdiff_t index() const {
			return static_cast<std::ptrdiff_t>(m_iteration_index);
		}

		const_reference operator*() const noexcept {
			return m_cached_key_value_pair;
		}

		reference operator*() noexcept {
			return m_cached_key_value_pair;
		}

		friend bool operator==(const pairs_iterator& left, const pairs_iterator& right) noexcept {
			return left.m_table_ref == right.m_table_ref && left.m_iteration_index == right.m_iteration_index;
		}

		friend bool operator!=(const pairs_iterator& left, const pairs_iterator& right) noexcept {
			return left.m_table_ref != right.m_table_ref || left.m_iteration_index != right.m_iteration_index;
		}

		friend bool operator==(const pairs_iterator& left, const pairs_sentinel&) noexcept {
			return left.m_key_index == empty_key_index;
		}

		friend bool operator!=(const pairs_iterator& left, const pairs_sentinel&) noexcept {
			return left.m_key_index != empty_key_index;
		}

		friend bool operator==(const pairs_sentinel&, const pairs_iterator& left) noexcept {
			return left.m_key_index == empty_key_index;
		}

		friend bool operator!=(const pairs_sentinel&, const pairs_iterator& left) noexcept {
			return left.m_key_index != empty_key_index;
		}

		~pairs_iterator() {
			if (m_key_index != empty_key_index) {
				m_clear();
			}
		}

	private:
		void m_clear() noexcept {
			lua_remove(m_L, m_key_index);
		}

		lua_State* m_L;
		protected_function m_next_function_ref;
		sol::reference m_table_ref;
		std::pair<object, object> m_cached_key_value_pair;
		int m_key_index;
		int m_iteration_index;
	};

	template <typename Source>
	class basic_pairs_range {
	private:
		using source_t = std::add_lvalue_reference_t<Source>;
		source_t m_source;

	public:
		using iterator = pairs_iterator;
		using const_iterator = pairs_iterator;

		basic_pairs_range(source_t source_) noexcept : m_source(source_) {
		}

		iterator begin() noexcept {
			return iterator(m_source);
		}

		iterator begin() const noexcept {
			return iterator(m_source);
		}

		const_iterator cbegin() const noexcept {
			return const_iterator(m_source);
		}

		pairs_sentinel end() noexcept {
			return {};
		}

		pairs_sentinel end() const noexcept {
			return {};
		}

		pairs_sentinel cend() const noexcept {
			return {};
		}
	};
} // namespace sol

#endif // SOL_PAIRS_ITERATOR_HPP
