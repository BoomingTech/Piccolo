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

#ifndef SOL_BYTECODE_HPP
#define SOL_BYTECODE_HPP

#include <sol/compatibility.hpp>
#include <sol/string_view.hpp>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace sol {

	template <typename Allocator = std::allocator<std::byte>>
	class basic_bytecode : private std::vector<std::byte, Allocator> {
	private:
		using base_t = std::vector<std::byte, Allocator>;

	public:
		using typename base_t::allocator_type;
		using typename base_t::const_iterator;
		using typename base_t::const_pointer;
		using typename base_t::const_reference;
		using typename base_t::const_reverse_iterator;
		using typename base_t::difference_type;
		using typename base_t::iterator;
		using typename base_t::pointer;
		using typename base_t::reference;
		using typename base_t::reverse_iterator;
		using typename base_t::size_type;
		using typename base_t::value_type;

		using base_t::base_t;
		using base_t::operator=;

		using base_t::data;
		using base_t::empty;
		using base_t::max_size;
		using base_t::size;

		using base_t::at;
		using base_t::operator[];
		using base_t::back;
		using base_t::front;

		using base_t::begin;
		using base_t::cbegin;
		using base_t::cend;
		using base_t::end;

		using base_t::crbegin;
		using base_t::crend;
		using base_t::rbegin;
		using base_t::rend;


		using base_t::get_allocator;
		using base_t::swap;

		using base_t::clear;
		using base_t::emplace;
		using base_t::emplace_back;
		using base_t::erase;
		using base_t::insert;
		using base_t::pop_back;
		using base_t::push_back;
		using base_t::reserve;
		using base_t::resize;
		using base_t::shrink_to_fit;

		string_view as_string_view() const {
			return string_view(reinterpret_cast<const char*>(this->data()), this->size());
		}
	};

	template <typename Container>
	inline int basic_insert_dump_writer(lua_State*, const void* memory, size_t memory_size, void* userdata_pointer) {
		using storage_t = Container;
		const std::byte* p_code = static_cast<const std::byte*>(memory);
		storage_t& bc = *static_cast<storage_t*>(userdata_pointer);
#if SOL_IS_OFF(SOL_EXCEPTIONS)
		bc.insert(bc.cend(), p_code, p_code + memory_size);
#else
		try {
			bc.insert(bc.cend(), p_code, p_code + memory_size);
		}
		catch (...) {
			return -1;
		}
#endif
		return 0;
	}

	using bytecode = basic_bytecode<>;

	constexpr inline auto bytecode_dump_writer = &basic_insert_dump_writer<bytecode>;

} // namespace sol

#endif // SOL_BYTECODE_HPP
