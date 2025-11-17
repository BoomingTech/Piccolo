cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Piccolo VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

# Support for macOS Universal Binary (x86_64 + arm64)
if(APPLE)
    # Option to build universal binary
    option(BUILD_UNIVERSAL_BINARY "Build universal binary for macOS (x86_64 + arm64)" OFF)
    
    if(BUILD_UNIVERSAL_BINARY)
        set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS" FORCE)
        message(STATUS "Building Universal Binary for macOS (x86_64 + arm64)")
    else()
        # Auto-detect architecture if not building universal binary
        if(NOT CMAKE_OSX_ARCHITECTURES)
            execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE NATIVE_ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(NATIVE_ARCH STREQUAL "arm64")
                set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)
                message(STATUS "Building for Apple Silicon (arm64)")
            else()
                set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for macOS" FORCE)
                message(STATUS "Building for Intel (x86_64)")
            endif()
        endif()
    endif()
    
    # Set minimum macOS version
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version" FORCE)
    endif()
endif()

include(CMakeDependentOption)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(PICCOLO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${PICCOLO_ROOT_DIR}/bin")
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")


add_subdirectory(engine)