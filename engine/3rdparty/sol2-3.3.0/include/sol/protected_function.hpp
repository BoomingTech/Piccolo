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

#ifndef SOL_PROTECTED_FUNCTION_HPP
#define SOL_PROTECTED_FUNCTION_HPP

#include <sol/reference.hpp>
#include <sol/object.hpp>
#include <sol/stack.hpp>
#include <sol/protected_function_result.hpp>
#include <sol/unsafe_function.hpp>
#include <sol/protected_handler.hpp>
#include <sol/bytecode.hpp>
#include <sol/dump_handler.hpp>

#include <cstdint>
#include <algorithm>

namespace sol {

	namespace detail {
		template <bool ShouldPush_, typename Handler_>
		inline void handle_protected_exception(
		     lua_State* L_, optional<const std::exception&> maybe_ex, const char* error, detail::protected_handler<ShouldPush_, Handler_>& handler_) {
			handler_.stack_index = 0;
			if (ShouldPush_) {
				handler_.target.push(L_);
				detail::call_exception_handler(L_, maybe_ex, error);
				lua_call(L_, 1, 1);
			}
			else {
				detail::call_exception_handler(L_, maybe_ex, error);
			}
		}
	} // namespace detail

	template <typename Reference, bool Aligned = false, typename Handler = reference>
	class basic_protected_function : public basic_object<Reference> {
	private:
		using base_t = basic_object<Reference>;
		using handler_t = Handler;
		inline static constexpr bool is_stack_handler_v = is_stack_based_v<handler_t>;

		basic_protected_function(std::true_type, const basic_protected_function& other_) noexcept
		: base_t(other_), m_error_handler(other_.m_error_handler.copy(lua_state())) {
		}

		basic_protected_function(std::false_type, const basic_protected_function& other_) noexcept : base_t(other_), m_error_handler(other_.m_error_handler) {
		}

	public:
		basic_protected_function() = default;
		template <typename T,
		     meta::enable<meta::neg<std::is_same<meta::unqualified_t<T>, basic_protected_function>>,
		          meta::neg<std::is_base_of<proxy_base_tag, meta::unqualified_t<T>>>, meta::neg<std::is_same<base_t, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_protected_function(T&& r) noexcept : base_t(std::forward<T>(r)), m_error_handler(get_default_handler(r.lua_state())) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			if (!is_function<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				constructor_handler handler {};
				stack::check<basic_protected_function>(lua_state(), -1, handler);
			}
#endif // Safety
		}
		basic_protected_function(const basic_protected_function& other_) noexcept
		: basic_protected_function(meta::boolean<is_stateless_lua_reference_v<Handler>>(), other_) {
		}
		basic_protected_function& operator=(const basic_protected_function& other_) {
			base_t::operator=(other_);
			if constexpr (is_stateless_lua_reference_v<Handler>) {
				m_error_handler.copy_assign(lua_state(), other_.m_error_handler);
			}
			else {
				m_error_handler = other_.m_error_handler;
			}
			return *this;
		}
		basic_protected_function(basic_protected_function&&) = default;
		basic_protected_function& operator=(basic_protected_function&&) = default;
		basic_protected_function(const basic_function<base_t>& b) : basic_protected_function(b, get_default_handler(b.lua_state())) {
		}
		basic_protected_function(basic_function<base_t>&& b) : basic_protected_function(std::move(b), get_default_handler(b.lua_state())) {
		}
		basic_protected_function(const basic_function<base_t>& b, handler_t eh) : base_t(b), m_error_handler(std::move(eh)) {
		}
		basic_protected_function(basic_function<base_t>&& b, handler_t eh) : base_t(std::move(b)), m_error_handler(std::move(eh)) {
		}
		basic_protected_function(const stack_reference& r) : basic_protected_function(r.lua_state(), r.stack_index(), get_default_handler(r.lua_state())) {
		}
		basic_protected_function(stack_reference&& r) : basic_protected_function(r.lua_state(), r.stack_index(), get_default_handler(r.lua_state())) {
		}
		basic_protected_function(const stack_reference& r, handler_t eh) : basic_protected_function(r.lua_state(), r.stack_index(), std::move(eh)) {
		}
		basic_protected_function(stack_reference&& r, handler_t eh) : basic_protected_function(r.lua_state(), r.stack_index(), std::move(eh)) {
		}

