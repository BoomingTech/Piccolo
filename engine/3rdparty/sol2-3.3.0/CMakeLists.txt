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

# # # # sol2
# # # Required minimum version statement
cmake_minimum_required(VERSION 3.16.0)
# # # Project Include - file that is included after project declaration is finished
set(CMAKE_PROJECT_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Includes/Project.cmake")

# # # project declaration
project(sol2 VERSION 4.0.0 LANGUAGES CXX C)

if (sol2-is-top-level-project)
	message(STATUS "sol2 is the top-level directory...")
endif()

# Include standard modules
include(CMakePackageConfigHelpers)
include(CheckCXXCompilerFlag)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(FetchContent)

# # # Configuration
# # Cached defines, strings, paths and options
set(SOL2_LUA_VERSION "5.4.4" CACHE STRING "The version of Lua needed. Can be 5.1, 5.2, 5.3, 5.4, LuaJIT, or a more specific 3-part version number for a specifc Lua (e.g., 5.4.4 or luajit-2.0.5)")
set(SOL2_BUILD_LUA TRUE CACHE BOOL "Always build Lua, do not search for it in the system")
set(SOL2_PLATFORM "x64" CACHE STRING "Target platform to compile for when building binaries (x86, x64)")
option(SOL2_CI "Whether or not we are in continguous integration mode" OFF)
option(SOL2_SYSTEM_INCLUDE "Whether or not sol2 should be considered a system include. This helps suppress errors for when the sol2 author is a big derp and doesn't fix every single warning, ever." ON)
option(SOL2_TESTS "Enable build of tests" OFF)
option(SOL2_EXAMPLES "Enable build of examples" OFF)
option(SOL2_INTEROP_EXAMPLES "Enable build of interop examples" OFF)
option(SOL2_DYNAMIC_LOADING_EXAMPLES "Enable build of interop examples" OFF)
option(SOL2_SINGLE "Enable generation and build of single header files" OFF)
option(SOL2_DOCS "Enable build of documentation" OFF)
option(SOL2_ENABLE_INSTALL "Enable installation of Sol2" ON)
# Single tests and examples tests will only be turned on if both SINGLE and TESTS are defined
CMAKE_DEPENDENT_OPTION(SOL2_TESTS_SINGLE "Enable build of tests using the premade single headers" ON
	"SOL2_SINGLE;SOL2_TESTS" OFF)
CMAKE_DEPENDENT_OPTION(SOL2_EXAMPLES_SINGLE "Enable build of examples using the generated single headers" OFF
	"SOL2_SINGLE;SOL2_EXAMPLES" OFF)
CMAKE_DEPENDENT_OPTION(SOL2_INTEROP_EXAMPLES_SINGLE "Enable build of interop examples using the generated single headers" OFF
	"SOL2_SINGLE;SOL2_INTEROP_EXAMPLES" OFF)
CMAKE_DEPENDENT_OPTION(SOL2_DYNAMIC_LOADING_EXAMPLES_SINGLE "Enable build of dynamic loading examples using the generated single headers" OFF
	"SOL2_SINGLE;SOL2_DYNAMIC_LOADING_EXAMPLES" OFF)
CMAKE_DEPENDENT_OPTION(SOL2_TESTS_EXAMPLES "Enable build of examples as tests" ON
	"SOL2_EXAMPLES" OFF)
CMAKE_DEPENDENT_OPTION(SOL2_TESTS_INTEROP_EXAMPLES "Enable build of interop examples as tests" ON
	"SOL2_INTEROP_EXAMPLES" OFF)
CMAKE_DEPENDENT_OPTION(SOL2_TESTS_DYNAMIC_LOADING_EXAMPLES "Enable build of dynamic loading examples as tests" ON
	"SOL2_DYNAMIC_LOADING_EXAMPLES" OFF)
CMAKE_DEPENDENT_OPTION(BUILD_LUA_AS_DLL "Build Lua as a DLL" ON
	"SOL2_BUILD_LUA" OFF)


if (SOL2_SYSTEM_INCLUDE)
	set(sol2-system-include SYSTEM)
endif()

# # # sol2 Source Groups
# # Sources everyone is going to need
# Header files
file(GLOB_RECURSE sol2-headers
	LIST_DIRECTORIES FALSE
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/include/sol*.hpp
)

file(GLOB_RECURSE sol2-sources
	LIST_DIRECTORIES FALSE
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/**
)

# # # sol2 Library
# # Add a target for sol2's library to be included by external users
add_library(sol2 INTERFACE)
add_library(sol2::sol2 ALIAS sol2)
target_include_directories(sol2 ${sol2-system-include}
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

# # Version configurations
configure_package_config_file(
	cmake/sol2-config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/sol2-config.cmake"
	INSTALL_DESTINATION lib/cmake/sol2
	NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/sol2-config-version.cmake"
	COMPATIBILITY AnyNewerVersion)

export(TARGETS sol2
	FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/sol2-targets.cmake")

if(SOL2_ENABLE_INSTALL)
	install(TARGETS sol2
		EXPORT sol2)

	install(EXPORT sol2
		FILE sol2-targets.cmake
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/sol2")

	install(DIRECTORY include/sol
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

	install(FILES
		"${CMAKE_CURRENT_BINARY_DIR}/cmake/sol2-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/cmake/sol2-config-version.cmake"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/sol2")
endif()

# # # sol2 Library - Single header target
if (SOL2_SINGLE)
	message(STATUS "sol2 adding single...")
	add_subdirectory(single)
endif()

# # # documentation
# Generates the docs
if (SOL2_DOCS)
	message(STATUS "sol2 adding docs...")
	add_subdirectory(documentation)
endif()

if(SOL2_ENABLE_INSTALL)
	# pkg-config support, except on Windows
	if(NOT WIN32 OR NOT CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
		set(PKGCONFIG_INSTALL_DIR
			"${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig"
			CACHE PATH "Path where sol2.pc is installed")

		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sol2.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/sol2.pc" @ONLY)
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sol2.pc" DESTINATION "${PKGCONFIG_INSTALL_DIR}")
	endif()
endif()

if (SOL2_CI)
	message(STATUS "sol2 Contiguous Integration is on")
endif()

if (SOL2_EXAMPLES OR SOL2_TESTS_EXAMPLES OR SOL2_EXAMPLES_SINGLE OR SOL2_INTEROP_EXAMPLES OR SOL2_TESTS_INTEROP_EXAMPLES OR SOL2_INTEROP_EXAMPLES_SINGLE OR SOL2_DYNAMIC_LOADING_EXAMPLES OR SOL2_TESTS_DYNAMIC_LOADING_EXAMPLES OR SOL2_DYNAMIC_LOADING_EXAMPLES_SINGLE)
	set(SOL2_DO_EXAMPLES TRUE)
else()
	set(SOL2_DO_EXAMPLES FALSE)
endif()

if (SOL2_TESTS OR SOL2_TESTS_SINGLE)
	set(SOL2_DO_TESTS TRUE)
else()
	set(SOL2_DO_TESTS FALSE)
endif()

# # # Tests, Examples and other CI suites that come with sol2
if (sol2-is-top-level-project)
	# # # General project output locations
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/x86/lib")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/x86/bin")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/x86/bin")
	else()
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/x64/lib")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/x64/bin")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/x64/bin")
	endif()

	if (NOT CMAKE_CXX_STANDARD GREATER_EQUAL 17)
		set(CMAKE_CXX_STANDARD 17)
	endif()

	if (NOT CMAKE_C_STANDARD GREATER_EQUAL 11)
		set(CMAKE_C_STANDARD 11)
	endif()

	# Build Flag Settings
	# Basic/Normal flags
	check_compiler_flag(disable-permissive MSVC /permissive- GCC -pedantic)
	check_compiler_flag(utf8-literal-encoding MSVC /execution-charset:utf-8 GCC -fexec-charset=utf-8)
	check_compiler_flag(utf8-source-encoding MSVC /source-charset:utf-8 GCC -finput-charset=utf-8)
	check_compiler_flag(extra-constexpr-depth MSVC /constexpr:depth2147483647 GCC -fconstexpr-depth=2147483647 CLANG -fconstexpr-depth=2147483647)
	check_compiler_flag(extra-constexpr-steps MSVC /constexpr:steps2147483647 GCC -fconstexpr-ops-limit=2147483647 CLANG -fconstexpr-steps=2147483647)
	check_compiler_flag(template-debugging-mode GCC -ftemplate-backtrace-limit=0)
	check_compiler_flag(big-obj MSVC /bigobj)
	# Overall warning flags
	check_compiler_flag(pedantic GCC -pedantic)
	check_compiler_flag(warn-pedantic GCC -Wpedantic)
	check_compiler_flag(warn-all MSVC /W4 GCC -Wall)
	check_compiler_flag(warn-extra GCC -Wextra)
	check_compiler_flag(warn-errors MSVC /WX GCC -Werror)
	# Individual warnings/errors
	check_compiler_diagnostic(unknown-warning)
	check_compiler_diagnostic(unknown-warning-option)
	check_compiler_diagnostic(microsoft-cast)
	check_compiler_diagnostic(noexcept-type)
	check_compiler_diagnostic(unreachable-code MSVC 4702)
	check_compiler_diagnostic(padding-from-alignment MSVC 4324)

	# # # Libraries
	# Here, we pull in all the necessary libraries for building examples and tests
	# Find threading library
	find_package(Threads REQUIRED)

	string(TOLOWER ${SOL2_LUA_VERSION} NORMALIZED_LUA_VERSION)
	# Find way to get Lua: build if requested, or attempt to build if no matching version is found
	if (SOL2_BUILD_LUA)
		find_package(LuaBuild REQUIRED COMPONENTS ${SOL2_LUA_VERSION})
	elseif (NOT SOL2_LUA_VERSION)
		find_package(LuaBuild REQUIRED)
	else ()
		if (NORMALIZED_LUA_VERSION MATCHES "5.1")
			set(CREATE_LUALIB_TARGET TRUE)
			find_package(Lua 5.1 EXACT REQUIRED)
		elseif(NORMALIZED_LUA_VERSION MATCHES "5.2")
			set(CREATE_LUALIB_TARGET TRUE)
			find_package(Lua 5.2 EXACT REQUIRED)
		elseif(NORMALIZED_LUA_VERSION MATCHES "5.3")
			set(CREATE_LUALIB_TARGET TRUE)
			find_package(Lua 5.3 EXACT REQUIRED)
		elseif(NORMALIZED_LUA_VERSION MATCHES "5.4")
			set(CREATE_LUALIB_TARGET TRUE)
			find_package(Lua 5.4 EXACT REQUIRED)
		elseif(NORMALIZED_LUA_VERSION MATCHES "luajit")
			set(CREATE_LUALIB_TARGET TRUE)
			find_package(LuaJIT REQUIRED)
		else()
			find_package(LuaBuild ${SOL2_LUA_VERSION} REQUIRED)
		endif()
	endif()

	if (CREATE_LUALIB_TARGET AND LUA_FOUND)
		set(lualib lua_imported_lib_${SOL2_LUA_VERSION})
		foreach(lua_search_lib ${LUA_LIBRARIES})
			get_filename_component(lsl_fname ${lua_search_lib} NAME)
			if (lsl_fname MATCHES "lua" OR lsl_fname MATCHES "Lua" OR lsl_fname MATCHES "LUA")
				if (lsl_fname MATCHES "\.so|\.dylib|\.dll")
					set(lualibtype SHARED)
					set(lualiblocation ${lua_search_lib})
				else()
					set(lualibtype STATIC)
					set(lualiblocation ${lua_search_lib})
				endif()
			else()
				set(LUA_SEARCH_DEPENDENCY_LIBS ${LUA_SEARCH_DEPENDENCY_LIBS} "${lua_search_lib}")
			endif()
		endforeach()
		add_library(${lualib} ${lualibtype} IMPORTED)
		set_target_properties(${lualib}
			PROPERTIES 
			INTERFACE_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR}
			INTERFACE_LINK_LIBRARIES "${LUA_SEARCH_DEPENDENCY_LIBS} ${CMAKE_DL_LIBS}"
			IMPORTED_LINK_INTERFACE_LANGUAGES C
			IMPORTED_LOCATION ${lualiblocation})
		set(LUA_LIBRARIES ${lualib})
	endif()

	if (NOT LUA_FOUND AND NOT LUABUILD_FOUND)
		message(FATAL_ERROR "sol2 Lua \"${SOL2_LUA_VERSION}\" not found and could not be targeted for building")
	endif()
	
	# # Enable test harness for regular, example or single tests
	if (SOL2_DO_TESTS OR (SOL2_TESTS_EXAMPLES AND SOL2_DO_EXAMPLES))
		# enable ctest
		message(STATUS "sol2 testing enabled...")
		enable_testing()
	endif()
	
	# # # Examples
	# # Enable examples to be built against the library
	if (SOL2_DO_EXAMPLES)
		# NOTE: will also add to tests if TESTS is defined
		message(STATUS "sol2 adding examples...")
		add_subdirectory(examples)
	endif()
	
	# # # Tests
	# # Add tests here
	if (SOL2_DO_TESTS)
		# add subdir to get going
		message(STATUS "sol2 adding tests...")
		add_subdirectory(tests)
	endif()

	# # # Scratch Space
	# # Scratch space for diagnosing bugs and other shenanigans
	if (SOL2_SCRATCH)
		message(STATUS "sol2 adding scratch space...")
		add_subdirectory(scratch)
	endif()
endif()
