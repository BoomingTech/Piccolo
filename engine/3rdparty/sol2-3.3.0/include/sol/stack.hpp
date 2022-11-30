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

#ifndef SOL_STACK_HPP
#define SOL_STACK_HPP

#include <sol/trampoline.hpp>
#include <sol/stack_core.hpp>
#include <sol/stack_reference.hpp>
#include <sol/stack_check.hpp>
#include <sol/stack_get.hpp>
#include <sol/stack_check_get.hpp>
#include <sol/stack_push.hpp>
#include <sol/stack_pop.hpp>
#include <sol/stack_field.hpp>
#include <sol/stack_probe.hpp>
#include <sol/assert.hpp>

#include <cstring>
#include <array>

namespace sol {
	namespace detail {
		using typical_chunk_name_t = char[SOL_ID_SIZE_I_];
		using typical_file_chunk_name_t = char[SOL_FILE_ID_SIZE_I_];

		inline const std::string& default_chunk_name() {
			static const std::string name = "";
			return name;
		}

		template <std::size_t N>
		const char* make_chunk_name(const string_view& code, const std::string& chunkname, char (&basechunkname)[N]) {
			if (chunkname.empty()) {
				auto it = code.cbegin();
				auto e = code.cend();
				std::size_t i = 0;
				static const std::size_t n = N - 4;
				for (i = 0; i < n && it != e; ++i, ++it) {
					basechunkname[i] = *it;
				}
				if (it != e) {
					for (std::size_t c = 0; c < 3; ++i, ++c) {
						basechunkname[i] = '.';
					}
				}
				basechunkname[i] = '\0';
				return &basechunkname[0];
			}
			else {
				return chunkname.c_str();
			}
		}

		inline void clear_entries(stack_reference r) {
			stack::push(r.lua_state(), lua_nil);
			while (lua_next(r.lua_state(), -2)) {
				absolute_index key(r.lua_state(), -2);
				auto pn = stack::pop_n(r.lua_state(), 1);
				stack::set_field<false, true>(r.lua_state(), key, lua_nil, r.stack_index());
			}
		}

		inline void clear_entries(const reference& registry_reference) {
			auto pp = stack::push_pop(registry_reference);
			stack_reference ref(registry_reference.lua_state(), -1);
			clear_entries(ref);
		}
	} // namespace detail

	namespace stack {
		namespace stack_detail {
			template <typename T>
			inline int push_as_upvalues(lua_State* L, T& item) {
				typedef std::decay_t<T> TValue;
				static const std::size_t itemsize = sizeof(TValue);
				static const std::size_t voidsize = sizeof(void*);
				static const std::size_t voidsizem1 = voidsize - 1;
				static const std::size_t data_t_count = (sizeof(TValue) + voidsizem1) / voidsize;
				typedef std::array<void*, data_t_count> data_t;

				data_t data { {} };
				std::memcpy(&data[0], std::addressof(item), itemsize);
				int pushcount = 0;
				for (const auto& v : data) {
					lua_pushlightuserdata(L, v);
					pushcount += 1;
				}
				return pushcount;
			}

			template <typename T>
			inline std::pair<T, int> get_as_upvalues(lua_State* L, int index = 2) {
				static const std::size_t data_t_count = (sizeof(T) + (sizeof(void*) - 1)) / sizeof(void*);
				typedef std::array<void*, data_t_count> data_t;
				data_t voiddata { {} };
				for (std::size_t i = 0, d = 0; d < sizeof(T); ++i, d += sizeof(void*)) {
					voiddata[i] = lua_touserdata(L, upvalue_index(index++));
				}
				return std::pair<T, int>(*reinterpret_cast<T*>(static_cast<void*>(voiddata.data())), index);
			}

			template <typename T>
			inline std::pair<T, int> get_as_upvalues_using_function(lua_State* L, int function_index = -1) {
				static const std::size_t data_t_count = (sizeof(T) + (sizeof(void*) - 1)) / sizeof(void*);
				typedef std::array<void*, data_t_count> data_t;
				function_index = lua_absindex(L, function_index);
				int index = 0;
				data_t voiddata { {} };
				for (std::size_t d = 0; d < sizeof(T); d += sizeof(void*)) {
					// first upvalue is nullptr to respect environment shenanigans
					// So +2 instead of +1
					const char* upvalue_name = lua_getupvalue(L, function_index, index + 2);
					if (upvalue_name == nullptr) {
						// We should freak out here...
						break;
					}
					voiddata[index] = lua_touserdata(L, -1);
					++index;
				}
				lua_pop(L, index);
				return std::pair<T, int>(*reinterpret_cast<T*>(static_cast<void*>(voiddata.data())), index);
			}

			template <bool checked, typename Handler, typename Fx, typename... Args>
			static decltype(auto) eval(types<>, std::index_sequence<>, lua_State*, int, Handler&&, record&, Fx&& fx, Args&&... args) {
				return std::forward<Fx>(fx)(std::forward<Args>(args)...);
			}

