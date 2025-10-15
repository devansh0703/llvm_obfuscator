#!/bin/bash

# Installation script

set -e

echo "======================================"
echo "Installing Adaptive LLVM Obfuscator"
echo "======================================"
echo ""

# Check if built
if [ ! -f "build/obfuscator-cli" ]; then
    echo "Error: Project not built. Please run ./build.sh first."
    exit 1
fi

# Install
cd build
sudo make install

echo ""
echo "======================================"
echo "Installation complete!"
echo "======================================"
echo ""
echo "Installed files:"
echo "  /usr/local/bin/obfuscator-cli"
echo "  /usr/local/lib/libObfuscatorCore.so"
echo "  /usr/local/include/obfuscator/"
echo ""
echo "Usage:"
echo "  obfuscator-cli input.ll -o output.ll"
echo ""
