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

# # # # sol2 - cmake - version matching

include_guard(GLOBAL)

#[[
This is used to create a version variable. How these are used or set is up
to the user.
TODO: Support find_version(<VAR> SOURCE <FILE>) for parsing a version name from a header file
TODO: Support find_version(<VAR> LIBRARY <FILE>) for .dll/.lib/.so/.dylib parsing
]]
function (find_version output)
	cmake_parse_arguments(ARG "" "OPTION;REGEX;COMMAND;DOC" "" ${ARGN})
	unset(version-output)
	unset(version-check)
	if (NOT ARG_OPTION)
		set(ARG_OPTION "--version")
	endif()
	if (NOT ARG_REGEX)
		set(ARG_REGEX "[^0-9]*([0-9]+)[.]([0-9]+)?[.]?([0-9]+)?[.]?([0-9]+)?.*")
	endif()
	if (ARG_COMMAND AND NOT DEFINED ${output})
		execute_process(
			COMMAND ${ARG_COMMAND} ${ARG_OPTION}
			OUTPUT_VARIABLE version-output
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ENCODING UTF-8)
		if (version-output)
			string(REGEX MATCH "${ARG_REGEX}" version-check ${version-output})
		endif()
		if (version-check)
			string(JOIN "." ${output}
				${CMAKE_MATCH_1}
				${CMAKE_MATCH_2}
				${CMAKE_MATCH_3}
				${CMAKE_MATCH_4})
			set(${output} "${${output}}" CACHE STRING "${ARG_DOC}" FORCE)
		endif()
	endif()
endfunction()
