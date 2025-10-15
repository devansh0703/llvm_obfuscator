#ifndef OBFUSCATION_CONFIG_H
#define OBFUSCATION_CONFIG_H

#include <string>
#include <vector>
#include <map>

namespace obfuscator {

struct ObfuscationConfig {
    // General settings
    int obfuscationCycles = 1;
    bool enableAIProfiler = true;
    bool enableReporting = true;
    std::string outputFormat = "binary"; // binary, ir, bitcode
    
    // Pass enable/disable flags
    bool enableControlFlowFlattening = true;
    bool enableBogusCodeInjection = true;
    bool enableStringObfuscation = true;
    bool enableInstructionSubstitution = true;
    bool enableOpaquePredicates = true;
    bool enableFakeLoops = true;
    bool enableWatermarking = false;
    bool enableAntiDebugging = true;
    bool enableJITStubs = true;
    
    // Intensity parameters (0.0 - 1.0)
    float bogusCodeIntensity = 0.5f;
    float controlFlowIntensity = 0.7f;
    float stringObfuscationIntensity = 1.0f;
    
    // Specific parameters
    int maxBogusCodePercent = 30;
    int fakeFunctionCount = 10;
    int fakeLoopCount = 50;
    int opaquePredicateCount = 100;
    
    // Watermarking
    std::string watermarkPayload;
    
    // JIT settings
    std::vector<std::string> jitTargetFunctions;
    bool jitAllCriticalFunctions = true;
    
    // Plugin paths
    std::vector<std::string> pluginPaths;
    
    // Reporting
    std::vector<std::string> reportFormats; // json, csv, pdf, html
    std::string reportOutputPath;
    
    // Advanced options
    bool preserveFunctionSignatures = false;
    bool deterministicObfuscation = false;
    unsigned int randomSeed = 0;
    
    // Target platform
    std::string targetTriple;
    std::string targetCPU;
    std::string targetFeatures;
};

} // namespace obfuscator

#endif // OBFUSCATION_CONFIG_H
