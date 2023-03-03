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

#ifndef SOL_TRAMPOLINE_HPP
#define SOL_TRAMPOLINE_HPP

#include <sol/types.hpp>
#include <sol/traits.hpp>
#include <exception>
#include <cstring>

#if SOL_IS_ON(SOL_PRINT_ERRORS)
#include <iostream>
#endif

namespace sol {
	// must push a single object to be the error object
	// NOTE: the VAST MAJORITY of all Lua libraries -- C or otherwise -- expect a string for the type of error
	// break this convention at your own risk
	using exception_handler_function = int (*)(lua_State*, optional<const std::exception&>, string_view);

	namespace detail {
		inline const char (&default_exception_handler_name())[11] {
			static const char name[11] = "sol.\xE2\x98\xA2\xE2\x98\xA2";
			return name;
		}

		// must push at least 1 object on the stack
		inline int default_exception_handler(lua_State* L, optional<const std::exception&>, string_view what) {
#if SOL_IS_ON(SOL_PRINT_ERRORS)
			std::cerr << "[sol2] An exception occurred: ";
			std::cerr.write(what.data(), static_cast<std::streamsize>(what.size()));
			std::cerr << std::endl;
#endif
			lua_pushlstring(L, what.data(), what.size());
			return 1;
		}

		inline int call_exception_handler(lua_State* L, optional<const std::exception&> maybe_ex, string_view what) {
			lua_getglobal(L, default_exception_handler_name());
			type t = static_cast<type>(lua_type(L, -1));
			if (t != type::lightuserdata) {
				lua_pop(L, 1);
				return default_exception_handler(L, std::move(maybe_ex), std::move(what));
			}
			void* vfunc = lua_touserdata(L, -1);
			lua_pop(L, 1);
			if (vfunc == nullptr) {
				return default_exception_handler(L, std::move(maybe_ex), std::move(what));
			}
			exception_handler_function exfunc = reinterpret_cast<exception_handler_function>(vfunc);
			return exfunc(L, std::move(maybe_ex), std::move(what));
		}

#if SOL_IS_OFF(SOL_EXCEPTIONS)
		template <lua_CFunction f>
		int static_trampoline(lua_State* L) noexcept {
			return f(L);
		}

#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
		template <lua_CFunction_noexcept f>
		int static_trampoline_noexcept(lua_State* L) noexcept {
			return f(L);
		}
#else
		template <lua_CFunction f>
		int static_trampoline_noexcept(lua_State* L) noexcept {
			return f(L);
		}
#endif

		template <typename Fx, typename... Args>
		int trampoline(lua_State* L, Fx&& f, Args&&... args) noexcept {
			return f(L, std::forward<Args>(args)...);
		}

		inline int c_trampoline(lua_State* L, lua_CFunction f) noexcept {
			return trampoline(L, f);
		}
#else

		inline int lua_cfunction_trampoline(lua_State* L, lua_CFunction f) {
#if SOL_IS_ON(SOL_PROPAGATE_EXCEPTIONS)
			return f(L);
#else
			try {
				return f(L);
			}
			catch (const char* cs) {
				call_exception_handler(L, optional<const std::exception&>(nullopt), string_view(cs));
			}
			catch (const std::string& s) {
				call_exception_handler(L, optional<const std::exception&>(nullopt), string_view(s.c_str(), s.size()));
			}
			catch (const std::exception& e) {
				call_exception_handler(L, optional<const std::exception&>(e), e.what());
			}
#if SOL_IS_ON(SOL_EXCEPTIONS_CATCH_ALL)
			// LuaJIT cannot have the catchall when the safe propagation is on
			// but LuaJIT will swallow all C++ errors
			// if we don't at least catch std::exception ones
			catch (...) {
				call_exception_handler(L, optional<const std::exception&>(nullopt), "caught (...) exception");
			}
#endif // LuaJIT cannot have the catchall, but we must catch std::exceps for it
			return lua_error(L);
#endif // Safe exceptions
		}

		template <lua_CFunction f>
		int static_trampoline(lua_State* L) {
			return lua_cfunction_trampoline(L, f);
		}

#if SOL_IS_ON(SOL_USE_NOEXCEPT_FUNCTION_TYPE)
		template <lua_CFunction_noexcept f>
		int static_trampoline_noexcept(lua_State* L) noexcept {
			return f(L);
		}
#else
		template <lua_CFunction f>
		int static_trampoline_noexcept(lua_State* L) noexcept {
			return f(L);
		}
#endif

		template <typename Fx, typename... Args>
		int trampoline(lua_State* L, Fx&& f, Args&&... args) {
			if constexpr (meta::bind_traits<meta::unqualified_t<Fx>>::is_noexcept) {
				return f(L, std::forward<Args>(args)...);
			}
			else {
#if SOL_IS_ON(SOL_PROPAGATE_EXCEPTIONS)
				return f(L, std::forward<Args>(args)...);
#else
				try {
					return f(L, std::forward<Args>(args)...);
				}
				catch (const char* cs) {
					call_exception_handler(L, optional<const std::exception&>(nullopt), string_view(cs));
				}
				catch (const std::string& s) {
					call_exception_handler(L, optional<const std::exception&>(nullopt), string_view(s.c_str(), s.size()));
				}
				catch (const std::exception& e) {
					call_exception_handler(L, optional<const std::exception&>(e), e.what());
				}
#if SOL_IS_ON(SOL_EXCEPTIONS_CATCH_ALL)
				// LuaJIT cannot have the catchall when the safe propagation is on
				// but LuaJIT will swallow all C++ errors
				// if we don't at least catch std::exception ones
				catch (...) {
					call_exception_handler(L, optional<const std::exception&>(nullopt), "caught (...) exception");
				}
#endif
				return lua_error(L);
#endif
			}
		}

		inline int c_trampoline(lua_State* L, lua_CFunction f) {
			return trampoline(L, f);
		}
#endif // Exceptions vs. No Exceptions

		template <typename F, F fx>
		inline int typed_static_trampoline(lua_State* L) {
#if 0
			// TODO: you must evaluate the get/check_get of every
			// argument, to ensure it doesn't throw
			// (e.g., for the sol_lua_check_access extension point!)
			// This incluudes properly noexcept-ing all the above
			// trampolines / safety nets
			if constexpr (meta::bind_traits<F>::is_noexcept) {
				return static_trampoline_noexcept<fx>(L);
			}
			else
#endif
			{ return static_trampoline<fx>(L); }
		}
	} // namespace detail

	inline void set_default_exception_handler(lua_State* L, exception_handler_function exf = &detail::default_exception_handler) {
		static_assert(sizeof(void*) >= sizeof(exception_handler_function),
		     "void* storage is too small to transport the exception handler: please file a bug on the sol2 issue tracker to get this looked at!");
		void* storage;
		std::memcpy(&storage, &exf, sizeof(exception_handler_function));
		lua_pushlightuserdata(L, storage);
		lua_setglobal(L, detail::default_exception_handler_name());
	}
} // namespace sol

#endif // SOL_TRAMPOLINE_HPP
