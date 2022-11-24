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

# # # # sol2 - cmake - compiler diagnostic checking

include_guard(GLOBAL)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

#[[
Given a diagnostic name and flag, like
check_cxx_compiler_diagnostic(pig MSVC 1312)
or
check_cxx_compiler_diagnostic(pig GCC acab)
we check if the given flag works C++ compiler. If it does, we then generate
a --warn, --allow, --deny, and --forbid prefixed set of variables. Users are
then free to simply apply them to targets at will.
]]
function (check_compiler_diagnostic diagnostic)
	cmake_parse_arguments(diagnostic "" "GCC;MSVC;CLANG" "" ${ARGN})
	if (NOT diagnostic_GCC)
		set(diagnostic_GCC ${diagnostic})
	endif()
	if (NOT diagnostic_MSVC)
		set(diagnostic_MSVC ${diagnostic})
	endif()
	if (NOT diagnostic_CLANG)
		set(diagnostic_CLANG ${diagnostic_GCC})
	endif()
	string(MAKE_C_IDENTIFIER "${diagnostic}" suffix)
	string(TOUPPER "${suffix}" suffix)
	get_property(enabled-languages GLOBAL PROPERTY ENABLED_LANGUAGES)
	if (CXX IN_LIST enabled-languages)
		if (MSVC)
			check_cxx_compiler_flag(-wo${diagnostic_MSVC} CXX_DIAGNOSTIC_${suffix})
		elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)
			check_cxx_compiler_flag(-W${diagnostic_CLANG} CXX_DIAGNOSTIC_${suffix})
		else()
			check_cxx_compiler_flag(-W${diagnostic_GCC} CXX_DIAGNOSTIC_${suffix})
		endif()
	endif()
	if (C IN_LIST enabled-languages)
		if (MSVC)
			check_c_compiler_flag(-wo${diagnostic_MSVC} C_DIAGNOSTIC_${suffix})
		elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)
			check_c_compiler_flag(-W${diagnostic_CLANG} C_DIAGNOSTIC_${suffix})
		else()
			check_c_compiler_flag(-W${diagnostic_GCC} C_DIAGNOSTIC_${suffix})
		endif()
	endif()
	string(CONCAT when $<OR:
		$<AND:$<BOOL:${CXX_DIAGNOSTIC_${suffix}}>,$<COMPILE_LANGUAGE:CXX>>,
		$<AND:$<BOOL:${C_DIAGNOSTIC_${suffix}}>,$<COMPILE_LANGUAGE:C>>
	>)
	string(CONCAT diagnostic_flag
		$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:${diagnostic_MSVC}>
		$<$<COMPILE_LANG_AND_ID:C,MSVC>:${diagnostic_MSVC}>
		$<$<COMPILE_LANG_AND_ID:CXX,GNU>:${diagnostic_GCC}>
		$<$<COMPILE_LANG_AND_ID:C,GNU>:${diagnostic_GCC}>
		$<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:${diagnostic_CLANG}>
		$<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:${diagnostic_CLANG}>
	)
	set(forbid_prefix $<IF:$<BOOL:${MSVC}>,-we,-Werror=>)
	set(allow_prefix $<IF:$<BOOL:${MSVC}>,-wd,-Wno->)
	set(warn_prefix $<IF:$<BOOL:${MSVC}>,-w1,-W>)
	
	set(--forbid-${diagnostic} $<${when}:${forbid_prefix}${diagnostic_flag}> PARENT_SCOPE)
	set(--deny-${diagnostic} ${--forbid-${diagnostic}} PARENT_SCOPE)
	set(--allow-${diagnostic} $<${when}:${allow_prefix}${diagnostic_flag}> PARENT_SCOPE)
	set(--warn-${diagnostic} $<${when}:${warn_prefix}${diagnostic_flag}> PARENT_SCOPE)

endfunction()
