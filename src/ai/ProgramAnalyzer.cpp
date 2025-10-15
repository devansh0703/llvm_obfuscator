#include "ai/ProgramAnalyzer.h"
#include "utils/Logger.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include <cmath>
#include <map>

namespace obfuscator {

ProgramAnalyzer::ProgramAnalyzer() = default;
ProgramAnalyzer::~ProgramAnalyzer() = default;

ProgramFeatures ProgramAnalyzer::analyze(const llvm::Module& module) {
    ProgramFeatures features;
    
    double totalComplexity = 0.0;
    features.maxCyclomaticComplexity = 0.0;
    
    for (const auto& F : module) {
        if (F.isDeclaration()) continue;
        
        features.functionCount++;
        
        // Count basic blocks and instructions
        for (const auto& BB : F) {
            features.basicBlockCount++;
            features.instructionCount += BB.size();
            
            // Analyze instructions
            for (const auto& I : BB) {
                std::string opName = I.getOpcodeName();
                features.instructionDistribution[opName]++;
                
                // Count specific patterns
                if (llvm::isa<llvm::BranchInst>(&I)) {
                    features.branchCount++;
                }
                
                if (llvm::isa<llvm::SwitchInst>(&I)) {
                    features.switchStatements++;
                }
                
                if (llvm::isa<llvm::LoadInst>(&I) || llvm::isa<llvm::StoreInst>(&I)) {
                    if (I.getOperand(0)->getType()->isPointerTy()) {
                        features.pointerOperations++;
                    }
                }
                
                if (llvm::isa<llvm::GetElementPtrInst>(&I)) {
                    features.arrayAccesses++;
                }
                
                if (llvm::isa<llvm::ConstantInt>(&I) || llvm::isa<llvm::ConstantFP>(&I)) {
                    features.constantCount++;
                }
            }
        }
        
        // Calculate complexity metrics
        double complexity = calculateCyclomaticComplexity(F);
        totalComplexity += complexity;
        features.maxCyclomaticComplexity = std::max(
            features.maxCyclomaticComplexity, complexity);
        
        // Count loops
        features.loopCount += countLoops(F);
        
        // Calculate nesting
        int nesting = calculateNestingLevel(F);
        features.maxNestingLevel = std::max(features.maxNestingLevel, nesting);
    }
    
    // Calculate averages
    if (features.functionCount > 0) {
        features.averageCyclomaticComplexity = 
            totalComplexity / features.functionCount;
    }
    
    // Count global variables
    for (const auto& GV : module.globals()) {
        if (!GV.isDeclaration()) {
            features.globalVariables++;
        }
        
        // Check if it's a string
        if (GV.hasInitializer()) {
            if (auto* init = llvm::dyn_cast<llvm::ConstantDataArray>(GV.getInitializer())) {
                if (init->isString()) {
                    features.stringCount++;
                }
            }
        }
    }
    
    // Calculate entropy
    features.codeEntropy = calculateEntropy(module);
    
    // Estimate call graph depth (simplified)
    features.callGraphDepth = std::min(10, features.functionCount / 5);
    
    Logger::info("Program analysis complete:");
    Logger::info("  Functions: " + std::to_string(features.functionCount));
    Logger::info("  Instructions: " + std::to_string(features.instructionCount));
    Logger::info("  Avg Complexity: " + std::to_string(features.averageCyclomaticComplexity));
    
    return features;
}

double ProgramAnalyzer::calculateCyclomaticComplexity(const llvm::Function& F) {
    int edges = 0;
    int nodes = F.size();
    
    for (const auto& BB : F) {
        if (const auto* term = BB.getTerminator()) {
            edges += term->getNumSuccessors();
        }
    }
    
    // M = E - N + 2
    return static_cast<double>(edges - nodes + 2);
}

int ProgramAnalyzer::calculateNestingLevel(const llvm::Function& F) {
    int maxNesting = 0;
    
    // Simplified: count nested control flow structures
    for (const auto& BB : F) {
        int currentNesting = 0;
        
        for (const auto* pred : llvm::predecessors(&BB)) {
            if (pred != &BB) {
                currentNesting++;
            }
        }
        
        maxNesting = std::max(maxNesting, currentNesting);
    }
    
    return maxNesting;
}

int ProgramAnalyzer::countLoops(const llvm::Function& F) {
    int loopCount = 0;
    
    // Simplified loop detection: look for back edges
    for (const auto& BB : F) {
        if (const auto* term = BB.getTerminator()) {
            for (unsigned i = 0; i < term->getNumSuccessors(); ++i) {
                llvm::BasicBlock* succ = term->getSuccessor(i);
                
                // Check if successor dominates current block (back edge)
                // This is a simplified heuristic
                if (succ <= &BB) {
                    loopCount++;
                    break;
                }
            }
        }
    }
    
    return loopCount;
}

int ProgramAnalyzer::countBranches(const llvm::Function& F) {
    int branchCount = 0;
    
    for (const auto& BB : F) {
        for (const auto& I : BB) {
            if (llvm::isa<llvm::BranchInst>(&I) || llvm::isa<llvm::SwitchInst>(&I)) {
                branchCount++;
            }
        }
    }
    
    return branchCount;
}

double ProgramAnalyzer::calculateEntropy(const llvm::Module& module) {
    std::map<std::string, int> opcodeFreq;
    int totalInstructions = 0;
    
    for (const auto& F : module) {
        if (F.isDeclaration()) continue;
        
        for (const auto& BB : F) {
            for (const auto& I : BB) {
                opcodeFreq[I.getOpcodeName()]++;
                totalInstructions++;
            }
        }
    }
    
    if (totalInstructions == 0) {
        return 0.0;
    }
    
    double entropy = 0.0;
    for (const auto& [opcode, freq] : opcodeFreq) {
        double probability = static_cast<double>(freq) / totalInstructions;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy;
}

} // namespace obfuscator
