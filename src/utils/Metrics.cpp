#include "utils/Metrics.h"

namespace obfuscator {

int Metrics::countInstructions(const llvm::Module& M) {
    int count = 0;
    for (const auto& F : M) {
        if (!F.isDeclaration()) {
            count += countInstructions(F);
        }
    }
    return count;
}

int Metrics::countInstructions(const llvm::Function& F) {
    int count = 0;
    for (const auto& BB : F) {
        count += BB.size();
    }
    return count;
}

int Metrics::countBasicBlocks(const llvm::Module& M) {
    int count = 0;
    for (const auto& F : M) {
        if (!F.isDeclaration()) {
            count += F.size();
        }
    }
    return count;
}

int Metrics::countBasicBlocks(const llvm::Function& F) {
    return F.size();
}

int Metrics::countFunctions(const llvm::Module& M) {
    int count = 0;
    for (const auto& F : M) {
        if (!F.isDeclaration()) {
            count++;
        }
    }
    return count;
}

size_t Metrics::estimateCodeSize(const llvm::Module& M) {
    // Rough estimate: average 4 bytes per instruction
    return countInstructions(M) * 4;
}

} // namespace obfuscator
