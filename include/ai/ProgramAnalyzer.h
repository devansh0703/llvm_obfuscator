#ifndef PROGRAM_ANALYZER_H
#define PROGRAM_ANALYZER_H

#include "AIProfiler.h"
#include "llvm/IR/Module.h"

namespace obfuscator {

class ProgramAnalyzer {
public:
    ProgramAnalyzer();
    ~ProgramAnalyzer();
    
    // Analyze module and extract features
    ProgramFeatures analyze(const llvm::Module& module);
    
private:
    // Calculate cyclomatic complexity
    double calculateCyclomaticComplexity(const llvm::Function& F);
    
    // Calculate nesting level
    int calculateNestingLevel(const llvm::Function& F);
    
    // Count specific patterns
    int countLoops(const llvm::Function& F);
    int countBranches(const llvm::Function& F);
    
    // Calculate code entropy
    double calculateEntropy(const llvm::Module& module);
};

} // namespace obfuscator

#endif // PROGRAM_ANALYZER_H
