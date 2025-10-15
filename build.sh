#!/bin/bash

# Build script for Adaptive Obfuscator

set -e

echo "======================================"
echo "Building Adaptive LLVM Obfuscator"
echo "======================================"
echo ""

# Check for LLVM
if ! command -v llvm-config &> /dev/null; then
    echo "Error: LLVM not found. Please install LLVM development libraries."
    echo ""
    echo "Ubuntu/Debian:"
    echo "  sudo apt-get install llvm-14-dev clang-14"
    echo ""
    echo "macOS:"
    echo "  brew install llvm"
    echo ""
    exit 1
fi

LLVM_VERSION=$(llvm-config --version)
echo "Found LLVM version: $LLVM_VERSION"
echo ""

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure
echo "[1/3] Configuring..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "[2/3] Building..."
make -j$(nproc)

# Test
echo ""
echo "[3/3] Running tests..."
ctest --output-on-failure

echo ""
echo "======================================"
echo "Build completed successfully!"
echo "======================================"
echo ""
echo "Executable: build/obfuscator-cli"
echo "Library:    build/libObfuscatorCore.so"
echo ""
echo "To install system-wide:"
echo "  sudo make install"
echo ""
echo "To run examples:"
echo "  ./examples/run_example.sh"
echo ""
