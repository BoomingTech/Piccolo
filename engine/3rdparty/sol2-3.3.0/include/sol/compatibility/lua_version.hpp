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

#ifndef SOL_COMPATIBILITY_VERSION_HPP
#define SOL_COMPATIBILITY_VERSION_HPP

#include <sol/version.hpp>

// clang-format off

#if SOL_IS_ON(SOL_USE_CXX_LUA)
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
#elif SOL_IS_ON(SOL_USE_LUA_HPP)
	#include <lua.hpp>
#else
	extern "C" {
		#include <lua.h>
		#include <lauxlib.h>
		#include <lualib.h>
	}
#endif // C++ Mangling for Lua vs. Not

#if defined(SOL_LUAJIT)
	#if (SOL_LUAJIT != 0)
		#define SOL_USE_LUAJIT_I_ SOL_ON
	#else
		#define SOL_USE_LUAJIT_I_ SOL_OFF
	#endif
#elif defined(LUAJIT_VERSION)
	#define SOL_USE_LUAJIT_I_ SOL_ON
#else
	#define SOL_USE_LUAJIT_I_ SOL_DEFAULT_OFF
#endif // luajit

#if SOL_IS_ON(SOL_USE_CXX_LUAJIT)
	#include <luajit.h>
#elif SOL_IS_ON(SOL_USE_LUAJIT)
	extern "C" {
		#include <luajit.h>
	}
#endif // C++ LuaJIT ... whatever that means

#if defined(SOL_LUAJIT_VERSION)
	#define SOL_LUAJIT_VERSION_I_ SOL_LUAJIT_VERSION
#elif SOL_IS_ON(SOL_USE_LUAJIT)
	#define SOL_LUAJIT_VERSION_I_ LUAJIT_VERSION_NUM
#else
	#define SOL_LUAJIT_VERSION_I_ 0
#endif

#if defined(SOL_LUAJIT_FFI_DISABLED)
	#define SOL_LUAJIT_FFI_DISABLED_I_ SOL_ON
#elif defined(LUAJIT_DISABLE_FFI)
	#define SOL_LUAJIT_FFI_DISABLED_I_ SOL_ON
#else
	#define SOL_LUAJIT_FFI_DISABLED_I_ SOL_DEFAULT_OFF
#endif

#if defined(MOONJIT_VERSION)
	#define SOL_USE_MOONJIT_I_ SOL_ON
#else
	#define SOL_USE_MOONJIT_I_ SOL_OFF
#endif

#if !defined(SOL_LUA_VERSION)
	#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 502
		#define SOL_LUA_VERSION LUA_VERSION_NUM
	#elif defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 501
		#define SOL_LUA_VERSION LUA_VERSION_NUM
	#elif !defined(LUA_VERSION_NUM) || !(LUA_VERSION_NUM)
		// Definitely 5.0
		#define SOL_LUA_VERSION 500
	#else
		// ??? Not sure, assume latest?
		#define SOL_LUA_VERSION 504
	#endif // Lua Version 503, 502, 501 || luajit, 500
#endif // SOL_LUA_VERSION

#if defined(SOL_LUA_VERSION)
	#define SOL_LUA_VERSION_I_ SOL_LUA_VERSION
#else
	#define SOL_LUA_VERSION_I_ 504
#endif

// Exception safety / propagation, according to Lua information
// and user defines. Note this can sometimes change based on version information...
#if defined(SOL_EXCEPTIONS_ALWAYS_UNSAFE)
	#if (SOL_EXCEPTIONS_ALWAYS_UNSAFE != 0)
		#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_OFF
	#else
		#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_ON
	#endif
#elif defined(SOL_EXCEPTIONS_SAFE_PROPAGATION)
	#if (SOL_EXCEPTIONS_SAFE_PROPAGATION != 0)
		#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_ON
	#else
		#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_OFF
	#endif
#elif SOL_LUAJIT_VERSION_I_ >= 20100
	// LuaJIT 2.1.0-beta3 and better have exception support locked in for all platforms (mostly)
	#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_DEFAULT_ON
#elif SOL_LUAJIT_VERSION_I_ >= 20000
	// LuaJIT 2.0.x have exception support only on x64 builds
	#if SOL_IS_ON(SOL_PLATFORM_X64)
		#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_DEFAULT_ON
	#else
		#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_OFF
	#endif
