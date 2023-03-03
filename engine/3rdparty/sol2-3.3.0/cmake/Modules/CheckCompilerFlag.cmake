# # # # sol2
# The MIT License (MIT)
# 
# Copyright (c) 2013-2022 Rapptz, ThePhD, and contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# # # # sol2 - cmake - compiler flag checking

include_guard(GLOBAL)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

#[[
Given a flag name and the actual flag, like
check_cxx_compiler_flag(strict-conformance MSVC /permissive- GCC -pedantic)
we check if the given flag works C++ compiler. If it is, then
--strict-conformance will be the provided flag. MSVC and GCC are the 2 different
"style" of flags to be tested for.
]]
function (check_compiler_flag flag_name)
	cmake_parse_arguments(PARSE_ARGV 1 flag "" "" "GCC;MSVC;CLANG")
	if (NOT flag_MSVC)
		set(flag_MSVC /${flag_name})
	endif()
	if (NOT flag_GCC)
		set(flag_GCC ${flag_name})
	endif()
	if (NOT flag_CLANG)
		set(flag_CLANG ${flag_GCC})
	endif()
	string(MAKE_C_IDENTIFIER "${flag_name}" suffix)
	string(TOUPPER "${suffix}" suffix)
	get_property(enabled-languages GLOBAL PROPERTY ENABLED_LANGUAGES)
	if (CXX IN_LIST enabled-languages)
		if (MSVC)
			check_cxx_compiler_flag(${flag_MSVC} CXX_CHECK_FLAG_${suffix})
		elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)
			check_cxx_compiler_flag(${flag_CLANG} CXX_CHECK_FLAG_${suffix})
		else()
			check_cxx_compiler_flag(${flag_GCC} CXX_CHECK_FLAG_${suffix})
		endif()
	endif()
	if (C IN_LIST enabled-languages)
		if (MSVC)
			check_c_compiler_flag(${flag_MSVC} C_CHECK_FLAG_${suffix})
		elseif (CMAKE_C_COMPILER_ID MATCHES Clang)
			check_c_compiler_flag(${flag_CLANG} C_CHECK_FLAG_${suffix})
		else()
			check_c_compiler_flag(${flag_GCC} C_CHECK_FLAG_${suffix})
		endif()
	endif()
	string(CONCAT when $<OR:
		$<AND:$<BOOL:${CXX_CHECK_FLAG_${suffix}}>,$<COMPILE_LANGUAGE:CXX>>,
		$<AND:$<BOOL:${C_CHECK_FLAG_${suffix}}>,$<COMPILE_LANGUAGE:C>>
	>)
	string(CONCAT compiler_flag
		$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:${flag_MSVC}>
		$<$<COMPILE_LANG_AND_ID:C,MSVC>:${flag_MSVC}>
		$<$<COMPILE_LANG_AND_ID:CXX,GNU>:${flag_GCC}>
		$<$<COMPILE_LANG_AND_ID:C,GNU>:${flag_GCC}>
		$<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:${flag_CLANG}>
		$<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:${flag_CLANG}>
	)

	set(--${flag_name} $<${when}:${compiler_flag}> PARENT_SCOPE)
endfunction()
