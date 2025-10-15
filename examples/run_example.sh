#!/bin/bash

# Example: Obfuscate a simple C program

echo "======================================"
echo "Adaptive Obfuscator - Example Workflow"
echo "======================================"
echo ""

# Step 1: Compile C to LLVM IR
echo "[1] Compiling C to LLVM IR..."
clang -S -emit-llvm examples/simple_program.c -o examples/simple_program.ll

if [ ! -f examples/simple_program.ll ]; then
    echo "Error: Failed to compile to LLVM IR"
    exit 1
fi

echo "✓ IR generated: examples/simple_program.ll"
echo ""

# Step 2: Run obfuscation
echo "[2] Running obfuscation..."
./build/obfuscator-cli examples/simple_program.ll \
    -o examples/obfuscated_program.ll \
    --cycles 2 \
    --cf-intensity 0.8 \
    --bc-intensity 0.6 \
    --watermark "Example v1.0" \
    -report examples/reports \
    -report-formats json,html

if [ $? -ne 0 ]; then
    echo "Error: Obfuscation failed"
    exit 1
fi

echo "✓ Obfuscated code: examples/obfuscated_program.ll"
echo ""

# Step 3: Compile obfuscated IR to executable
echo "[3] Compiling obfuscated IR to executable..."
clang examples/obfuscated_program.ll -o examples/obfuscated_program

if [ ! -f examples/obfuscated_program ]; then
    echo "Error: Failed to compile obfuscated program"
    exit 1
fi

echo "✓ Executable created: examples/obfuscated_program"
echo ""

# Step 4: Test the obfuscated program
echo "[4] Testing obfuscated program..."
echo ""
echo "Test 1: Invalid license"
./examples/obfuscated_program "WRONG_KEY"
echo ""

echo "Test 2: Valid license"
./examples/obfuscated_program "ABC123XYZ"
echo ""

# Step 5: Show report
echo "[5] Obfuscation report:"
echo ""
if [ -f examples/reports/report.json ]; then
    cat examples/reports/report.json | head -n 30
    echo "..."
    echo ""
    echo "Full report: examples/reports/report.json"
    if [ -f examples/reports/report.html ]; then
        echo "HTML report: examples/reports/report.html"
    fi
fi

echo ""
echo "======================================"
echo "Example workflow completed!"
echo "======================================"
