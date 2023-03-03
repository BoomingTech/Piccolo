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

# # # # sol2 tests - header compilation tests

file(GLOB_RECURSE sources
	LIST_DIRECTORIES FALSE
	CONFIGURE_DEPENDS
	source/*.cpp)
set(sources-single
	"source/main.cpp" "source/forward.cpp"
	"source/sol.cpp" "source/config.cpp")

set(target-variants
	"SOL_ALL_SAFETIES_ON"
	"SOL_USE_INTEROP"
	"SOL_STRINGS_ARE_NUMBERS")

function(CREATE_TEST test_target_name target_sol)
	set(test_sources ${ARGN})
	add_executable(${test_target_name} ${test_sources})
	sol2_add_test_properties(${test_target_name})
	target_link_libraries(${test_target_name}
		PRIVATE ${target_sol})
endfunction()

foreach(variant ${target-variants})
	set(target-name "sol2.tests.inclusion.${variant}")
	set(single-target-name "sol2.single.tests.inclusion.${variant}")
	
	CREATE_TEST(${target-name} sol2::sol2 ${sources})
	target_compile_definitions(${target-name} PRIVATE ${variant}=1)
	
	if (SOL2_TESTS_SINGLE)
		CREATE_TEST(${single-target-name} sol2::sol2::single ${sources-single})
		target_compile_definitions(${single-target-name} PRIVATE ${variant}=1)
	endif()
endforeach()
