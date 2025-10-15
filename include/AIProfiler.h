#ifndef AI_PROFILER_H
#define AI_PROFILER_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "llvm/IR/Module.h"
#include "ObfuscationConfig.h"

namespace obfuscator {

struct ProgramFeatures {
    // Code structure features
    int functionCount = 0;
    int basicBlockCount = 0;
    int instructionCount = 0;
    int loopCount = 0;
    int branchCount = 0;
    
    // Complexity metrics
    double averageCyclomaticComplexity = 0.0;
    double maxCyclomaticComplexity = 0.0;
    int callGraphDepth = 0;
    int maxNestingLevel = 0;
    
    // Data flow features
    int globalVariables = 0;
    int pointerOperations = 0;
    int arrayAccesses = 0;
    
    // Code patterns
    int stringCount = 0;
    int constantCount = 0;
    int switchStatements = 0;
    
    // Advanced metrics
    double codeEntropy = 0.0;
    std::map<std::string, int> instructionDistribution;
};

struct ObfuscationStrategy {
    // Recommended pass intensities
    float controlFlowIntensity = 0.5f;
    float bogusCodeIntensity = 0.5f;
    float stringObfuscationIntensity = 0.5f;
    
    // Recommended counts
    int fakeLoopCount = 50;
    int opaquePredicateCount = 100;
    int bogusInstructionCount = 1000;
    
    // Priority functions for JIT
    std::vector<std::string> criticalFunctions;
    
    // Recommended passes
    std::vector<std::string> recommendedPasses;
    
    // Confidence scores
    std::map<std::string, float> confidenceScores;
    
    // Strategy name
    std::string strategyName;
};

class ProgramAnalyzer;
class MLModel;

class AIProfiler {
public:
    AIProfiler();
    ~AIProfiler();
    
    // Analyze program and generate obfuscation strategy
    ObfuscationStrategy profileProgram(const llvm::Module& module);
    
    // Update strategy based on config
    void refineStrategy(ObfuscationStrategy& strategy, 
                       const ObfuscationConfig& config);
    
    // Get program features
    ProgramFeatures extractFeatures(const llvm::Module& module);
    
    // Train/update ML model (if enabled)
    bool trainModel(const std::vector<ProgramFeatures>& features,
                   const std::vector<ObfuscationStrategy>& strategies);
    
    // Load/save model
    bool loadModel(const std::string& path);
    bool saveModel(const std::string& path);
    
private:
    std::unique_ptr<ProgramAnalyzer> analyzer_;
    std::unique_ptr<MLModel> model_;
    
    // Default strategy generation
    ObfuscationStrategy generateDefaultStrategy(const ProgramFeatures& features);
    
    // Identify critical functions
    std::vector<std::string> identifyCriticalFunctions(const llvm::Module& module,
                                                       const ProgramFeatures& features);
};

} // namespace obfuscator

#endif // AI_PROFILER_H
