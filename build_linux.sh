#!/bin/bash

if test \( $# -ne 1 \);
then
    echo "Usage: build.sh arch config"
    echo ""
    echo "Configs:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
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

# replaced by your own cmake path
export PATH=./build_github_workflows/cmake-3.19.8-Linux-x86_64/bin${PATH:+:${PATH}}

export CC=clang
export CXX=clang++
cmake -S engine -B build "${CMAKE_ARG_BUILD_TYPE_CONFIG}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "${MY_DIR}/build" -- -j