#else
	// otherwise, there is no exception safety for
	// shoving exceptions through Lua and errors should
	// always be serialized
	#define SOL_PROPAGATE_EXCEPTIONS_I_ SOL_DEFAULT_OFF
#endif

// Some configurations work with exceptions,
// but cannot catch-all everything...
#if defined(SOL_EXCEPTIONS_CATCH_ALL)
	#if (SOL_EXCEPTIONS_CATCH_ALL != 0)
		#define SOL_EXCEPTIONS_CATCH_ALL_I_ SOL_ON
	#else
		#define SOL_EXCEPTIONS_CATCH_ALL_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_USE_LUAJIT)
		#define SOL_EXCEPTIONS_CATCH_ALL_I_ SOL_DEFAULT_OFF
	#elif SOL_IS_ON(SOL_USE_CXX_LUAJIT)
		#define SOL_EXCEPTIONS_CATCH_ALL_I_ SOL_DEFAULT_OFF
	#elif SOL_IS_ON(SOL_USE_CXX_LUA)
		#define SOL_EXCEPTIONS_CATCH_ALL_I_ SOL_DEFAULT_OFF
	#else
		#define SOL_EXCEPTIONS_CATCH_ALL_I_ SOL_DEFAULT_ON
	#endif
#endif

#if defined(SOL_LUAJIT_USE_EXCEPTION_TRAMPOLINE)
	#if (SOL_LUAJIT_USE_EXCEPTION_TRAMPOLINE != 0)
		#define SOL_USE_LUAJIT_EXCEPTION_TRAMPOLINE_I_ SOL_ON
	#else
		#define SOL_USE_LUAJIT_EXCEPTION_TRAMPOLINE_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_OFF(SOL_PROPAGATE_EXCEPTIONS) && SOL_IS_ON(SOL_USE_LUAJIT)
		#define SOL_USE_LUAJIT_EXCEPTION_TRAMPOLINE_I_ SOL_ON
	#else
		#define SOL_USE_LUAJIT_EXCEPTION_TRAMPOLINE_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_LUAL_STREAM_HAS_CLOSE_FUNCTION)
	#if (SOL_LUAL_STREAM_HAS_CLOSE_FUNCTION != 0)
		#define SOL_LUAL_STREAM_USE_CLOSE_FUNCTION_I_ SOL_ON
	#else
		#define SOL_LUAL_STREAM_USE_CLOSE_FUNCTION_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_OFF(SOL_USE_LUAJIT) && (SOL_LUA_VERSION > 501)
		#define SOL_LUAL_STREAM_USE_CLOSE_FUNCTION_I_ SOL_ON
	#else
		#define SOL_LUAL_STREAM_USE_CLOSE_FUNCTION_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined (SOL_LUA_BIT32_LIB)
	#if SOL_LUA_BIT32_LIB != 0
		#define SOL_LUA_BIT32_LIB_I_ SOL_ON
	#else
		#define SOL_LUA_BIT32_LIB_I_ SOL_OFF
	#endif
#else
	// Lua 5.2 only (deprecated in 5.3 (503)) (Can be turned on with Compat flags)
	// Lua 5.2, or other versions of Lua with the compat flag, or Lua that is not 5.2 with the specific define (5.4.1 either removed it entirely or broke it)
	#if (SOL_LUA_VERSION_I_ == 502) || (defined(LUA_COMPAT_BITLIB) && (LUA_COMPAT_BITLIB != 0)) || (SOL_LUA_VERSION_I_ < 504 && (defined(LUA_COMPAT_5_2) && (LUA_COMPAT_5_2 != 0)))
		#define SOL_LUA_BIT32_LIB_I_ SOL_ON
	#else
		#define SOL_LUA_BIT32_LIB_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined (SOL_LUA_NIL_IN_TABLES)
	#if SOL_LUA_NIL_IN_TABLES != 0
		#define SOL_LUA_NIL_IN_TABLES_I_ SOL_ON
	#else
		#define SOL_LUA_NIL_IN_TABLES_I_ SOL_OFF
	#endif
#else
	#if defined(LUA_NILINTABLE) && (LUA_NILINTABLE != 0)
		#define SOL_LUA_NIL_IN_TABLES_I_ SOL_DEFAULT_ON
	#else
		#define SOL_LUA_NIL_IN_TABLES_I_ SOL_DEFAULT_OFF
	#endif
#endif

// clang-format on

#endif // SOL_COMPATIBILITY_VERSION_HPP
