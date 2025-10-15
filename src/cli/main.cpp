#include "ObfuscationEngine.h"
#include "utils/Logger.h"
#include <iostream>
#include <string>
#include <map>

void printUsage(const char* programName) {
    std::cout << "Adaptive LLVM Obfuscator - Advanced Code Protection Tool\n\n";
    std::cout << "Usage: " << programName << " [options] <input-file>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output>          Output file path (default: output.ll)\n";
    std::cout << "  -cycles <n>          Number of obfuscation cycles (default: 1)\n";
    std::cout << "  -format <fmt>        Output format: ir, bitcode (default: ir)\n";
    std::cout << "  -report <dir>        Report output directory\n";
    std::cout << "  -report-formats <f>  Report formats: json,csv,html,pdf (default: json)\n";
    std::cout << "\nObfuscation Passes:\n";
    std::cout << "  --no-control-flow    Disable control flow flattening\n";
    std::cout << "  --no-bogus-code      Disable bogus code injection\n";
    std::cout << "  --no-string-obf      Disable string obfuscation\n";
    std::cout << "  --no-inst-sub        Disable instruction substitution\n";
    std::cout << "  --no-opaque          Disable opaque predicates\n";
    std::cout << "  --no-fake-loops      Disable fake loop generation\n";
    std::cout << "  --no-anti-debug      Disable anti-debugging\n";
    std::cout << "  --no-jit             Disable JIT stubs\n";
    std::cout << "\nIntensity (0.0 - 1.0):\n";
    std::cout << "  --cf-intensity <f>   Control flow intensity (default: 0.7)\n";
    std::cout << "  --bc-intensity <f>   Bogus code intensity (default: 0.5)\n";
    std::cout << "  --str-intensity <f>  String obfuscation intensity (default: 1.0)\n";
    std::cout << "\nAdvanced:\n";
    std::cout << "  --watermark <text>   Embed watermark payload\n";
    std::cout << "  --plugin <path>      Load obfuscation plugin\n";
    std::cout << "  --no-ai              Disable AI profiler\n";
    std::cout << "  --seed <n>           Random seed for deterministic obfuscation\n";
    std::cout << "  -v, --verbose        Verbose output\n";
    std::cout << "  -h, --help           Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " input.ll -o obfuscated.ll\n";
    std::cout << "  " << programName << " program.bc --cycles 3 --cf-intensity 0.9\n";
    std::cout << "  " << programName << " code.ll --watermark \"MyApp v1.0\" -report ./reports\n\n";
}

int main(int argc, char** argv) {
    using namespace obfuscator;
    
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Initialize logger
    Logger::init();
    Logger::setConsoleOutput(true);
    Logger::setLevel(LogLevel::INFO);
    
    // Parse command line arguments
    std::string inputFile;
    std::string outputFile = "output.ll";
    ObfuscationConfig config;
    
    config.reportOutputPath = "./reports";
    config.reportFormats.push_back("json");
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            Logger::setLevel(LogLevel::DEBUG);
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "-cycles" && i + 1 < argc) {
            config.obfuscationCycles = std::stoi(argv[++i]);
        } else if (arg == "-format" && i + 1 < argc) {
            config.outputFormat = argv[++i];
        } else if (arg == "-report" && i + 1 < argc) {
            config.reportOutputPath = argv[++i];
        } else if (arg == "-report-formats" && i + 1 < argc) {
            config.reportFormats.clear();
            std::string formats = argv[++i];
            size_t pos = 0;
            while ((pos = formats.find(',')) != std::string::npos) {
                config.reportFormats.push_back(formats.substr(0, pos));
                formats.erase(0, pos + 1);
            }
            if (!formats.empty()) {
                config.reportFormats.push_back(formats);
            }
        } else if (arg == "--no-control-flow") {
            config.enableControlFlowFlattening = false;
        } else if (arg == "--no-bogus-code") {
            config.enableBogusCodeInjection = false;
        } else if (arg == "--no-string-obf") {
            config.enableStringObfuscation = false;
        } else if (arg == "--no-inst-sub") {
            config.enableInstructionSubstitution = false;
        } else if (arg == "--no-opaque") {
            config.enableOpaquePredicates = false;
        } else if (arg == "--no-fake-loops") {
            config.enableFakeLoops = false;
        } else if (arg == "--no-anti-debug") {
            config.enableAntiDebugging = false;
        } else if (arg == "--no-jit") {
            config.enableJITStubs = false;
        } else if (arg == "--cf-intensity" && i + 1 < argc) {
            config.controlFlowIntensity = std::stof(argv[++i]);
        } else if (arg == "--bc-intensity" && i + 1 < argc) {
            config.bogusCodeIntensity = std::stof(argv[++i]);
        } else if (arg == "--str-intensity" && i + 1 < argc) {
            config.stringObfuscationIntensity = std::stof(argv[++i]);
        } else if (arg == "--watermark" && i + 1 < argc) {
            config.watermarkPayload = argv[++i];
            config.enableWatermarking = true;
        } else if (arg == "--plugin" && i + 1 < argc) {
            config.pluginPaths.push_back(argv[++i]);
        } else if (arg == "--no-ai") {
            config.enableAIProfiler = false;
        } else if (arg == "--seed" && i + 1 < argc) {
            config.randomSeed = std::stoul(argv[++i]);
            config.deterministicObfuscation = true;
        } else if (arg[0] != '-') {
            inputFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }
    
    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // Print configuration
    Logger::info("========================================");
    Logger::info("Adaptive LLVM Obfuscator");
    Logger::info("========================================");
    Logger::info("Input:  " + inputFile);
    Logger::info("Output: " + outputFile);
    Logger::info("Cycles: " + std::to_string(config.obfuscationCycles));
    Logger::info("========================================");
    
    // Create obfuscation engine
    ObfuscationEngine engine;
    
    // Run obfuscation
    bool success = engine.obfuscate(inputFile, outputFile, config);
    
    if (success) {
        Logger::info("========================================");
        Logger::info("Obfuscation completed successfully!");
        Logger::info("========================================");
        
        const auto& report = engine.getReport();
        
        std::cout << "\nSummary:\n";
        std::cout << "  Input size:        " << report.inputFileSize << " bytes\n";
        std::cout << "  Output size:       " << report.outputFileSize << " bytes\n";
        std::cout << "  Size increase:     " << (report.outputFileSize - report.inputFileSize) << " bytes\n";
        std::cout << "  Instructions:      " << report.inputInstructionCount << " -> " << report.outputInstructionCount << "\n";
        std::cout << "  Entropy increase:  " << (report.entropyDelta * 100) << "%\n";
        std::cout << "  Execution time:    " << report.totalObfuscationTimeMs << " ms\n";
        std::cout << "\nTransformations:\n";
        std::cout << "  Bogus code:        " << report.bogusCodeInsertions << "\n";
        std::cout << "  Fake loops:        " << report.fakeLoopsGenerated << "\n";
        std::cout << "  Strings obfuscated:" << report.stringObfuscationCount << "\n";
        std::cout << "  Opaque predicates: " << report.opaquePredicatesAdded << "\n";
        std::cout << "  JIT stubs:         " << report.jitStubsCreated << "\n";
        std::cout << "\n";
        
        return 0;
    } else {
        Logger::error("========================================");
        Logger::error("Obfuscation failed!");
        Logger::error("========================================");
        return 1;
    }
}
