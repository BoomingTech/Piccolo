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

# # # sol2 Examples

function(sol2_add_example_properties target-name)
	target_link_libraries(${target-name}
		PUBLIC Threads::Threads ${LUA_LIBRARIES} ${CMAKE_DL_LIBS})
	target_compile_definitions(${target-name}
		PUBLIC SOL_PRINT_ERRORS=1)
	target_compile_options(${target-name}
		PRIVATE
		${--template-debugging-mode}
		${--big-obj}
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
		${--allow-unreachable-code}
		${--allow-padding-from-alignment}
	)

	target_compile_definitions(${target-name}
		PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)

	if (SOL2_CI)
		target_compile_definitions(${target-name}
			PRIVATE SOL2_CI)
	endif()
	
	if (SOL2_TESTS_EXAMPLES)
		add_test(NAME ${target-name} COMMAND ${target-name})
	endif()
	if(SOL2_ENABLE_INSTALL)
		install(TARGETS ${target-name} RUNTIME DESTINATION bin)
	endif()
endfunction()

if (SOL2_DYNAMIC_LOADING_EXAMPLES OR SOL2_DYNAMIC_LOADING_EXAMPLES_SINGLE)
	# # require_from_dll example
	# just add the subdirectory
	add_subdirectory(require_dll_example)
endif()

if (SOL2_INTEROP_EXAMPLES OR SOL2_INTEROP_EXAMPLES_SINGLE)
	# # interop examples
	add_subdirectory(interop/kaguya)
	add_subdirectory(interop/tolua)
	add_subdirectory(interop/LuaBridge)
	add_subdirectory(interop/luwra)
endif()

# # In-depth customization example
add_subdirectory(customization)

# # single-source compilable examples

file(GLOB sol2.examples.sources source/*.cpp source/tutorials/*.cpp source/tutorials/quick_n_dirty/*.cpp source/docs/*.cpp)

function (MAKE_EXAMPLE example_source_file example_prefix target_sol)
	get_filename_component(example_name ${example_source_file} NAME_WE)
	file(RELATIVE_PATH example_source_file_relative ${CMAKE_SOURCE_DIR} ${example_source_file})
	get_filename_component(example_output_relative_dir ${example_source_file_relative} DIRECTORY)
	file(TO_CMAKE_PATH "${example_output_relative_dir}" example_output_relative_dir_name)
	STRING(REGEX REPLACE "/" "." example_output_relative_dir_name "${example_output_relative_dir}")
	if (example_output_relative_dir_name STREQUAL "")
		set(example_output_name ${example_name})
		set(example_name "${example_prefix}.${example_name}")
	else()
		set(example_output_name ${example_output_relative_dir_name}.${example_name})
		set(example_name "${example_prefix}.${example_output_relative_dir_name}.${example_name}")
	endif()

	add_executable(${example_name} ${example_source_file})
	sol2_add_example_properties(${example_name})
	target_link_libraries(${example_name}
		PRIVATE ${target_sol})
endfunction(MAKE_EXAMPLE)

if (SOL2_EXAMPLES)
	foreach(example_source_file ${sol2.examples.sources})
		MAKE_EXAMPLE(${example_source_file} "sol2" sol2::sol2)
	endforeach()
endif()

if (SOL2_EXAMPLES_SINGLE)
	foreach(example_source_file ${sol2.examples.sources})
		MAKE_EXAMPLE(${example_source_file} "sol2.single" sol2::sol2::single)
	endforeach()
endif()
