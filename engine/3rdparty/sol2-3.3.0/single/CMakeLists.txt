# # # # sol3
# The MIT License (MIT)
# 
# Copyright (c) 2013-2019 Rapptz, ThePhD, and contributors
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

# # # # sol3, single
# # # Required minimum version statement
cmake_minimum_required(VERSION 3.15.0)

find_package(PythonInterp 3 REQUIRED)

# to generate, need all of the existing header files
file(GLOB sol2_generated_header_sources ${CMAKE_CURRENT_SOURCE_DIR}/../include/**/*.*)

# generate the single header if we can find the python3 interpreter
add_custom_command(
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/include/sol"
	COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/single.py" --input "${CMAKE_CURRENT_SOURCE_DIR}/../include" --output "${CMAKE_CURRENT_BINARY_DIR}/include/sol/sol.hpp"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/single.py" "${sol2_generated_header_sources}"
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/sol/sol.hpp" "${CMAKE_CURRENT_BINARY_DIR}/include/sol/forward.hpp")
add_custom_target(sol2_single_header_generator ALL
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/single.py"
	"${CMAKE_CURRENT_BINARY_DIR}/include/sol/sol.hpp" "${CMAKE_CURRENT_BINARY_DIR}/include/sol/forward.hpp" 
	"${sol2_generated_header_sources}")

# # # sol3 generated single header library
add_library(sol2_single INTERFACE)
add_library(sol2::sol2::single ALIAS sol2_single)
set_target_properties(sol2_single
	PROPERTIES
	EXPORT_NAME sol2::sol2::single
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/include")
add_dependencies(sol2_single sol2_single_header_generator)

if(SOL2_ENABLE_INSTALL)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/sol/sol.hpp" "${CMAKE_CURRENT_BINARY_DIR}/include/sol/config.hpp" "${CMAKE_CURRENT_BINARY_DIR}/include/sol/forward.hpp"
		DESTINATION include/sol/single/sol)
endif()
