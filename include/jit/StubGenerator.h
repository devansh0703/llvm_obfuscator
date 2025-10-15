#ifndef STUB_GENERATOR_H
#define STUB_GENERATOR_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

namespace obfuscator {

class StubGenerator {
public:
    StubGenerator();
    ~StubGenerator();
    
    // Generate obfuscated stub for function
    llvm::Function* generateStub(llvm::Module& module, llvm::Function* original);
    
private:
    // Create stub prologue
    void createPrologue(llvm::Function* stub, llvm::IRBuilder<>& builder);
    
    // Create stub epilogue
    void createEpilogue(llvm::Function* stub, llvm::IRBuilder<>& builder);
};

} // namespace obfuscator

#endif // STUB_GENERATOR_H
