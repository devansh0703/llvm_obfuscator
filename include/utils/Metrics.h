#ifndef UTILS_METRICS_H
#define UTILS_METRICS_H

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

namespace obfuscator {

class Metrics {
public:
    // Count instructions in module
    static int countInstructions(const llvm::Module& M);
    
    // Count instructions in function
    static int countInstructions(const llvm::Function& F);
    
    // Count basic blocks
    static int countBasicBlocks(const llvm::Module& M);
    static int countBasicBlocks(const llvm::Function& F);
    
    // Count functions
    static int countFunctions(const llvm::Module& M);
    
    // Calculate code size estimate
    static size_t estimateCodeSize(const llvm::Module& M);
};

} // namespace obfuscator

#endif // UTILS_METRICS_H
