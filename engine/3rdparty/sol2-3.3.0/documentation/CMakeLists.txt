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

# # # # sol2, documentation generation
# # # Required minimum version statement
cmake_minimum_required(VERSION 3.16.0)
	
find_package(Doxygen REQUIRED)
find_package(Python3 REQUIRED)
find_package(Sphinx REQUIRED)

# Find all the public headers
file(GLOB_RECURSE sol2_public_headers
  LIST_DIRECTORIES NO
  CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/**.*)
# make a list copy for use with dependency tracking later down below
set(sol2_doxygen_public_headers ${sol2_public_headers})
# Remove every single public header
list(FILTER sol2_doxygen_public_headers EXCLUDE REGEX /detail/.*)
# separate text with spaces and surround them with quotes for Doxygen to understand them all
list(TRANSFORM sol2_doxygen_public_headers REPLACE "(.+)" [["\1"]])
list(JOIN sol2_doxygen_public_headers " " sol2_doxygen_public_headers)

set(SOL2_DOXYGEN_PROJECT_DESCRIPTION ${PROJECT_DESCRIPTION})
set(SOL2_DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
set(SOL2_DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(SOL2_DOXYGEN_INPUT ${sol2_doxygen_public_headers})
set(SOL2_DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(SOL2_DOXYGEN_XML_OUTPUT_DIR ${SOL2_DOXYGEN_OUTPUT_DIR}/xml)
set(SOL2_DOXYGEN_HTML_OUTPUT_DIR ${SOL2_DOXYGEN_OUTPUT_DIR}/html)
set(SOL2_DOXYGEN_INDEX_FILE ${SOL2_DOXYGEN_XML_OUTPUT_DIR}/index.xml)
set(SOL2_DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(SOL2_DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(SOL2_SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SOL2_SPHINX_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/sphinx)

# Get all Sphinx source files
file(GLOB_RECURSE sol2_sphinx_sources
  LIST_DIRECTORIES NO
  CONFIGURE_DEPENDS ${SOL2_SPHINX_SOURCE_DIR}/**.*)

#Replace variables inside @@ with the current values
configure_file(${SOL2_DOXYFILE_IN} ${SOL2_DOXYFILE_OUT} @ONLY)

# Ensure Doxygen/Sphinx won't crash because it doesn't create directories for us
file(MAKE_DIRECTORY ${SOL2_DOXYGEN_OUTPUT_DIR})
file(MAKE_DIRECTORY ${SOL2_DOXYGEN_XML_OUTPUT_DIR})
file(MAKE_DIRECTORY ${SOL2_DOXYGEN_HTML_OUTPUT_DIR})
file(MAKE_DIRECTORY ${SOL2_SPHINX_BUILD_DIR})

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
  ${SOL2_DOXYGEN_XML_OUTPUT_DIR}
  ${SOL2_DOXYGEN_HTML_OUTPUT_DIR}
  ${SOL2_DOXYGEN_OUTPUT_DIR}
  ${SOL2_SPHINX_BUILD_DIR})

add_custom_command(OUTPUT ${SOL2_DOXYGEN_INDEX_FILE}
	COMMAND ${DOXYGEN_EXECUTABLE} ${SOL2_DOXYFILE_OUT}
	MAIN_DEPENDENCY ${SOL2_DOXYFILE_IN} ${SOL2_DOXYFILE_OUT}
	DEPENDS ${sol2_public_headers}
	COMMENT "[sol2/documentation] Generating Doxygen XML..."
)
add_custom_target(sol2.documentation.doxygen ALL DEPENDS ${SOL2_DOXYGEN_INDEX_FILE})

if (NOT SOL2_DOCUMENTATION_NO_SPHINX)
	add_custom_target(sol2.documentation.sphinx ALL
    COMMAND Sphinx::Build
      -b html
      -Dbreathe_projects.sol2=${SOL2_DOXYGEN_XML_OUTPUT_DIR}
      ${SOL2_SPHINX_SOURCE_DIR}
      ${SOL2_SPHINX_BUILD_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${SOL2_DOXYGEN_INDEX_FILE} ${sol2_sphinx_sources}
    COMMENT "[sol2/documentation] Generating Sphinx documentation..."
    COMMAND_EXPAND_LISTS
    VERBATIM)

	add_dependencies(sol2.documentation.sphinx sol2.documentation.doxygen)
endif()

# For the install target
install(DIRECTORY ${SOL2_SPHINX_BUILD_DIR}
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
