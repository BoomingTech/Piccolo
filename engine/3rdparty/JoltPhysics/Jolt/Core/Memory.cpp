// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/Memory.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <cstdlib>
JPH_SUPPRESS_WARNINGS_STD_END
#include <stdlib.h>

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#endif

JPH_NAMESPACE_BEGIN

void *AlignedAlloc(size_t inSize, size_t inAlignment)
{
#if defined(JPH_PLATFORM_WINDOWS)
	// Microsoft doesn't implement C++17 std::aligned_alloc
	return _aligned_malloc(inSize, inAlignment);
#elif defined(JPH_PLATFORM_ANDROID)
	return memalign(inAlignment, AlignUp(inSize, inAlignment));
#elif defined(__APPLE__) && (defined(MAC_OS_X_VERSION_10_16)) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_16
    // For C++14, usr/include/malloc/_malloc.h declares aligned_alloc()) only
    // with the MacOSX11.0 SDK in Xcode 12 (which is what adds
    // MAC_OS_X_VERSION_10_16), even though the function is marked
    // availabe for 10.15. That's why the preprocessor checks for 10.16 but
    // the __builtin_available checks for 10.15.
    // People who use C++17 could call aligned_alloc with the 10.15 SDK already.
    return aligned_alloc(inAlignment, AlignUp(inSize, inAlignment));
#else
	return std::aligned_alloc(inAlignment, AlignUp(inSize, inAlignment));
#endif
}

void AlignedFree(void *inBlock)
{
#if defined(JPH_PLATFORM_WINDOWS)
	_aligned_free(inBlock);
#elif defined(JPH_PLATFORM_ANDROID)
	free(inBlock);
#else
	std::free(inBlock);
#endif
}

JPH_NAMESPACE_END