		template <typename Super>
		basic_protected_function(const proxy_base<Super>& p) : basic_protected_function(p, get_default_handler(p.lua_state())) {
		}
		template <typename Super>
		basic_protected_function(proxy_base<Super>&& p) : basic_protected_function(std::move(p), get_default_handler(p.lua_state())) {
		}
		template <typename Proxy, typename HandlerReference,
		     meta::enable<std::is_base_of<proxy_base_tag, meta::unqualified_t<Proxy>>,
		          meta::neg<is_lua_index<meta::unqualified_t<HandlerReference>>>> = meta::enabler>
		basic_protected_function(Proxy&& p, HandlerReference&& eh)
		: basic_protected_function(detail::force_cast<base_t>(p), make_reference<handler_t>(p.lua_state(), std::forward<HandlerReference>(eh))) {
		}

		template <typename T, meta::enable<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_protected_function(lua_State* L_, T&& r) : basic_protected_function(L_, std::forward<T>(r), get_default_handler(L_)) {
		}
		template <typename T, meta::enable<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_protected_function(lua_State* L_, T&& r, handler_t eh) : base_t(L_, std::forward<T>(r)), m_error_handler(std::move(eh)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_protected_function>(lua_state(), -1, handler);
#endif // Safety
		}

		basic_protected_function(lua_nil_t n) : base_t(n), m_error_handler(n) {
		}

		basic_protected_function(lua_State* L_, int index_ = -1) : basic_protected_function(L_, index_, get_default_handler(L_)) {
		}
		basic_protected_function(lua_State* L_, int index_, handler_t eh) : base_t(L_, index_), m_error_handler(std::move(eh)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_protected_function>(L_, index_, handler);
#endif // Safety
		}
		basic_protected_function(lua_State* L_, absolute_index index_) : basic_protected_function(L_, index_, get_default_handler(L_)) {
		}
		basic_protected_function(lua_State* L_, absolute_index index_, handler_t eh) : base_t(L_, index_), m_error_handler(std::move(eh)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_protected_function>(L_, index_, handler);
#endif // Safety
		}
		basic_protected_function(lua_State* L_, raw_index index_) : basic_protected_function(L_, index_, get_default_handler(L_)) {
		}
		basic_protected_function(lua_State* L_, raw_index index_, handler_t eh) : base_t(L_, index_), m_error_handler(std::move(eh)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_protected_function>(L_, index_, handler);
#endif // Safety
		}
		basic_protected_function(lua_State* L_, ref_index index_) : basic_protected_function(L_, index_, get_default_handler(L_)) {
		}
		basic_protected_function(lua_State* L_, ref_index index_, handler_t eh) : base_t(L_, index_), m_error_handler(std::move(eh)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_protected_function>(lua_state(), -1, handler);
#endif // Safety
		}

		using base_t::lua_state;

		template <typename Fx>
		int dump(lua_Writer writer, void* userdata_pointer_, bool strip, Fx&& on_error) const {
			this->push();
			auto ppn = stack::push_popper_n<false>(this->lua_state(), 1);
			int r = lua_dump(this->lua_state(), writer, userdata_pointer_, strip ? 1 : 0);
			if (r != 0) {
				return on_error(this->lua_state(), r, writer, userdata_pointer_, strip);
			}
			return r;
		}

		int dump(lua_Writer writer, void* userdata_pointer_, bool strip = false) const {
			return dump(writer, userdata_pointer_, strip, &dump_pass_on_error);
		}

		template <typename Container = bytecode>
		Container dump() const {
			Container bc;
			(void)dump(static_cast<lua_Writer>(&basic_insert_dump_writer<Container>), static_cast<void*>(&bc), false, &dump_throw_on_error);
			return bc;
		}

