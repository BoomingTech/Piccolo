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

#ifndef SOL_VERSION_HPP
#define SOL_VERSION_HPP

#include <sol/config.hpp>

// clang-format off

#define SOL_VERSION_MAJOR 3
#define SOL_VERSION_MINOR 2
#define SOL_VERSION_PATCH 3
#define SOL_VERSION_STRING "3.2.3"
#define SOL_VERSION ((SOL_VERSION_MAJOR * 100000) + (SOL_VERSION_MINOR * 100) + (SOL_VERSION_PATCH))

#define SOL_TOKEN_TO_STRING_POST_EXPANSION_I_(_TOKEN) #_TOKEN
#define SOL_TOKEN_TO_STRING_I_(_TOKEN) SOL_TOKEN_TO_STRING_POST_EXPANSION_I_(_TOKEN)

#define SOL_CONCAT_TOKENS_POST_EXPANSION_I_(_LEFT, _RIGHT) _LEFT##_RIGHT
#define SOL_CONCAT_TOKENS_I_(_LEFT, _RIGHT) SOL_CONCAT_TOKENS_POST_EXPANSION_I_(_LEFT, _RIGHT)

#define SOL_RAW_IS_ON(OP_SYMBOL) ((3 OP_SYMBOL 3) != 0)
#define SOL_RAW_IS_OFF(OP_SYMBOL) ((3 OP_SYMBOL 3) == 0)
#define SOL_RAW_IS_DEFAULT_ON(OP_SYMBOL) ((3 OP_SYMBOL 3) > 3)
#define SOL_RAW_IS_DEFAULT_OFF(OP_SYMBOL) ((3 OP_SYMBOL 3 OP_SYMBOL 3) < 0)

#define SOL_IS_ON(OP_SYMBOL) SOL_RAW_IS_ON(OP_SYMBOL ## _I_)
#define SOL_IS_OFF(OP_SYMBOL) SOL_RAW_IS_OFF(OP_SYMBOL ## _I_)
#define SOL_IS_DEFAULT_ON(OP_SYMBOL) SOL_RAW_IS_DEFAULT_ON(OP_SYMBOL ## _I_)
#define SOL_IS_DEFAULT_OFF(OP_SYMBOL) SOL_RAW_IS_DEFAULT_OFF(OP_SYMBOL ## _I_)

#define SOL_ON          |
#define SOL_OFF         ^
#define SOL_DEFAULT_ON  +
#define SOL_DEFAULT_OFF -

#if defined(SOL_BUILD_CXX_MODE)
	#if (SOL_BUILD_CXX_MODE != 0)
		#define SOL_BUILD_CXX_MODE_I_ SOL_ON
	#else
		#define SOL_BUILD_CXX_MODE_I_ SOL_OFF
	#endif
#elif defined(__cplusplus)
	#define SOL_BUILD_CXX_MODE_I_ SOL_DEFAULT_ON
#else
	#define SOL_BUILD_CXX_MODE_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_BUILD_C_MODE)
	#if (SOL_BUILD_C_MODE != 0)
		#define SOL_BUILD_C_MODE_I_ SOL_ON
	#else
		#define SOL_BUILD_C_MODE_I_ SOL_OFF
	#endif
#elif defined(__STDC__)
	#define SOL_BUILD_C_MODE_I_ SOL_DEFAULT_ON
#else
	#define SOL_BUILD_C_MODE_I_ SOL_DEFAULT_OFF
#endif

#if SOL_IS_ON(SOL_BUILD_C_MODE)
	#include <stddef.h>
	#include <stdint.h>
	#include <limits.h>
#else
	#include <cstddef>
	#include <cstdint>
	#include <climits>
#endif

#if defined(SOL_COMPILER_VCXX)
	#if defined(SOL_COMPILER_VCXX != 0)
		#define SOL_COMPILER_VCXX_I_ SOL_ON
	#else
		#define SOL_COMPILER_VCXX_I_ SOL_OFF
	#endif
#elif defined(_MSC_VER)
	#define SOL_COMPILER_VCXX_I_ SOL_DEFAULT_ON
