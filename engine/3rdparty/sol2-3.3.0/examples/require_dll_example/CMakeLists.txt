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

# # # sol2 Examples - require_from_dll

# # Reusable function to call for single target
# # Also hides variables from directory/global scope
function(make_require_from_dll_example target_lib example_lib_name_suffix)
	# define sources
	set(my_object_sources source/my_object.cpp)
	set(require_from_dll_sources source/require_from_dll.cpp)

	# define names
	set(example_lib_name my_object)
	set(example_name require_from_dll)
	set(example_lib_name "${example_lib_name}${example_lib_name_suffix}")
	set(example_name "${example_name}${example_lib_name_suffix}")
	
	# add library target my_object for the require_from_dll program
	add_library(${example_lib_name} SHARED ${my_object_sources})
	set_target_properties(${example_lib_name} PROPERTIES
		PREFIX "")

	target_include_directories(${example_lib_name} 
		PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
	target_compile_definitions(${example_lib_name} 
		PUBLIC MY_OBJECT_DLL 
		PRIVATE MY_OBJECT_BUILD)
	target_link_libraries(${example_lib_name}
		PUBLIC ${target_lib} ${LUA_LIBRARIES} ${CMAKE_DL_LIBS})
	target_include_directories(${example_lib_name}
		PUBLIC "${LUA_INCLUDE_DIRS}")
	
	if (MSVC)
		target_compile_options(${example_lib_name}
			PRIVATE /std:c++latest /EHsc "$<$<CONFIG:Debug>:/MDd>"
			"$<$<CONFIG:Release>:/MD>"
			"$<$<CONFIG:RelWithDebInfo>:/MD>"
			"$<$<CONFIG:MinSizeRel>:/MD>")
		target_compile_definitions(${example_lib_name}
			PRIVATE UNICODE _UNICODE 
			_CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
	else()
		target_compile_options(${example_lib_name}
			PRIVATE -std=c++1z 
			-Wno-unknown-warning -Wno-unknown-warning-option
			-Wall -Wextra -Wpedantic -pedantic -pedantic-errors
			-Wno-noexcept-type)

		if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			# For another day, when C++ is not so crap
			# and we have time to audit the entire lib
			# for all uses of `detail::swallow`...
			#target_compile_options(${test_target_name}
			#	PRIVATE -Wcomma)		
		endif()


		if (IS_X86)
			target_compile_options(${example_lib_name} BEFORE PRIVATE -m32)
		endif()
	endif()

	# add executable target that represents require_from_dll program
	add_executable(${example_name} ${require_from_dll_sources})
	target_link_libraries(${example_name}
		PRIVATE my_object ${LUA_LIBRARIES} ${target_lib} ${CMAKE_DL_LIBS})
	target_include_directories(${example_name}
		PRIVATE ${LUA_INCLUDE_DIRS})

	if (MSVC)
		target_compile_options(${example_name}
			PRIVATE /std:c++latest /EHsc "$<$<CONFIG:Debug>:/MDd>"
			"$<$<CONFIG:Release>:/MD>"
			"$<$<CONFIG:RelWithDebInfo>:/MD>"
			"$<$<CONFIG:MinSizeRel>:/MD>")
		target_compile_definitions(${example_name}
			PRIVATE UNICODE _UNICODE 
			_CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
	else()
		target_compile_options(${example_name}
			PRIVATE -std=c++1z
			-Wno-unknown-warning -Wno-unknown-warning-option
			-Wall -Wextra -Wpedantic -pedantic -pedantic-errors
			-Wno-noexcept-type)

		if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			# For another day, when C++ is not so crap
			# and we have time to audit the entire lib
			# for all uses of `detail::swallow`...
			#target_compile_options(${test_target_name}
			#	PRIVATE -Wcomma)		
		endif()
	endif()
	
	if (SOL2_TESTS_DYNAMIC_LOADING_EXAMPLES)
		get_target_property(example_working_dir ${example_name} RUNTIME_OUTPUT_DIRECTORY)
		add_test(NAME ${example_name} COMMAND ${example_name} WORKING_DIRECTORY "${example_working_dir}")
	endif()
endfunction()

list(GET LUA_LIBRARIES 0 lua_lib_target)
get_target_property(lua_lib_type ${lua_lib_target} TYPE)
if (lua_lib_type MATCHES "STATIC")
	# avoid multiply defined references due to linking in the same static library
	# twice over, and get "multiple definition" errors during linking
	return()
endif()

if (SOL2_DYNAMIC_LOADING_EXAMPLES)
	make_require_from_dll_example(sol2::sol2 "")
endif()
if (SOL2_DYNAMIC_LOADING_EXAMPLES_SINGLE)
	make_require_from_dll_example(sol2::sol2::single ".single")
endif()
