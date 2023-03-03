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

#ifndef SOL_UNSAFE_FUNCTION_HPP
#define SOL_UNSAFE_FUNCTION_HPP

#include <sol/reference.hpp>
#include <sol/object.hpp>
#include <sol/stack.hpp>
#include <sol/function_result.hpp>
#include <sol/function_types.hpp>
#include <sol/bytecode.hpp>
#include <sol/dump_handler.hpp>
#include <cstdint>

namespace sol {
	template <typename ref_t, bool aligned = false>
	class basic_function : public basic_object<ref_t> {
	private:
		using base_t = basic_object<ref_t>;

		void luacall(std::ptrdiff_t argcount, std::ptrdiff_t resultcount) const {
			lua_call(lua_state(), static_cast<int>(argcount), static_cast<int>(resultcount));
		}

		template <std::size_t... I, typename... Ret>
		auto invoke(types<Ret...>, std::index_sequence<I...>, std::ptrdiff_t n) const {
			luacall(n, lua_size<std::tuple<Ret...>>::value);
			return stack::pop<std::tuple<Ret...>>(lua_state());
		}

		template <std::size_t I, typename Ret, meta::enable<meta::neg<std::is_void<Ret>>> = meta::enabler>
		Ret invoke(types<Ret>, std::index_sequence<I>, std::ptrdiff_t n) const {
			luacall(n, lua_size<Ret>::value);
			return stack::pop<Ret>(lua_state());
		}

		template <std::size_t I>
		void invoke(types<void>, std::index_sequence<I>, std::ptrdiff_t n) const {
			luacall(n, 0);
		}

		unsafe_function_result invoke(types<>, std::index_sequence<>, std::ptrdiff_t n) const {
			int stacksize = lua_gettop(lua_state());
			int firstreturn = (std::max)(1, stacksize - static_cast<int>(n));
			luacall(n, LUA_MULTRET);
			int poststacksize = lua_gettop(lua_state());
			int returncount = poststacksize - (firstreturn - 1);
			return unsafe_function_result(lua_state(), firstreturn, returncount);
		}

	public:
		using base_t::lua_state;

		basic_function() = default;
		template <typename T,
		     meta::enable<meta::neg<std::is_same<meta::unqualified_t<T>, basic_function>>, meta::neg<std::is_same<base_t, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_function(T&& r) noexcept : base_t(std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			if (!is_function<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				constructor_handler handler {};
				stack::check<basic_function>(lua_state(), -1, handler);
			}
#endif // Safety
		}
		basic_function(const basic_function&) = default;
		basic_function& operator=(const basic_function&) = default;
		basic_function(basic_function&&) = default;
		basic_function& operator=(basic_function&&) = default;
		basic_function(const stack_reference& r) : basic_function(r.lua_state(), r.stack_index()) {
		}
		basic_function(stack_reference&& r) : basic_function(r.lua_state(), r.stack_index()) {
		}
		basic_function(lua_nil_t n) : base_t(n) {
		}
		template <typename T, meta::enable<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_function(lua_State* L, T&& r) : base_t(L, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_function>(lua_state(), -1, handler);
#endif // Safety
		}
		basic_function(lua_State* L, int index = -1) : base_t(L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_function>(L, index, handler);
#endif // Safety
		}
		basic_function(lua_State* L, ref_index index) : base_t(L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_function>(lua_state(), -1, handler);
#endif // Safety
		}

		template <typename Fx>
		int dump(lua_Writer writer, void* userdata, bool strip, Fx&& on_error) const {
			this->push();
			auto ppn = stack::push_popper_n<false>(this->lua_state(), 1);
			int r = lua_dump(this->lua_state(), writer, userdata, strip ? 1 : 0);
			if (r != 0) {
				return on_error(this->lua_state(), r, writer, userdata, strip);
			}
			return r;
		}

		int dump(lua_Writer writer, void* userdata, bool strip = false) const {
			return dump(writer, userdata, strip, &dump_throw_on_error);
		}

		template <typename Container = bytecode>
		Container dump() const {
			Container bc;
			(void)dump(static_cast<lua_Writer>(&basic_insert_dump_writer<Container>), static_cast<void*>(&bc), false, &dump_panic_on_error);
			return bc;
		}

		template <typename Container = bytecode, typename Fx>
		Container dump(Fx&& on_error) const {
			Container bc;
			(void)dump(static_cast<lua_Writer>(&basic_insert_dump_writer<Container>), static_cast<void*>(&bc), false, std::forward<Fx>(on_error));
			return bc;
		}

		template <typename... Args>
		unsafe_function_result operator()(Args&&... args) const {
			return call<>(std::forward<Args>(args)...);
		}

		template <typename... Ret, typename... Args>
		decltype(auto) operator()(types<Ret...>, Args&&... args) const {
			return call<Ret...>(std::forward<Args>(args)...);
		}

		template <typename... Ret, typename... Args>
		decltype(auto) call(Args&&... args) const {
			if (!aligned) {
				base_t::push();
			}
			int pushcount = stack::multi_push_reference(lua_state(), std::forward<Args>(args)...);
			return invoke(types<Ret...>(), std::make_index_sequence<sizeof...(Ret)>(), static_cast<std::ptrdiff_t>(pushcount));
		}
	};
} // namespace sol

#endif // SOL_UNSAFE_FUNCTION_HPP