#else
	#define SOL_COMPILER_VCXX_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_COMPILER_GCC)
	#if defined(SOL_COMPILER_GCC != 0)
		#define SOL_COMPILER_GCC_I_ SOL_ON
	#else
		#define SOL_COMPILER_GCC_I_ SOL_OFF
	#endif
#elif defined(__GNUC__)
	#define SOL_COMPILER_GCC_I_ SOL_DEFAULT_ON
#else
	#define SOL_COMPILER_GCC_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_COMPILER_CLANG)
	#if defined(SOL_COMPILER_CLANG != 0)
		#define SOL_COMPILER_CLANG_I_ SOL_ON
	#else
		#define SOL_COMPILER_CLANG_I_ SOL_OFF
	#endif
#elif defined(__clang__)
	#define SOL_COMPILER_CLANG_I_ SOL_DEFAULT_ON
#else
	#define SOL_COMPILER_CLANG_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_COMPILER_EDG)
	#if defined(SOL_COMPILER_EDG != 0)
		#define SOL_COMPILER_EDG_I_ SOL_ON
	#else
		#define SOL_COMPILER_EDG_I_ SOL_OFF
	#endif
#else
	#define SOL_COMPILER_EDG_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_COMPILER_MINGW)
	#if (SOL_COMPILER_MINGW != 0)
		#define SOL_COMPILER_MINGW_I_ SOL_ON
	#else
		#define SOL_COMPILER_MINGW_I_ SOL_OFF
	#endif
#elif defined(__MINGW32__)
	#define SOL_COMPILER_MINGW_I_ SOL_DEFAULT_ON
#else
	#define SOL_COMPILER_MINGW_I_ SOL_DEFAULT_OFF
#endif

#if SIZE_MAX <= 0xFFFFULL
	#define SOL_PLATFORM_X16_I_ SOL_ON
	#define SOL_PLATFORM_X86_I_ SOL_OFF
	#define SOL_PLATFORM_X64_I_ SOL_OFF
#elif SIZE_MAX <= 0xFFFFFFFFULL
	#define SOL_PLATFORM_X16_I_ SOL_OFF
	#define SOL_PLATFORM_X86_I_ SOL_ON
	#define SOL_PLATFORM_X64_I_ SOL_OFF
#else
	#define SOL_PLATFORM_X16_I_ SOL_OFF
	#define SOL_PLATFORM_X86_I_ SOL_OFF
	#define SOL_PLATFORM_X64_I_ SOL_ON
#endif

// TODO: ARM codes? So far, not needed explicitly...
#define SOL_PLATFORM_ARM32_I_ SOL_OFF
#define SOL_PLATFORM_ARM64_I_ SOL_OFF

#if defined(SOL_PLATFORM_WINDOWS)
	#if (SOL_PLATFORM_WINDOWS != 0)
		#define SOL_PLATFORM_WINDOWS_I_ SOL_ON
	#else
		#define SOL_PLATFORM_WINDOWS_I_ SOL_OFF
	#endif
#elif defined(_WIN32)
	#define SOL_PLATFORM_WINDOWS_I_ SOL_DEFAULT_ON
#else
	#define SOL_PLATFORM_WINDOWS_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_PLATFORM_CYGWIN)
	#if (SOL_PLATFORM_CYGWIN != 0)
		#define SOL_PLATFORM_CYGWIN_I_ SOL_ON
	#else
		#define SOL_PLATFORM_CYGWIN_I_ SOL_ON
	#endif
#elif defined(__CYGWIN__)
	#define SOL_PLATFORM_CYGWIN_I_ SOL_DEFAULT_ON
#else
	#define SOL_PLATFORM_CYGWIN_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_PLATFORM_APPLE)
	#if (SOL_PLATFORM_APPLE != 0)
		#define SOL_PLATFORM_APPLE_I_ SOL_ON
	#else
		#define SOL_PLATFORM_APPLE_I_ SOL_OFF
	#endif
#elif defined(__APPLE__)
	#define SOL_PLATFORM_APPLE_I_ SOL_DEFAULT_ON