			template <bool checked, typename Arg, typename... Args, std::size_t I, std::size_t... Is, typename Handler, typename Fx, typename... FxArgs>
			static decltype(auto) eval(types<Arg, Args...>, std::index_sequence<I, Is...>, lua_State* L_, int start_index_, Handler&& handler_,
			     record& tracking_, Fx&& fx_, FxArgs&&... fxargs_) {
#if SOL_IS_ON(SOL_PROPAGATE_EXCEPTIONS)
				// We can save performance/time by letting errors unwind produced arguments
				// rather than checking everything once, and then potentially re-doing work
				if constexpr (checked) {
					return eval<checked>(types<Args...>(),
					     std::index_sequence<Is...>(),
					     L_,
					     start_index_,
					     std::forward<Handler>(handler_),
					     tracking_,
					     std::forward<Fx>(fx_),
					     std::forward<FxArgs>(fxargs_)...,
					     *stack_detail::check_get_arg<Arg>(L_, start_index_ + tracking_.used, handler_, tracking_));
				}
				else
#endif
				{
					return eval<checked>(types<Args...>(),
					     std::index_sequence<Is...>(),
					     L_,
					     start_index_,
					     std::forward<Handler>(handler_),
					     tracking_,
					     std::forward<Fx>(fx_),
					     std::forward<FxArgs>(fxargs_)...,
					     stack_detail::unchecked_get_arg<Arg>(L_, start_index_ + tracking_.used, tracking_));
				}
			}

			template <bool checkargs = detail::default_safe_function_calls, std::size_t... I, typename R, typename... Args, typename Fx, typename... FxArgs>
			inline decltype(auto) call(types<R>, types<Args...> argument_types_, std::index_sequence<I...> argument_indices_, lua_State* L_,
			     int start_index_, Fx&& fx_, FxArgs&&... args_) {
				static_assert(meta::all_v<meta::is_not_move_only<Args>...>,
				     "One of the arguments being bound is a move-only type, and it is not being taken by reference: this will break your code. Please take "
				     "a reference and std::move it manually if this was your intention.");
				argument_handler<types<R, Args...>> handler {};
				record tracking {};
#if SOL_IS_OFF(SOL_PROPAGATE_EXCEPTIONS)
				if constexpr (checkargs) {
					multi_check<Args...>(L_, start_index_, handler);
				}
#endif
				if constexpr (std::is_void_v<R>) {
					eval<checkargs>(
					     argument_types_, argument_indices_, L_, start_index_, handler, tracking, std::forward<Fx>(fx_), std::forward<FxArgs>(args_)...);
				}
				else {
					return eval<checkargs>(
					     argument_types_, argument_indices_, L_, start_index_, handler, tracking, std::forward<Fx>(fx_), std::forward<FxArgs>(args_)...);
				}
			}

			template <typename T>
			void raw_table_set(lua_State* L, T&& arg, int tableindex = -2) {
				int push_count = push(L, std::forward<T>(arg));
				sol_c_assert(push_count == 1);
				std::size_t unique_index = static_cast<std::size_t>(luaL_len(L, tableindex) + 1u);
				lua_rawseti(L, tableindex, unique_index);
			}

		} // namespace stack_detail

		template <typename T>
		int set_ref(lua_State* L, T&& arg, int tableindex = -2) {
			int push_count = push(L, std::forward<T>(arg));
			sol_c_assert(push_count == 1);
			return luaL_ref(L, tableindex);
		}

