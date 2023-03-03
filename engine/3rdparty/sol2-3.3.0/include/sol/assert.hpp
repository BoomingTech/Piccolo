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

#ifndef SOL_ASSERT_HPP
#define SOL_ASSERT_HPP

#include <sol/forward.hpp>

#if SOL_IS_ON(SOL2_CI)

struct pre_main {
	pre_main() {
#ifdef _MSC_VER
		_set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
	}
} inline sol2_ci_dont_lock_ci_please = {};

#endif // Prevent lockup when doing Continuous Integration


// clang-format off

#if SOL_IS_ON(SOL_USER_C_ASSERT)
	#define sol_c_assert(...) SOL_C_ASSERT(__VA_ARGS__)
#else
	#if SOL_IS_ON(SOL_DEBUG_BUILD)
		#include <exception>
		#include <iostream>
		#include <cstdlib>

			#define sol_c_assert(...)                                                                                               \
				do {                                                                                                               \
					if (!(__VA_ARGS__)) {                                                                                           \
						std::cerr << "Assertion `" #__VA_ARGS__ "` failed in " << __FILE__ << " line " << __LINE__ << std::endl; \
						std::terminate();                                                                                        \
					}                                                                                                             \
				} while (false)
	#else
		#define sol_c_assert(...)           \
			do {                           \
				if (false) {              \
					(void)(__VA_ARGS__); \
				}                         \
			} while (false)
	#endif
#endif

#if SOL_IS_ON(SOL_USER_M_ASSERT)
	#define sol_m_assert(message, ...) SOL_M_ASSERT(message, __VA_ARGS__)
#else
	#if SOL_IS_ON(SOL_DEBUG_BUILD)
		#include <exception>
		#include <iostream>
		#include <cstdlib>

		#define sol_m_assert(message, ...)                                                                                                         \
			do {                                                                                                                                  \
				if (!(__VA_ARGS__)) {                                                                                                              \
					std::cerr << "Assertion `" #__VA_ARGS__ "` failed in " << __FILE__ << " line " << __LINE__ << ": " << message << std::endl; \
					std::terminate();                                                                                                           \
				}                                                                                                                                \
			} while (false)
	#else
		#define sol_m_assert(message, ...)    \
			do {                             \
				if (false) {                \
					(void)(__VA_ARGS__);   \
					(void)sizeof(message); \
				}                           \
			} while (false)
	#endif
#endif

// clang-format on

#endif // SOL_ASSERT_HPP