#else
	#define SOL_PLATFORM_APPLE_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_PLATFORM_UNIX)
	#if (SOL_PLATFORM_UNIX != 0)
		#define SOL_PLATFORM_UNIXLIKE_I_ SOL_ON
	#else
		#define SOL_PLATFORM_UNIXLIKE_I_ SOL_OFF
	#endif
#elif defined(__unix__)
	#define SOL_PLATFORM_UNIXLIKE_I_ SOL_DEFAUKT_ON
#else
	#define SOL_PLATFORM_UNIXLIKE_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_PLATFORM_LINUX)
	#if (SOL_PLATFORM_LINUX != 0)
		#define SOL_PLATFORM_LINUXLIKE_I_ SOL_ON
	#else
		#define SOL_PLATFORM_LINUXLIKE_I_ SOL_OFF
	#endif
#elif defined(__LINUX__)
	#define SOL_PLATFORM_LINUXLIKE_I_ SOL_DEFAUKT_ON
#else
	#define SOL_PLATFORM_LINUXLIKE_I_ SOL_DEFAULT_OFF
#endif

#define SOL_PLATFORM_APPLE_IPHONE_I_ SOL_OFF
#define SOL_PLATFORM_BSDLIKE_I_      SOL_OFF

#if defined(SOL_IN_DEBUG_DETECTED)
	#if SOL_IN_DEBUG_DETECTED != 0
		#define SOL_DEBUG_BUILD_I_ SOL_ON
	#else
		#define SOL_DEBUG_BUILD_I_ SOL_OFF
	#endif
#elif !defined(NDEBUG)
	#if SOL_IS_ON(SOL_COMPILER_VCXX) && defined(_DEBUG)
		#define SOL_DEBUG_BUILD_I_ SOL_ON
	#elif (SOL_IS_ON(SOL_COMPILER_CLANG) || SOL_IS_ON(SOL_COMPILER_GCC)) && !defined(__OPTIMIZE__)
		#define SOL_DEBUG_BUILD_I_ SOL_ON
	#else
		#define SOL_DEBUG_BUILD_I_ SOL_OFF
	#endif
#else
	#define SOL_DEBUG_BUILD_I_ SOL_DEFAULT_OFF
#endif // We are in a debug mode of some sort

#if defined(SOL_NO_EXCEPTIONS)
	#if (SOL_NO_EXCEPTIONS != 0)
		#define SOL_EXCEPTIONS_I_ SOL_OFF
	#else
		#define SOL_EXCEPTIONS_I_ SOL_ON
	#endif
#elif SOL_IS_ON(SOL_COMPILER_VCXX)
	#if !defined(_CPPUNWIND)
		#define SOL_EXCEPTIONS_I_ SOL_OFF
	#else
		#define SOL_EXCEPTIONS_I_ SOL_ON
	#endif
#elif SOL_IS_ON(SOL_COMPILER_CLANG) || SOL_IS_ON(SOL_COMPILER_GCC)
	#if !defined(__EXCEPTIONS)
		#define SOL_EXCEPTIONS_I_ SOL_OFF
	#else
		#define SOL_EXCEPTIONS_I_ SOL_ON
	#endif
#else
	#define SOL_EXCEPTIONS_I_ SOL_DEFAULT_ON
#endif


#if defined(SOL_NO_RTTI)
	#if (SOL_NO_RTTI != 0)
		#define SOL_RTTI_I_ SOL_OFF
	#else
		#define SOL_RTTI_I_ SOL_ON
	#endif
#elif SOL_IS_ON(SOL_COMPILER_VCXX)
	#if !defined(_CPPRTTI)
		#define SOL_RTTI_I_ SOL_OFF
	#else
		#define SOL_RTTI_I_ SOL_ON
	#endif
#elif SOL_IS_ON(SOL_COMPILER_CLANG) || SOL_IS_ON(SOL_COMPILER_GCC)
	#if !defined(__GXX_RTTI)
		#define SOL_RTTI_I_ SOL_OFF
	#else
		#define SOL_RTTI_I_ SOL_ON
	#endif
