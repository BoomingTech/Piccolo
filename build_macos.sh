#!/bin/bash

# Default to native architecture
BUILD_UNIVERSAL=0

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        debug|release)
            if [ -z "$CONFIG" ]; then
                if [ "$1" = "debug" ]; then
                    CONFIG="Debug"
                else
                    CONFIG="Release"
                fi
            fi
            shift
            ;;
        --universal)
            BUILD_UNIVERSAL=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo ""
            echo "Usage: ./build_macos.sh [config] [options]"
            echo ""
            echo "config:"
            echo "  debug   -   build with the debug configuration"
            echo "  release -   build with the release configuration"
            echo ""
            echo "options:"
            echo "  --universal   -   build universal binary (x86_64 + arm64)"
            echo ""
            echo "Examples:"
            echo "  ./build_macos.sh release              # Build for native architecture"
            echo "  ./build_macos.sh release --universal  # Build universal binary"
            echo ""
            exit 1
            ;;
    esac
done

# Check if config is set
if [ -z "$CONFIG" ]; then
    echo "Error: Configuration not specified!"
    echo ""
    echo "Usage: ./build_macos.sh [config] [options]"
    echo ""
    echo "config:"
    echo "  debug   -   build with the debug configuration"
    echo "  release -   build with the release configuration"
    echo ""
    echo "options:"
    echo "  --universal   -   build universal binary (x86_64 + arm64)"
    echo ""
    exit 1
fi

# Detect current architecture
ARCH=$(uname -m)
echo "Detected architecture: $ARCH"

# Configure CMake
if [ $BUILD_UNIVERSAL -eq 1 ]; then
    echo "Building Universal Binary (x86_64 + arm64)..."
    cmake -S . -B build -G "Xcode" -DBUILD_UNIVERSAL_BINARY=ON
else
    echo "Building for native architecture ($ARCH)..."
    cmake -S . -B build -G "Xcode"
fi

# Build
echo "Building with $CONFIG configuration..."
cmake --build build --config "${CONFIG}"

echo ""
echo "Build completed successfully!"
echo "Binary location: bin/"
