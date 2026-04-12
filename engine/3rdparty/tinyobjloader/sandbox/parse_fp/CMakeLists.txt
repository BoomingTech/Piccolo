cmake_minimum_required(VERSION 3.0)
project(parse_fp_test CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(test_parse_fp test_parse_fp.cc)

# Optional: enable optimizations for benchmark mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(MSVC)
    target_compile_options(test_parse_fp PRIVATE /O2)
  else()
    target_compile_options(test_parse_fp PRIVATE -O2)
  endif()
endif()