#else
	#define SOL_RTTI_I_ SOL_DEFAULT_ON
#endif

#if defined(SOL_NO_THREAD_LOCAL)
	#if SOL_NO_THREAD_LOCAL != 0
		#define SOL_USE_THREAD_LOCAL_I_ SOL_OFF
	#else
		#define SOL_USE_THREAD_LOCAL_I_ SOL_ON
	#endif
#else
	#define SOL_USE_THREAD_LOCAL_I_ SOL_DEFAULT_ON
#endif // thread_local keyword is bjorked on some platforms

#if defined(SOL_ALL_SAFETIES_ON)
	#if SOL_ALL_SAFETIES_ON != 0
		#define SOL_ALL_SAFETIES_ON_I_ SOL_ON
	#else
		#define SOL_ALL_SAFETIES_ON_I_ SOL_OFF
	#endif
#else
	#define SOL_ALL_SAFETIES_ON_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_SAFE_GETTER)
	#if SOL_SAFE_GETTER != 0
		#define SOL_SAFE_GETTER_I_ SOL_ON
	#else
		#define SOL_SAFE_GETTER_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_GETTER_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_GETTER_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_GETTER_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_SAFE_USERTYPE)
	#if SOL_SAFE_USERTYPE != 0
		#define SOL_SAFE_USERTYPE_I_ SOL_ON
	#else
		#define SOL_SAFE_USERTYPE_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_USERTYPE_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_USERTYPE_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_USERTYPE_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_SAFE_REFERENCES)
	#if SOL_SAFE_REFERENCES != 0
		#define SOL_SAFE_REFERENCES_I_ SOL_ON
	#else
		#define SOL_SAFE_REFERENCES_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_REFERENCES_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_REFERENCES_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_REFERENCES_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_SAFE_FUNCTIONS)
	#if SOL_SAFE_FUNCTIONS != 0
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_ON
	#else
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_OFF
	#endif
#elif defined (SOL_SAFE_FUNCTION_OBJECTS)
	#if SOL_SAFE_FUNCTION_OBJECTS != 0
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_ON
	#else
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_FUNCTION_OBJECTS_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_SAFE_FUNCTION_CALLS)
	#if SOL_SAFE_FUNCTION_CALLS != 0
		#define SOL_SAFE_FUNCTION_CALLS_I_ SOL_ON
	#else
		#define SOL_SAFE_FUNCTION_CALLS_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_FUNCTION_CALLS_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_FUNCTION_CALLS_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_FUNCTION_CALLS_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_SAFE_PROXIES)
	#if SOL_SAFE_PROXIES != 0
		#define SOL_SAFE_PROXIES_I_ SOL_ON
	#else
		#define SOL_SAFE_PROXIES_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_PROXIES_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_PROXIES_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_PROXIES_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_SAFE_NUMERICS)
	#if SOL_SAFE_NUMERICS != 0
		#define SOL_SAFE_NUMERICS_I_ SOL_ON
	#else
		#define SOL_SAFE_NUMERICS_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_NUMERICS_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_NUMERICS_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_NUMERICS_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_ALL_INTEGER_VALUES_FIT)
	#if (SOL_ALL_INTEGER_VALUES_FIT != 0)
		#define SOL_ALL_INTEGER_VALUES_FIT_I_ SOL_ON
	#else
		#define SOL_ALL_INTEGER_VALUES_FIT_I_ SOL_OFF
	#endif
#elif !SOL_IS_DEFAULT_OFF(SOL_SAFE_NUMERICS) && SOL_IS_OFF(SOL_SAFE_NUMERICS)
	// if numerics is intentionally turned off, flip this on
	#define SOL_ALL_INTEGER_VALUES_FIT_I_ SOL_DEFAULT_ON
#else
	// default to off
	#define SOL_ALL_INTEGER_VALUES_FIT_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_SAFE_STACK_CHECK)
	#if SOL_SAFE_STACK_CHECK != 0
		#define SOL_SAFE_STACK_CHECK_I_ SOL_ON
	#else
		#define SOL_SAFE_STACK_CHECK_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_SAFE_STACK_CHECK_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_SAFE_STACK_CHECK_I_ SOL_DEFAULT_ON
	#else
		#define SOL_SAFE_STACK_CHECK_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_NO_CHECK_NUMBER_PRECISION)
	#if SOL_NO_CHECK_NUMBER_PRECISION != 0
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_OFF
	#else
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_ON
	#endif
