#include "AIProfiler.h"
#include "ai/ProgramAnalyzer.h"
#include "ai/MLModel.h"
#include "utils/Logger.h"
#include "llvm/IR/CFG.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include <algorithm>
#include <cmath>

namespace obfuscator {

AIProfiler::AIProfiler()
    : analyzer_(std::make_unique<ProgramAnalyzer>()),
      model_(std::make_unique<MLModel>()) {
    Logger::info("AIProfiler initialized");
}

AIProfiler::~AIProfiler() = default;

ObfuscationStrategy AIProfiler::profileProgram(const llvm::Module& module) {
    Logger::info("Profiling program: " + module.getName().str());
    
    // Extract program features
    ProgramFeatures features = extractFeatures(module);
    
    // Use ML model to generate strategy if available
    ObfuscationStrategy strategy;
    
    if (model_->isLoaded()) {
        Logger::info("Using ML model for strategy generation");
        strategy = model_->predictStrategy(features);
    } else {
        Logger::info("Using rule-based strategy generation");
        strategy = generateDefaultStrategy(features);
    }
    
    // Identify critical functions
    strategy.criticalFunctions = identifyCriticalFunctions(module, features);
    
    Logger::info("Generated strategy: " + strategy.strategyName);
    
    return strategy;
}

void AIProfiler::refineStrategy(ObfuscationStrategy& strategy,
                               const ObfuscationConfig& config) {
    // Adjust strategy based on user configuration
    if (config.controlFlowIntensity > 0.0f) {
        strategy.controlFlowIntensity = config.controlFlowIntensity;
    }
    
    if (config.bogusCodeIntensity > 0.0f) {
        strategy.bogusCodeIntensity = config.bogusCodeIntensity;
    }
    
    if (config.stringObfuscationIntensity > 0.0f) {
        strategy.stringObfuscationIntensity = config.stringObfuscationIntensity;
    }
}

ProgramFeatures AIProfiler::extractFeatures(const llvm::Module& module) {
    return analyzer_->analyze(module);
}

bool AIProfiler::trainModel(const std::vector<ProgramFeatures>& features,
                           const std::vector<ObfuscationStrategy>& strategies) {
    if (features.size() != strategies.size() || features.empty()) {
        Logger::error("Invalid training data");
        return false;
    }
    
    Logger::info("Training ML model with " + std::to_string(features.size()) + " samples");
    return model_->train(features, strategies);
}

bool AIProfiler::loadModel(const std::string& path) {
    Logger::info("Loading ML model from: " + path);
    return model_->load(path);
}

bool AIProfiler::saveModel(const std::string& path) {
    Logger::info("Saving ML model to: " + path);
    return model_->save(path);
}

ObfuscationStrategy AIProfiler::generateDefaultStrategy(const ProgramFeatures& features) {
    ObfuscationStrategy strategy;
    strategy.strategyName = "RuleBased";
    
    // Calculate complexity-based intensities
    float complexityFactor = std::min(1.0f, 
        static_cast<float>(features.averageCyclomaticComplexity) / 20.0f);
    
    // High complexity programs need less aggressive obfuscation
    strategy.controlFlowIntensity = std::max(0.3f, 0.8f - complexityFactor * 0.3f);
    strategy.bogusCodeIntensity = std::max(0.2f, 0.6f - complexityFactor * 0.2f);
    strategy.stringObfuscationIntensity = 0.9f;
    
    // Calculate recommended counts based on program size
    strategy.fakeLoopCount = features.loopCount * 2;
    strategy.opaquePredicateCount = features.branchCount * 3;
    strategy.bogusInstructionCount = features.instructionCount / 10;
    
    // Determine recommended passes based on features
    strategy.recommendedPasses.push_back("ControlFlowFlattening");
    strategy.recommendedPasses.push_back("StringObfuscation");
    
    if (features.loopCount > 5) {
        strategy.recommendedPasses.push_back("FakeLoopGeneration");
    }
    
    if (features.functionCount > 10) {
        strategy.recommendedPasses.push_back("BogusCodeInjection");
    }
    
    if (features.branchCount > 20) {
        strategy.recommendedPasses.push_back("OpaquePredicates");
    }
    
    // Set confidence scores
    strategy.confidenceScores["controlFlow"] = 0.85f;
    strategy.confidenceScores["bogusCode"] = 0.75f;
    strategy.confidenceScores["stringObf"] = 0.95f;
    
    return strategy;
}

std::vector<std::string> AIProfiler::identifyCriticalFunctions(
    const llvm::Module& module,
    const ProgramFeatures& features) {
    
    std::vector<std::string> criticalFuncs;
    
    // Identify functions based on various criteria
    for (const auto& F : module) {
        if (F.isDeclaration()) continue;
        
        bool isCritical = false;
        
        // Check function size
        size_t instructionCount = 0;
        for (const auto& BB : F) {
            instructionCount += BB.size();
        }
        
        // Large functions are potentially critical
        if (instructionCount > 100) {
            isCritical = true;
        }
        
        // Functions with many basic blocks
        if (F.size() > 20) {
            isCritical = true;
        }
        
        // Functions with specific naming patterns
        std::string name = F.getName().str();
        if (name.find("encrypt") != std::string::npos ||
            name.find("decrypt") != std::string::npos ||
            name.find("verify") != std::string::npos ||
            name.find("validate") != std::string::npos ||
            name.find("auth") != std::string::npos ||
            name.find("check") != std::string::npos) {
            isCritical = true;
        }
        
        if (isCritical) {
            criticalFuncs.push_back(name);
        }
    }
    
    Logger::info("Identified " + std::to_string(criticalFuncs.size()) + " critical functions");
    
    return criticalFuncs;
}

} // namespace obfuscator