		template <typename Container = bytecode, typename Fx>
		Container dump(Fx&& on_error) const {
			Container bc;
			(void)dump(static_cast<lua_Writer>(&basic_insert_dump_writer<Container>), static_cast<void*>(&bc), false, std::forward<Fx>(on_error));
			return bc;
		}

		template <typename... Args>
		protected_function_result operator()(Args&&... args) const {
			return call<>(std::forward<Args>(args)...);
		}

		template <typename... Ret, typename... Args>
		decltype(auto) operator()(types<Ret...>, Args&&... args) const {
			return call<Ret...>(std::forward<Args>(args)...);
		}

		template <typename... Ret, typename... Args>
		decltype(auto) call(Args&&... args) const {
			if constexpr (!Aligned) {
				// we do not expect the function to already be on the stack: push it
				if (m_error_handler.valid(lua_state())) {
					detail::protected_handler<true, handler_t> h(lua_state(), m_error_handler);
					base_t::push();
					int pushcount = stack::multi_push_reference(lua_state(), std::forward<Args>(args)...);
					return invoke(types<Ret...>(), std::make_index_sequence<sizeof...(Ret)>(), pushcount, h);
				}
				else {
					detail::protected_handler<false, handler_t> h(lua_state(), m_error_handler);
					base_t::push();
					int pushcount = stack::multi_push_reference(lua_state(), std::forward<Args>(args)...);
					return invoke(types<Ret...>(), std::make_index_sequence<sizeof...(Ret)>(), pushcount, h);
				}
			}
			else {
				// the function is already on the stack at the right location
				if (m_error_handler.valid()) {
					// the handler will be pushed onto the stack manually,
					// since it's not already on the stack this means we need to push our own
					// function on the stack too and swap things to be in-place
					if constexpr (!is_stack_handler_v) {
						// so, we need to remove the function at the top and then dump the handler out ourselves
						base_t::push();
					}
					detail::protected_handler<true, handler_t> h(lua_state(), m_error_handler);
					if constexpr (!is_stack_handler_v) {
						lua_replace(lua_state(), -3);
						h.stack_index = lua_absindex(lua_state(), -2);
					}
					int pushcount = stack::multi_push_reference(lua_state(), std::forward<Args>(args)...);
					return invoke(types<Ret...>(), std::make_index_sequence<sizeof...(Ret)>(), pushcount, h);
				}
				else {
					detail::protected_handler<false, handler_t> h(lua_state(), m_error_handler);
					int pushcount = stack::multi_push_reference(lua_state(), std::forward<Args>(args)...);
					return invoke(types<Ret...>(), std::make_index_sequence<sizeof...(Ret)>(), pushcount, h);
				}
			}
		}

		~basic_protected_function() {
			if constexpr (is_stateless_lua_reference_v<handler_t>) {
				this->m_error_handler.reset(lua_state());
			}
		}

		static handler_t get_default_handler(lua_State* L_) {
			return detail::get_default_handler<handler_t, is_main_threaded_v<base_t>>(L_);
		}

		template <typename T>
		static void set_default_handler(const T& ref) {
			detail::set_default_handler(ref.lua_state(), ref);
		}

		auto get_error_handler() const noexcept {
			if constexpr (is_stateless_lua_reference_v<handler_t>) {
				if constexpr (is_stack_based_v<handler_t>) {
					return stack_reference(lua_state(), m_error_handler.stack_index());
				}
				else {
					return basic_reference<is_main_threaded_v<base_t>>(lua_state(), ref_index(m_error_handler.registry_index()));
				}
			}
			else {
				return m_error_handler;
			}
		}

		template <typename ErrorHandler_>
		void set_error_handler(ErrorHandler_&& error_handler_) noexcept {
			static_assert(!is_stack_based_v<handler_t> || is_stack_based_v<ErrorHandler_>,
			     "A stack-based error handler can only be set from a parameter that is also stack-based.");
			if constexpr (std::is_rvalue_reference_v<ErrorHandler_>) {
				m_error_handler = std::forward<ErrorHandler_>(error_handler_);
			}
			else {
				m_error_handler.copy_assign(lua_state(), std::forward<ErrorHandler_>(error_handler_));
			}
		}