#elif defined(SOL_NO_CHECKING_NUMBER_PRECISION)
	#if SOL_NO_CHECKING_NUMBER_PRECISION != 0
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_OFF
	#else
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_ON
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_ON
	#elif SOL_IS_ON(SOL_SAFE_NUMERICS)
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_DEFAULT_ON
	#else
		#define SOL_NUMBER_PRECISION_CHECKS_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_STRINGS_ARE_NUMBERS)
	#if (SOL_STRINGS_ARE_NUMBERS != 0)
		#define SOL_STRINGS_ARE_NUMBERS_I_ SOL_ON
	#else
		#define SOL_STRINGS_ARE_NUMBERS_I_ SOL_OFF
	#endif
#else
	#define SOL_STRINGS_ARE_NUMBERS_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_ENABLE_INTEROP)
	#if SOL_ENABLE_INTEROP != 0
		#define SOL_USE_INTEROP_I_ SOL_ON
	#else
		#define SOL_USE_INTEROP_I_ SOL_OFF
	#endif
#elif defined(SOL_USE_INTEROP)
	#if SOL_USE_INTEROP != 0
		#define SOL_USE_INTEROP_I_ SOL_ON
	#else
		#define SOL_USE_INTEROP_I_ SOL_OFF
	#endif
#else
	#define SOL_USE_INTEROP_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_NO_NIL)
	#if (SOL_NO_NIL != 0)
		#define SOL_NIL_I_ SOL_OFF
	#else
		#define SOL_NIL_I_ SOL_ON
	#endif
#elif defined(__MAC_OS_X_VERSION_MAX_ALLOWED) || defined(__OBJC__) || defined(nil)
	#define SOL_NIL_I_ SOL_DEFAULT_OFF
#else
	#define SOL_NIL_I_ SOL_DEFAULT_ON
#endif

#if defined(SOL_USERTYPE_TYPE_BINDING_INFO)
	#if (SOL_USERTYPE_TYPE_BINDING_INFO != 0)
		#define SOL_USERTYPE_TYPE_BINDING_INFO_I_ SOL_ON
	#else
		#define SOL_USERTYPE_TYPE_BINDING_INFO_I_ SOL_OFF
	#endif
#else
	#define SOL_USERTYPE_TYPE_BINDING_INFO_I_ SOL_DEFAULT_ON
#endif // We should generate a my_type.__type table with lots of class information for usertypes

#if defined(SOL_AUTOMAGICAL_TYPES_BY_DEFAULT)
	#if (SOL_AUTOMAGICAL_TYPES_BY_DEFAULT != 0)
		#define SOL_DEFAULT_AUTOMAGICAL_USERTYPES_I_ SOL_ON
	#else
		#define SOL_DEFAULT_AUTOMAGICAL_USERTYPES_I_ SOL_OFF
	#endif
#elif defined(SOL_DEFAULT_AUTOMAGICAL_USERTYPES)
	#if (SOL_DEFAULT_AUTOMAGICAL_USERTYPES != 0)
		#define SOL_DEFAULT_AUTOMAGICAL_USERTYPES_I_ SOL_ON
	#else
		#define SOL_DEFAULT_AUTOMAGICAL_USERTYPES_I_ SOL_OFF
	#endif
#else
	#define SOL_DEFAULT_AUTOMAGICAL_USERTYPES_I_ SOL_DEFAULT_ON
#endif // make is_automagical on/off by default

