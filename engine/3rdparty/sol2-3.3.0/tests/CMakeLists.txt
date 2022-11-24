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

# # # # sol2 tests

# # Dependencies
FetchContent_Declare(
	catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG devel
)
FetchContent_MakeAvailable(catch2)

function(sol2_add_test_properties target-name)
	target_link_libraries(${target-name}
		PUBLIC Threads::Threads ${LUA_LIBRARIES} Catch2::Catch2 ${CMAKE_DL_LIBS})
	target_compile_definitions(${target-name}
		PUBLIC SOL_PRINT_ERRORS=1)
	target_compile_options(${target-name}
		PRIVATE
		${--template-debugging-mode}
		${--big-obj}
		${--no-unknown-warning}
		${--disable-permissive}
		${--pedantic}
		${--warn-all}
		${--warn-pedantic}
		${--warn-extra}
		${--warn-errors}
		${--utf8-literal-encoding}
		${--utf8-source-encoding}

		${--allow-unknown-warning}
		${--allow-unknown-warning-option}
		${--allow-noexcept-type}
		${--allow-microsoft-cast}
	)

	target_compile_definitions(${target-name}
		PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)

	if (SOL2_CI)
		target_compile_definitions(${target-name}
			PRIVATE SOL2_CI)
	endif()
	
	add_test(NAME ${target-name} COMMAND ${target-name})
	if(SOL2_ENABLE_INSTALL)
		install(TARGETS ${target-name} RUNTIME DESTINATION bin)
	endif()
endfunction()

function(sol2_create_basic_test test_target_name target_sol)
	set(test_target_name ${test_target_name})
	set(sources ${ARGN})
	add_executable(${test_target_name} ${sources})
	sol2_add_test_properties(${test_target_name})
	target_link_libraries(${test_target_name}
		PRIVATE ${target_sol})
	target_compile_definitions(${test_target_name}
		PRIVATE SOL_ALL_SAFETIES_ON=1)
	target_compile_definitions(${test_target_name}
		PRIVATE SOL_TESTS_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})
endfunction()

add_subdirectory(inclusion)
add_subdirectory(enum)
add_subdirectory(environment)
add_subdirectory(exceptions)
add_subdirectory(lua_lifetime)
add_subdirectory(numerics)
add_subdirectory(config_tests)
add_subdirectory(regression_tests)
add_subdirectory(run_time)
