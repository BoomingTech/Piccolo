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

# # # sol2 Customization Examples

function (MAKE_CUSTOMIZATION_EXAMPLE example_suffix target_sol)
	set(customization_example_name customization_to_table${example_suffix})
	
	add_executable(${customization_example_name} source/main.cpp source/lua_interop.cpp source/lua_zm_interop.cpp)
	set_target_properties(${customization_example_name}
		PROPERTIES
		OUTPUT_NAME "${customization_example_name}"
		EXPORT_NAME sol2::${customization_example_name})

	if (MSVC)
		target_compile_options(${customization_example_name}
			PRIVATE /std:c++latest /EHsc "$<$<CONFIG:Debug>:/MDd>"
			"$<$<CONFIG:Release>:/MD>"
			"$<$<CONFIG:RelWithDebInfo>:/MD>"
			"$<$<CONFIG:MinSizeRel>:/MD>")
		target_compile_definitions(${customization_example_name}
			PRIVATE UNICODE _UNICODE 
			_CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
	else()
		target_compile_options(${customization_example_name}
			PRIVATE -std=c++1z 
			-ftemplate-backtrace-limit=0
			-Wno-unknown-warning -Wno-unknown-warning-option
			-Wall -Wpedantic -Werror -pedantic -pedantic-errors
			-Wno-noexcept-type)
	endif()

	target_link_libraries(${customization_example_name}
		PRIVATE Threads::Threads ${target_sol} ${LUA_LIBRARIES})
	target_include_directories(${customization_example_name}
		PRIVATE include)
endfunction()

if (SOL2_EXAMPLES)
	MAKE_CUSTOMIZATION_EXAMPLE("" sol2::sol2)
endif()

if (SOL2_EXAMPLES_SINGLE)
	MAKE_CUSTOMIZATION_EXAMPLE(".single" sol2::sol2::single)
endif()
