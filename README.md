# Adaptive LLVM Obfuscator

A production-grade, machine-learning-augmented LLVM obfuscation framework with advanced static and dynamic code transformations.

## Features

### Core Obfuscation Techniques
- **Control Flow Flattening**: Converts control flow to switch-based dispatchers
- **Bogus Code Injection**: Inserts realistic but non-executing code
- **String Obfuscation**: Encrypts strings with runtime decryption
- **Instruction Substitution**: Replaces instructions with complex equivalents
- **Opaque Predicates**: Adds always-true/false conditions
- **Fake Loop Generation**: Creates non-executing loops
- **Anti-Debugging**: Inserts debugger detection checks
- **Code Watermarking**: Embeds tamper-evident code markers

### Advanced Features
- **AI-Driven Profiling**: ML model analyzes programs and generates custom obfuscation strategies
- **JIT Stubs**: Runtime code morphing with encrypted function stubs
- **Plugin System**: Extensible architecture for custom obfuscation passes
- **Comprehensive Reporting**: JSON, CSV, HTML, and PDF reports with detailed metrics

## Building

### Prerequisites
- LLVM 14+ with development headers
- CMake 3.20+
- C++17 compatible compiler
- (Optional) pdflatex for PDF reports

### Build Steps

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Installation

```bash
sudo make install
```

## Usage

### Basic Obfuscation

```bash
obfuscator-cli input.ll -o obfuscated.ll
```

### Advanced Usage

```bash
# Multiple obfuscation cycles with custom intensity
obfuscator-cli program.ll -o output.ll \\
    --cycles 3 \\
    --cf-intensity 0.9 \\
    --bc-intensity 0.7 \\
    -report ./reports \\
    -report-formats json,html

# With code watermarking
obfuscator-cli code.ll \\
    --watermark "MyCompany v1.0" \\
    -o protected.ll

# Using custom plugins
obfuscator-cli input.ll \\
    --plugin ./plugins/custom_pass.so \\
    -o output.ll

# Deterministic obfuscation
obfuscator-cli input.ll \\
    --seed 12345 \\
    -o output.ll
```

## Command Line Options

### Input/Output
- `-o <file>`: Output file path
- `-format <fmt>`: Output format (ir, bitcode)

### Obfuscation Control
- `-cycles <n>`: Number of obfuscation cycles
- `--cf-intensity <0-1>`: Control flow flattening intensity
- `--bc-intensity <0-1>`: Bogus code injection intensity  
- `--str-intensity <0-1>`: String obfuscation intensity

### Pass Control
- `--no-control-flow`: Disable control flow flattening
- `--no-bogus-code`: Disable bogus code injection
- `--no-string-obf`: Disable string obfuscation
- `--no-inst-sub`: Disable instruction substitution
- `--no-opaque`: Disable opaque predicates
- `--no-fake-loops`: Disable fake loop generation
- `--no-anti-debug`: Disable anti-debugging
- `--no-jit`: Disable JIT stubs
- `--no-ai`: Disable AI profiler

### Advanced
- `--watermark <text>`: Embed watermark payload
- `--plugin <path>`: Load custom plugin
- `--seed <n>`: Random seed for deterministic obfuscation
- `-report <dir>`: Report output directory
- `-report-formats <list>`: Report formats (comma-separated)

## Architecture

```
src/
├── core/           # Core obfuscation engine
├── passes/         # LLVM transformation passes
├── ai/             # AI profiler and ML model
├── jit/            # JIT engine and runtime obfuscation
├── utils/          # Utilities (crypto, random, logging)
└── cli/            # Command-line interface

include/            # Public headers
plugins/            # Plugin examples
examples/           # Example programs
tests/              # Test suite
```

## Creating Custom Plugins

```cpp
#include "PluginAPI.h"

class MyCustomPass : public llvm::FunctionPass {
    // Implement your custom obfuscation
};

class MyPlugin : public obfuscator::IObfuscationPlugin {
public:
    obfuscator::PluginInfo getInfo() const override {
        return {"MyPlugin", "1.0", "Author", "Description", 1};
    }
    
    llvm::Pass* createPass() override {
        return new MyCustomPass();
    }
    
    bool initialize(const std::map<std::string, std::string>& config) override {
        return true;
    }
    
    void cleanup() override {}
};

extern "C" {
    obfuscator::IObfuscationPlugin* createPlugin() {
        return new MyPlugin();
    }
    
    void destroyPlugin(obfuscator::IObfuscationPlugin* plugin) {
        delete plugin;
    }
}
```

## Report Format

The obfuscator generates comprehensive reports with:

- Input/output file statistics
- Transformation counts per pass
- Code entropy analysis
- Execution time metrics
- AI profiler recommendations
- Per-pass detailed metrics

## Performance

Typical obfuscation overhead:
- **Size increase**: 2-3x original size
- **Compile time**: ~5-10 seconds per MLOC
- **Runtime overhead**: 10-30% (depends on JIT stub usage)

## Examples

See `examples/` directory for:
- Simple C program obfuscation
- Complex application protection
- Plugin development examples
- Integration with build systems

## Testing

```bash
cd build
ctest --verbose
```

## License

[Specify your license here]

## Contributing

Contributions welcome! Please see CONTRIBUTING.md

## Citation

If you use this tool in research, please cite:
```bibtex
@software{adaptive_obfuscator_2025,
  title={Adaptive LLVM Obfuscator},
  author={Your Name},
  year={2025}
}
```

## Acknowledgments

Inspired by:
- Obfuscator-LLVM (O-LLVM)
- Tigress C Obfuscator
- Academic research in software protection