#if defined(SOL_STD_VARIANT)
	#if (SOL_STD_VARIANT != 0)
		#define SOL_STD_VARIANT_I_ SOL_ON
	#else
		#define SOL_STD_VARIANT_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_COMPILER_CLANG) && SOL_IS_ON(SOL_PLATFORM_APPLE)
		#if defined(__has_include)
			#if __has_include(<variant>)
				#define SOL_STD_VARIANT_I_ SOL_DEFAULT_ON
			#else
				#define SOL_STD_VARIANT_I_ SOL_DEFAULT_OFF
			#endif
		#else
			#define SOL_STD_VARIANT_I_ SOL_DEFAULT_OFF
		#endif
	#else
		#define SOL_STD_VARIANT_I_ SOL_DEFAULT_ON
	#endif
#endif // make is_automagical on/off by default

#if defined(SOL_NOEXCEPT_FUNCTION_TYPE)
	#if (SOL_NOEXCEPT_FUNCTION_TYPE != 0)
		#define SOL_USE_NOEXCEPT_FUNCTION_TYPE_I_ SOL_ON
	#else
		#define SOL_USE_NOEXCEPT_FUNCTION_TYPE_I_ SOL_OFF
	#endif
#else
	#if defined(__cpp_noexcept_function_type)
		#define SOL_USE_NOEXCEPT_FUNCTION_TYPE_I_ SOL_ON
	#elif SOL_IS_ON(SOL_COMPILER_VCXX) && (defined(_MSVC_LANG) && (_MSVC_LANG < 201403L))
		// There is a bug in the VC++ compiler??
		// on /std:c++latest under x86 conditions (VS 15.5.2),
		// compiler errors are tossed for noexcept markings being on function types
		// that are identical in every other way to their non-noexcept marked types function types...
		// VS 2019: There is absolutely a bug.
		#define SOL_USE_NOEXCEPT_FUNCTION_TYPE_I_ SOL_OFF
	#else
		#define SOL_USE_NOEXCEPT_FUNCTION_TYPE_I_ SOL_DEFAULT_ON
	#endif
#endif // noexcept is part of a function's type

#if defined(SOL_STACK_STRING_OPTIMIZATION_SIZE) && SOL_STACK_STRING_OPTIMIZATION_SIZE > 0
	#define SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_ SOL_STACK_STRING_OPTIMIZATION_SIZE
#else
	#define SOL_OPTIMIZATION_STRING_CONVERSION_STACK_SIZE_I_ 1024
#endif

#if defined(SOL_ID_SIZE) && SOL_ID_SIZE > 0
	#define SOL_ID_SIZE_I_ SOL_ID_SIZE
#else
	#define SOL_ID_SIZE_I_ 512
#endif

#if defined(LUA_IDSIZE) && LUA_IDSIZE > 0
	#define SOL_FILE_ID_SIZE_I_ LUA_IDSIZE
#elif defined(SOL_ID_SIZE) && SOL_ID_SIZE > 0
	#define SOL_FILE_ID_SIZE_I_ SOL_FILE_ID_SIZE
#else
	#define SOL_FILE_ID_SIZE_I_ 2048
#endif

#if defined(SOL_PRINT_ERRORS)
	#if (SOL_PRINT_ERRORS != 0)
		#define SOL_PRINT_ERRORS_I_ SOL_ON
	#else
		#define SOL_PRINT_ERRORS_I_ SOL_OFF
	#endif
#else
	#if SOL_IS_ON(SOL_ALL_SAFETIES_ON)
		#define SOL_PRINT_ERRORS_I_ SOL_ON
	#elif SOL_IS_ON(SOL_DEBUG_BUILD)
		#define SOL_PRINT_ERRORS_I_ SOL_DEFAULT_ON
	#else
		#define SOL_PRINT_ERRORS_I_ SOL_OFF
	#endif
#endif

#if defined(SOL_DEFAULT_PASS_ON_ERROR)
	#if (SOL_DEFAULT_PASS_ON_ERROR != 0)
		#define SOL_DEFAULT_PASS_ON_ERROR_I_ SOL_ON
	#else
		#define SOL_DEFAULT_PASS_ON_ERROR_I_ SOL_OFF
	#endif
#else
	#define SOL_DEFAULT_PASS_ON_ERROR_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_USING_CXX_LUA)
	#if (SOL_USING_CXX_LUA != 0)
		#define SOL_USE_CXX_LUA_I_ SOL_ON
	#else
		#define SOL_USE_CXX_LUA_I_ SOL_OFF
	#endif
