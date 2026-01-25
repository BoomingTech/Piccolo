#!/bin/bash

if test \( $# -ne 1 \);
then
    echo "Usage: ./build_macos.sh config"
    echo ""
    echo "config:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
    exit 1
fi


if test \( \( -n "$1" \) -a \( "$1" = "debug" \) \);then
    CONFIG="Debug"
elif test \( \( -n "$1" \) -a \( "$1" = "release" \) \);then
    CONFIG="Release"
else
    echo "The config \"$1\" is not supported!"
    echo ""
    echo "Configs:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
    exit 1
fi

# Use Unix Makefiles to avoid requiring full Xcode app
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${CONFIG}" -DCMAKE_POLICY_VERSION_MINIMUM=3.5

# Build (single-config generator, no --config needed)
cmake --build build
