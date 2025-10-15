#include "ObfuscationEngine.h"
#include "AIProfiler.h"
#include "JITEngine.h"
#include "PluginLoader.h"
#include "InputHandler.h"
#include "passes/ControlFlowFlattening.h"
#include "passes/BogusCodeInjection.h"
#include "passes/StringObfuscation.h"
#include "passes/InstructionSubstitution.h"
#include "passes/OpaquePredicates.h"
#include "passes/FakeLoopGeneration.h"
#include "passes/CodeWatermarking.h"
#include "passes/AntiDebugging.h"
#include "utils/Logger.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>
#include <fstream>

namespace obfuscator {

ObfuscationEngine::ObfuscationEngine() 
    : context_(std::make_unique<llvm::LLVMContext>()),
      aiProfiler_(std::make_unique<AIProfiler>()),
      jitEngine_(std::make_unique<JITEngine>()),
      pluginLoader_(std::make_unique<PluginLoader>()),
      reportingModule_(std::make_unique<ReportingModule>()) {
    Logger::info("ObfuscationEngine initialized");
}

ObfuscationEngine::~ObfuscationEngine() {
    Logger::info("ObfuscationEngine destroyed");
}

bool ObfuscationEngine::obfuscate(const std::string& inputPath,
                                  const std::string& outputPath,
                                  const ObfuscationConfig& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    Logger::info("Starting obfuscation of: " + inputPath);
    
    // Reset report
    report_ = ObfuscationReport();
    report_.inputFile = inputPath;
    report_.outputFile = outputPath;
    report_.timestamp = std::chrono::system_clock::now();
    
    // Get input file size
    std::ifstream inputFile(inputPath, std::ios::binary | std::ios::ate);
    if (inputFile.is_open()) {
        report_.inputFileSize = inputFile.tellg();
        inputFile.close();
    }
    
    // Load module
    llvm::SMDiagnostic err;
    std::unique_ptr<llvm::Module> module = llvm::parseIRFile(inputPath, err, *context_);
    
    if (!module) {
        Logger::error("Failed to load module: " + err.getMessage().str());
        return false;
    }
    
    Logger::info("Module loaded successfully");
    
    // Collect initial metrics
    report_.inputInstructionCount = 0;
    report_.inputFunctionCount = 0;
    report_.inputBasicBlockCount = 0;
    
    for (auto& F : *module) {
        if (!F.isDeclaration()) {
            report_.inputFunctionCount++;
            for (auto& BB : F) {
                report_.inputBasicBlockCount++;
                report_.inputInstructionCount += BB.size();
            }
        }
    }
    
    // Calculate input entropy
    report_.codeEntropyBefore = ReportingModule::calculateEntropy(inputPath);
    
    // Load plugins if specified
    for (const auto& pluginPath : config.pluginPaths) {
        if (!loadPlugin(pluginPath)) {
            Logger::warning("Failed to load plugin: " + pluginPath);
        }
    }
    
    // AI Profiler - analyze and generate strategy
    ObfuscationStrategy strategy;
    if (config.enableAIProfiler) {
        Logger::info("Running AI profiler...");
        strategy = aiProfiler_->profileProgram(*module);
        report_.aiObfuscationStrategy = strategy.strategyName;
        report_.aiConfidenceScores = strategy.confidenceScores;
        Logger::info("AI strategy: " + strategy.strategyName);
    }
    
    // Initialize passes based on config
    initializePasses(config);
    
    // Run obfuscation cycles
    report_.obfuscationCycles = config.obfuscationCycles;
    for (int cycle = 0; cycle < config.obfuscationCycles; ++cycle) {
        Logger::info("Obfuscation cycle " + std::to_string(cycle + 1));
        if (!runObfuscationCycles(*module, config)) {
            Logger::error("Obfuscation cycle failed");
            return false;
        }
    }
    
    // JIT stub creation
    if (config.enableJITStubs) {
        Logger::info("Creating JIT stubs...");
        jitEngine_->initialize(*module);
        
        std::vector<std::string> targetFuncs = config.jitTargetFunctions;
        if (config.jitAllCriticalFunctions && config.enableAIProfiler) {
            targetFuncs.insert(targetFuncs.end(),
                             strategy.criticalFunctions.begin(),
                             strategy.criticalFunctions.end());
        }
        
        if (jitEngine_->createJITStubs(*module, targetFuncs)) {
            report_.jitStubsCreated = jitEngine_->getStubInfo().size();
            Logger::info("Created " + std::to_string(report_.jitStubsCreated) + " JIT stubs");
        }
    }
    
    // Collect final metrics
    report_.outputInstructionCount = 0;
    report_.outputFunctionCount = 0;
    report_.outputBasicBlockCount = 0;
    
    for (auto& F : *module) {
        if (!F.isDeclaration()) {
            report_.outputFunctionCount++;
            for (auto& BB : F) {
                report_.outputBasicBlockCount++;
                report_.outputInstructionCount += BB.size();
            }
        }
    }
    
    // Write output
    std::error_code EC;
    llvm::raw_fd_ostream outFile(outputPath, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        Logger::error("Failed to open output file: " + EC.message());
        return false;
    }
    
    if (config.outputFormat == "bitcode") {
        llvm::WriteBitcodeToFile(*module, outFile);
    } else {
        module->print(outFile, nullptr);
    }
    
    outFile.close();
    Logger::info("Output written to: " + outputPath);
    
    // Get output file size
    std::ifstream outputFile(outputPath, std::ios::binary | std::ios::ate);
    if (outputFile.is_open()) {
        report_.outputFileSize = outputFile.tellg();
        outputFile.close();
    }
    
    // Calculate output entropy
    report_.codeEntropyAfter = ReportingModule::calculateEntropy(outputPath);
    report_.entropyDelta = report_.codeEntropyAfter - report_.codeEntropyBefore;
    
    // Calculate total time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    report_.totalObfuscationTimeMs = duration.count();
    
    Logger::info("Obfuscation completed in " + std::to_string(report_.totalObfuscationTimeMs) + " ms");
    
    // Generate reports
    if (config.enableReporting) {
        for (const auto& format : config.reportFormats) {
            std::string reportPath = config.reportOutputPath + "/report." + format;
            
            if (format == "json") {
                reportingModule_->generateJSON(report_, reportPath);
            } else if (format == "csv") {
                reportingModule_->generateCSV(report_, reportPath);
            } else if (format == "html") {
                reportingModule_->generateHTML(report_, reportPath);
            } else if (format == "pdf") {
                reportingModule_->generatePDF(report_, reportPath);
            }
            
            Logger::info("Report generated: " + reportPath);
        }
    }
    
    return true;
}

void ObfuscationEngine::initializePasses(const ObfuscationConfig& config) {
    passes_.clear();
    
    if (config.enableControlFlowFlattening) {
        passes_.push_back(std::make_unique<ControlFlowFlattening>(config.controlFlowIntensity));
        Logger::info("Enabled: Control Flow Flattening");
    }
    
    if (config.enableBogusCodeInjection) {
        passes_.push_back(std::make_unique<BogusCodeInjection>(
            config.bogusCodeIntensity, config.maxBogusCodePercent));
        Logger::info("Enabled: Bogus Code Injection");
    }
    
    if (config.enableStringObfuscation) {
        passes_.push_back(std::make_unique<StringObfuscation>(config.stringObfuscationIntensity));
        Logger::info("Enabled: String Obfuscation");
    }
    
    if (config.enableInstructionSubstitution) {
        passes_.push_back(std::make_unique<InstructionSubstitution>());
        Logger::info("Enabled: Instruction Substitution");
    }
    
    if (config.enableOpaquePredicates) {
        passes_.push_back(std::make_unique<OpaquePredicates>(config.opaquePredicateCount));
        Logger::info("Enabled: Opaque Predicates");
    }
    
    if (config.enableFakeLoops) {
        passes_.push_back(std::make_unique<FakeLoopGeneration>(config.fakeLoopCount));
        Logger::info("Enabled: Fake Loop Generation");
    }
    
    if (config.enableWatermarking && !config.watermarkPayload.empty()) {
        passes_.push_back(std::make_unique<CodeWatermarking>(config.watermarkPayload));
        Logger::info("Enabled: Code Watermarking");
    }
    
    if (config.enableAntiDebugging) {
        passes_.push_back(std::make_unique<AntiDebugging>());
        Logger::info("Enabled: Anti-Debugging");
    }
    
    report_.obfuscationPasses = passes_.size();
}

bool ObfuscationEngine::runObfuscationCycles(llvm::Module& module,
                                            const ObfuscationConfig& config) {
    llvm::legacy::PassManager PM;
    
    for (auto& pass : passes_) {
        PM.add(pass.get());
    }
    
    PM.run(module);
    
    return true;
}

bool ObfuscationEngine::loadPlugin(const std::string& pluginPath) {
    return pluginLoader_->loadPlugin(pluginPath);
}

void ObfuscationEngine::unloadAllPlugins() {
    pluginLoader_->unloadAll();
}

} // namespace obfuscator
