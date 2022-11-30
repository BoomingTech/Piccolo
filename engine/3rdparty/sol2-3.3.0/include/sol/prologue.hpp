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

#pragma once

// clang-format off

#if defined(SOL_PROLOGUE_I_)
	#error "[sol2] Library Prologue was already included in translation unit and not properly ended with an epilogue."
#endif

#define SOL_PROLOGUE_I_ 1


#if SOL_IS_ON(SOL_BUILD_CXX_MODE)
	#define _FWD(...) static_cast<decltype( __VA_ARGS__ )&&>( __VA_ARGS__ )

	#if SOL_IS_ON(SOL_COMPILER_GCC) || SOL_IS_ON(SOL_COMPILER_CLANG)
		#define _MOVE(...) static_cast<__typeof( __VA_ARGS__ )&&>( __VA_ARGS__ )
	#else
		#include <type_traits>
		
		#define _MOVE(...) static_cast<::std::remove_reference_t<( __VA_ARGS__ )>&&>( __VA_OPT__(,) )
	#endif
#endif

// clang-format on