		void abandon () noexcept {
			this->m_error_handler.abandon();
			base_t::abandon();
		}

	private:
		handler_t m_error_handler;

		template <bool b>
		call_status luacall(std::ptrdiff_t argcount, std::ptrdiff_t result_count_, detail::protected_handler<b, handler_t>& h) const {
			return static_cast<call_status>(lua_pcall(lua_state(), static_cast<int>(argcount), static_cast<int>(result_count_), h.stack_index));
		}

		template <std::size_t... I, bool b, typename... Ret>
		auto invoke(types<Ret...>, std::index_sequence<I...>, std::ptrdiff_t n, detail::protected_handler<b, handler_t>& h) const {
			luacall(n, sizeof...(Ret), h);
			return stack::pop<std::tuple<Ret...>>(lua_state());
		}

		template <std::size_t I, bool b, typename Ret>
		Ret invoke(types<Ret>, std::index_sequence<I>, std::ptrdiff_t n, detail::protected_handler<b, handler_t>& h) const {
			luacall(n, 1, h);
			return stack::pop<Ret>(lua_state());
		}

		template <std::size_t I, bool b>
		void invoke(types<void>, std::index_sequence<I>, std::ptrdiff_t n, detail::protected_handler<b, handler_t>& h) const {
			luacall(n, 0, h);
		}

		template <bool b>
		protected_function_result invoke(types<>, std::index_sequence<>, std::ptrdiff_t n, detail::protected_handler<b, handler_t>& h) const {
			int stacksize = lua_gettop(lua_state());
			int poststacksize = stacksize;
			int firstreturn = 1;
			int returncount = 0;
			call_status code = call_status::ok;
#if SOL_IS_ON(SOL_EXCEPTIONS) && SOL_IS_OFF(SOL_PROPAGATE_EXCEPTIONS)
			try {
#endif // No Exceptions
				firstreturn = (std::max)(1, static_cast<int>(stacksize - n - static_cast<int>(h.valid() && !is_stack_handler_v)));
				code = luacall(n, LUA_MULTRET, h);
				poststacksize = lua_gettop(lua_state()) - static_cast<int>(h.valid() && !is_stack_handler_v);
				returncount = poststacksize - (firstreturn - 1);
#if SOL_IS_ON(SOL_EXCEPTIONS) && SOL_IS_OFF(SOL_PROPAGATE_EXCEPTIONS)
			}
			// Handle C++ errors thrown from C++ functions bound inside of lua
			catch (const char* error) {
				detail::handle_protected_exception(lua_state(), optional<const std::exception&>(nullopt), error, h);
				firstreturn = lua_gettop(lua_state());
				return protected_function_result(lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
			catch (const std::string& error) {
				detail::handle_protected_exception(lua_state(), optional<const std::exception&>(nullopt), error.c_str(), h);
				firstreturn = lua_gettop(lua_state());
				return protected_function_result(lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
			catch (const std::exception& error) {
				detail::handle_protected_exception(lua_state(), optional<const std::exception&>(error), error.what(), h);
				firstreturn = lua_gettop(lua_state());
				return protected_function_result(lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
#if SOL_IS_ON(SOL_EXCEPTIONS_CATCH_ALL)
			// LuaJIT cannot have the catchall when the safe propagation is on
			// but LuaJIT will swallow all C++ errors
			// if we don't at least catch std::exception ones
			catch (...) {
				detail::handle_protected_exception(lua_state(), optional<const std::exception&>(nullopt), detail::protected_function_error, h);
				firstreturn = lua_gettop(lua_state());
				return protected_function_result(lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
#endif // Always catch edge case
#else
			// do not handle exceptions: they can be propogated into C++ and keep all type information / rich information
#endif // Exceptions vs. No Exceptions
			return protected_function_result(lua_state(), firstreturn, returncount, returncount, code);
		}
	};
} // namespace sol

#endif // SOL_FUNCTION_HPP
