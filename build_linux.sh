#!/bin/bash

if test \( $# -gt 2 \);
then
    echo "Usage: ./build_linux.sh config [build-tool]"
    echo ""
    echo "config:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
    echo "build-tool:"
    echo "  make    -   build with Unix Make"
    echo "  ninja   -   build with Ninja"
    echo ""
    exit 1
fi

if test \( $# -eq 1 \) ;then
    if command -v ninja &> /dev/null; then
        CMAKE_ARG_BUILD_TOOL_TYPE_CONFIG="-G Ninja"
    else
        CMAKE_ARG_BUILD_TOOL_TYPE_CONFIG="-G Unix Makefiles"
    fi
elif test \( \( -n "$2" \) -a \( "$2" = "make" \) \);then
    CMAKE_ARG_BUILD_TOOL_TYPE_CONFIG="-G Unix Makefiles"
elif test \( \( -n "$2" \) -a \( "$2" = "ninja" \) \);then
    CMAKE_ARG_BUILD_TOOL_TYPE_CONFIG="-G Ninja"
else
    echo "The build-tool \"$2\" is not supported!"
    exit 1
fi

if test \( \( -n "$1" \) -a \( "$1" = "debug" \) \);then 
    CMAKE_ARG_BUILD_TYPE_CONFIG="-DCMAKE_BUILD_TYPE=Debug"
elif test \( \( -n "$1" \) -a \( "$1" = "release" \) \);then
    CMAKE_ARG_BUILD_TYPE_CONFIG="-DCMAKE_BUILD_TYPE=Release"
else
    echo "The config \"$1\" is not supported!"
    echo ""
    echo "Configs:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
    exit 1
fi

MY_DIR="$(cd "$(dirname "$0")" 1>/dev/null 2>/dev/null && pwd)"  
cd "${MY_DIR}"

mkdir -p "engine/shader/generated/spv"

export CC=clang
export CXX=clang++
cmake -S . -B build "${CMAKE_ARG_BUILD_TYPE_CONFIG}" "${CMAKE_ARG_BUILD_TOOL_TYPE_CONFIG}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${MY_DIR}/build" -- all -j$(nproc)