#elif defined(SOL_USE_CXX_LUA)
	#if (SOL_USE_CXX_LUA != 0)
		#define SOL_USE_CXX_LUA_I_ SOL_ON
	#else
		#define SOL_USE_CXX_LUA_I_ SOL_OFF
	#endif
#else
	#define SOL_USE_CXX_LUA_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_USING_CXX_LUAJIT)
	#if (SOL_USING_CXX_LUA != 0)
		#define SOL_USE_CXX_LUAJIT_I_ SOL_ON
	#else
		#define SOL_USE_CXX_LUAJIT_I_ SOL_OFF
	#endif
#elif defined(SOL_USE_CXX_LUAJIT)
	#if (SOL_USE_CXX_LUA != 0)
		#define SOL_USE_CXX_LUAJIT_I_ SOL_ON
	#else
		#define SOL_USE_CXX_LUAJIT_I_ SOL_OFF
	#endif
#else
	#define SOL_USE_CXX_LUAJIT_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_NO_LUA_HPP)
	#if (SOL_NO_LUA_HPP != 0)
		#define SOL_USE_LUA_HPP_I_ SOL_OFF
	#else
		#define SOL_USE_LUA_HPP_I_ SOL_ON
	#endif
#elif defined(SOL_USING_CXX_LUA)
	#define SOL_USE_LUA_HPP_I_ SOL_OFF
#elif defined(__has_include)
	#if __has_include(<lua.hpp>)
		#define SOL_USE_LUA_HPP_I_ SOL_ON
	#else
		#define SOL_USE_LUA_HPP_I_ SOL_OFF
	#endif
#else
	#define SOL_USE_LUA_HPP_I_ SOL_DEFAULT_ON
#endif

#if defined(SOL_CONTAINERS_START)
	#define SOL_CONTAINER_START_INDEX_I_ SOL_CONTAINERS_START
#elif defined(SOL_CONTAINERS_START_INDEX)
	#define SOL_CONTAINER_START_INDEX_I_ SOL_CONTAINERS_START_INDEX
#elif defined(SOL_CONTAINER_START_INDEX)
	#define SOL_CONTAINER_START_INDEX_I_ SOL_CONTAINER_START_INDEX
#else
	#define SOL_CONTAINER_START_INDEX_I_ 1
#endif

#if defined (SOL_NO_MEMORY_ALIGNMENT)
	#if (SOL_NO_MEMORY_ALIGNMENT != 0)
		#define SOL_ALIGN_MEMORY_I_ SOL_OFF
	#else
		#define SOL_ALIGN_MEMORY_I_ SOL_ON
	#endif
#else
	#define SOL_ALIGN_MEMORY_I_ SOL_DEFAULT_ON
#endif

#if defined(SOL_USE_BOOST)
	#if (SOL_USE_BOOST != 0)
		#define SOL_USE_BOOST_I_ SOL_ON
	#else
		#define SOL_USE_BOOST_I_ SOL_OFF
	#endif
#else
		#define SOL_USE_BOOST_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_USE_UNSAFE_BASE_LOOKUP)
	#if (SOL_USE_UNSAFE_BASE_LOOKUP != 0)
		#define SOL_USE_UNSAFE_BASE_LOOKUP_I_ SOL_ON
	#else
		#define SOL_USE_UNSAFE_BASE_LOOKUP_I_ SOL_OFF
	#endif
#else
	#define SOL_USE_UNSAFE_BASE_LOOKUP_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_INSIDE_UNREAL)
	#if (SOL_INSIDE_UNREAL != 0)
		#define SOL_INSIDE_UNREAL_ENGINE_I_ SOL_ON
	#else
		#define SOL_INSIDE_UNREAL_ENGINE_I_ SOL_OFF
	#endif
