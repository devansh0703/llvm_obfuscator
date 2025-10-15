#ifndef ANTI_DEBUGGING_H
#define ANTI_DEBUGGING_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

namespace obfuscator {

class AntiDebugging : public llvm::ModulePass {
public:
    static char ID;
    
    AntiDebugging();
    
    bool runOnModule(llvm::Module &M) override;
    
    llvm::StringRef getPassName() const override {
        return "Anti-Debugging";
    }
    
    int getCheckCount() const { return checkCount_; }
    
private:
    int checkCount_;
    
    // Create debugger detection function
    llvm::Function* createDebuggerCheckFunction(llvm::Module &M);
    
    // Insert anti-debug checks
    bool insertAntiDebugChecks(llvm::Module &M, llvm::Function* checkFunc);
    
    // Create timing-based detection
    llvm::Function* createTimingCheckFunction(llvm::Module &M);
};

} // namespace obfuscator

#endif // ANTI_DEBUGGING_H
