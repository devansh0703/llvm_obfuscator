#include "AIProfiler.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include <cassert>

extern void test_ai_profiler();

void test_ai_profiler() {
    llvm::LLVMContext ctx;
    llvm::Module M("test_module", ctx);
    
    obfuscator::AIProfiler profiler;
    
    // Profile a simple module
    obfuscator::ObfuscationStrategy strategy = profiler.profileProgram(M);
    
    // Should return a valid strategy
    assert(!strategy.strategyName.empty());
    assert(strategy.controlFlowIntensity >= 0.0f && strategy.controlFlowIntensity <= 1.0f);
    assert(strategy.bogusCodeIntensity >= 0.0f && strategy.bogusCodeIntensity <= 1.0f);
}