		template <bool check_args = detail::default_safe_function_calls, typename R, typename... Args, typename Fx, typename... FxArgs>
		inline decltype(auto) call(types<R> tr, types<Args...> ta, lua_State* L, int start, Fx&& fx, FxArgs&&... args) {
			using args_indices = std::make_index_sequence<sizeof...(Args)>;
			if constexpr (std::is_void_v<R>) {
				stack_detail::call<check_args>(tr, ta, args_indices(), L, start, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
			}
			else {
				return stack_detail::call<check_args>(tr, ta, args_indices(), L, start, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
			}
		}

		template <bool check_args = detail::default_safe_function_calls, typename R, typename... Args, typename Fx, typename... FxArgs>
		inline decltype(auto) call(types<R> tr, types<Args...> ta, lua_State* L, Fx&& fx, FxArgs&&... args) {
			if constexpr (std::is_void_v<R>) {
				call<check_args>(tr, ta, L, 1, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
			}
			else {
				return call<check_args>(tr, ta, L, 1, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
			}
		}

		template <bool check_args = detail::default_safe_function_calls, typename R, typename... Args, typename Fx, typename... FxArgs>
		inline decltype(auto) call_from_top(types<R> tr, types<Args...> ta, lua_State* L, Fx&& fx, FxArgs&&... args) {
			using expected_count_t = meta::count_for_pack<lua_size, Args...>;
			if constexpr (std::is_void_v<R>) {
				call<check_args>(tr,
				     ta,
				     L,
				     (std::max)(static_cast<int>(lua_gettop(L) - expected_count_t::value), static_cast<int>(0)),
				     std::forward<Fx>(fx),
				     std::forward<FxArgs>(args)...);
			}
			else {
				return call<check_args>(tr,
				     ta,
				     L,
				     (std::max)(static_cast<int>(lua_gettop(L) - expected_count_t::value), static_cast<int>(0)),
				     std::forward<Fx>(fx),
				     std::forward<FxArgs>(args)...);
			}
		}

		template <bool check_args = detail::default_safe_function_calls, bool clean_stack = true, typename Ret0, typename... Ret, typename... Args,
		     typename Fx, typename... FxArgs>
		inline int call_into_lua(types<Ret0, Ret...> tr, types<Args...> ta, lua_State* L, int start, Fx&& fx, FxArgs&&... fxargs) {
			if constexpr (std::is_void_v<Ret0>) {
				call<check_args>(tr, ta, L, start, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)...);
				if constexpr (clean_stack) {
					lua_settop(L, 0);
				}
				return 0;
			}
			else {
				(void)tr;
				decltype(auto) r
				     = call<check_args>(types<meta::return_type_t<Ret0, Ret...>>(), ta, L, start, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)...);
				using R = meta::unqualified_t<decltype(r)>;
				using is_stack = meta::any<is_stack_based<R>, std::is_same<R, absolute_index>, std::is_same<R, ref_index>, std::is_same<R, raw_index>>;
				if constexpr (clean_stack && !is_stack::value) {
					lua_settop(L, 0);
				}
				return push_reference(L, std::forward<decltype(r)>(r));
			}
		}

		template <bool check_args = detail::default_safe_function_calls, bool clean_stack = true, typename Fx, typename... FxArgs>
		inline int call_lua(lua_State* L, int start, Fx&& fx, FxArgs&&... fxargs) {
			using traits_type = lua_bind_traits<meta::unqualified_t<Fx>>;
			using args_list = typename traits_type::args_list;
			using returns_list = typename traits_type::returns_list;
			return call_into_lua<check_args, clean_stack>(returns_list(), args_list(), L, start, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)...);
		}

		inline call_syntax get_call_syntax(lua_State* L, const string_view& key, int index) {
			if (lua_gettop(L) < 1) {
				return call_syntax::dot;
			}
			luaL_getmetatable(L, key.data());
			auto pn = pop_n(L, 1);
			if (lua_compare(L, -1, index, LUA_OPEQ) != 1) {
				return call_syntax::dot;
			}
			return call_syntax::colon;
		}

		inline void script(
		     lua_State* L, lua_Reader reader, void* data, const std::string& chunkname = detail::default_chunk_name(), load_mode mode = load_mode::any) {
			detail::typical_chunk_name_t basechunkname = {};
			const char* chunknametarget = detail::make_chunk_name("lua_Reader", chunkname, basechunkname);
			if (lua_load(L, reader, data, chunknametarget, to_string(mode).c_str()) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
				lua_error(L);
			}
		}

		inline void script(
		     lua_State* L, const string_view& code, const std::string& chunkname = detail::default_chunk_name(), load_mode mode = load_mode::any) {

			detail::typical_chunk_name_t basechunkname = {};
			const char* chunknametarget = detail::make_chunk_name(code, chunkname, basechunkname);
			if (luaL_loadbufferx(L, code.data(), code.size(), chunknametarget, to_string(mode).c_str()) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
				lua_error(L);
			}
		}

		inline void script_file(lua_State* L, const std::string& filename, load_mode mode = load_mode::any) {
			if (luaL_loadfilex(L, filename.c_str(), to_string(mode).c_str()) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
				lua_error(L);
			}
		}

		inline void luajit_exception_handler(lua_State* L, int (*handler)(lua_State*, lua_CFunction) = detail::c_trampoline) {
#if SOL_IS_ON(SOL_USE_LUAJIT_EXCEPTION_TRAMPOLINE)
			if (L == nullptr) {
				return;
			}
#if SOL_IS_ON(SOL_SAFE_STACK_CHECK)
			luaL_checkstack(L, 1, detail::not_enough_stack_space_generic);
#endif // make sure stack doesn't overflow
			lua_pushlightuserdata(L, (void*)handler);
			auto pn = pop_n(L, 1);
			luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
#else
			(void)L;
			(void)handler;
#endif
		}

		inline void luajit_exception_off(lua_State* L) {
#if SOL_IS_ON(SOL_USE_LUAJIT_EXCEPTION_TRAMPOLINE)
			if (L == nullptr) {
				return;
			}
			luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_OFF);
#else
			(void)L;
#endif
		}
	} // namespace stack
} // namespace sol

#endif // SOL_STACK_HPP
