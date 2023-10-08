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
    CONFIG=" Debug"
elif test \( \( -n "$1" \) -a \( "$1" = "release" \) \);then
    CONFIG=" Release"
else
    echo "The config \"$1\" is not supported!"
    echo ""
    echo "Configs:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
    exit 1
fi

cmake -S . -B build -G "Xcode"

cmake --build build --config "${CONFIG}"