#else
	#if defined(UE_BUILD_DEBUG) || defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_TEST) || defined(UE_BUILD_SHIPPING) || defined(UE_SERVER)
		#define SOL_INSIDE_UNREAL_ENGINE_I_ SOL_DEFAULT_ON
	#else
		#define SOL_INSIDE_UNREAL_ENGINE_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if defined(SOL_NO_COMPAT)
	#if (SOL_NO_COMPAT != 0)
		#define SOL_USE_COMPATIBILITY_LAYER_I_ SOL_OFF
	#else
		#define SOL_USE_COMPATIBILITY_LAYER_I_ SOL_ON
	#endif
#else
	#define SOL_USE_COMPATIBILITY_LAYER_I_ SOL_DEFAULT_ON
#endif

#if defined(SOL_GET_FUNCTION_POINTER_UNSAFE)
	#if (SOL_GET_FUNCTION_POINTER_UNSAFE != 0)
		#define SOL_GET_FUNCTION_POINTER_UNSAFE_I_ SOL_ON
	#else
		#define SOL_GET_FUNCTION_POINTER_UNSAFE_I_ SOL_OFF
	#endif
#else
	#define SOL_GET_FUNCTION_POINTER_UNSAFE_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_FUNCTION_CALL_VALUE_SEMANTICS)
	#if (SOL_FUNCTION_CALL_VALUE_SEMANTICS != 0)
		#define SOL_FUNCTION_CALL_VALUE_SEMANTICS_I_ SOL_ON
	#else
		#define SOL_FUNCTION_CALL_VALUE_SEMANTICS_I_ SOL_OFF
	#endif
#else
	#define SOL_FUNCTION_CALL_VALUE_SEMANTICS_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_MINGW_CCTYPE_IS_POISONED)
	#if (SOL_MINGW_CCTYPE_IS_POISONED != 0)
		#define SOL_MINGW_CCTYPE_IS_POISONED_I_ SOL_ON
	#else
		#define SOL_MINGW_CCTYPE_IS_POISONED_I_ SOL_OFF
	#endif
#elif SOL_IS_ON(SOL_COMPILER_MINGW) && defined(__GNUC__) && (__GNUC__ < 6)
	// MinGW is off its rocker in some places...
	#define SOL_MINGW_CCTYPE_IS_POISONED_I_ SOL_DEFAULT_ON
#else
	#define SOL_MINGW_CCTYPE_IS_POISONED_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_CHAR8_T)
	#if (SOL_CHAR8_T != 0)
		#define SOL_CHAR8_T_I_ SOL_ON
	#else
		#define SOL_CHAR8_T_I_ SOL_OFF
	#endif
#else
	#if defined(__cpp_char8_t)
		#define SOL_CHAR8_T_I_ SOL_DEFAULT_ON
	#else
		#define SOL_CHAR8_T_I_ SOL_DEFAULT_OFF
	#endif
#endif

#if SOL_IS_ON(SOL_USE_BOOST)
	#include <boost/version.hpp>

	#if BOOST_VERSION >= 107500 // Since Boost 1.75.0 boost::none is constexpr
		#define SOL_BOOST_NONE_CONSTEXPR_I_ constexpr
	#else
		#define SOL_BOOST_NONE_CONSTEXPR_I_ const
	#endif // BOOST_VERSION
#else
	// assume boost isn't using a garbage version
	#define SOL_BOOST_NONE_CONSTEXPR_I_ constexpr
#endif

#if defined(SOL2_CI)
	#if (SOL2_CI != 0)
		#define SOL2_CI_I_ SOL_ON
	#else
		#define SOL2_CI_I_ SOL_OFF
	#endif
#else
	#define SOL2_CI_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_C_ASSERT)
	#define SOL_USER_C_ASSERT_I_ SOL_ON
#else
	#define SOL_USER_C_ASSERT_I_ SOL_DEFAULT_OFF
#endif

#if defined(SOL_M_ASSERT)
	#define SOL_USER_M_ASSERT_I_ SOL_ON
#else
	#define SOL_USER_M_ASSERT_I_ SOL_DEFAULT_OFF
#endif

#include <sol/prologue.hpp>
#include <sol/epilogue.hpp>

// clang-format on

#include <sol/detail/build_version.hpp>

#endif // SOL_VERSION_HPP
