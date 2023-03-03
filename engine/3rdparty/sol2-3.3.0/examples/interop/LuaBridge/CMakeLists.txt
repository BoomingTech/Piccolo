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

# # # sol2 interop Examples - luabridge

find_package(LuaBridgeBuild)

function (make_luabridge_interop_example target_library example_suffix)
	set(example_name luabridge_interop_example)
	set(example_name "${example_name}${example_suffix}")
	
	add_executable(${example_name} source/LuaBridge.cpp)
	target_link_libraries(${example_name} 
		PRIVATE ${LUA_LIBRARIES} ${LUABRIDGE_LIBRARIES} 
		${target_library} ${CMAKE_DL_LIBS})

	if (MSVC)
		target_compile_options(${example_name}
			PRIVATE /std:c++latest /EHsc "$<$<CONFIG:Debug>:/MDd>"
			"$<$<CONFIG:Release>:/MD>"
			"$<$<CONFIG:RelWithDebInfo>:/MD>"
			"$<$<CONFIG:MinSizeRel>:/MD>")
		target_compile_definitions(${example_name}
			PRIVATE /W1
			UNICODE _UNICODE 
			_CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
	else()
		target_compile_options(${example_name}
			PRIVATE -std=c++1z -w
			-Wno-unknown-warning -Wno-unknown-warning-option)
	endif()

	if (SOL2_TESTS_INTEROP_EXAMPLES)
		add_test(NAME ${example_name} COMMAND ${example_name})
	endif()
endfunction()

if (SOL2_INTEROP_EXAMPLES)
	make_luabridge_interop_example(sol2::sol2 "")
endif()
if (SOL2_INTEROP_EXAMPLES_SINGLE)
	make_luabridge_interop_example(sol2::sol2::single ".single")
endif()
