#ifndef BOGUS_CODE_INJECTION_H
#define BOGUS_CODE_INJECTION_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

namespace obfuscator {

class BogusCodeInjection : public llvm::FunctionPass {
public:
    static char ID;
    
    BogusCodeInjection();
    explicit BogusCodeInjection(float intensity, int maxPercent);
    
    bool runOnFunction(llvm::Function &F) override;
    
    llvm::StringRef getPassName() const override {
        return "Bogus Code Injection";
    }
    
    void setIntensity(float intensity) { intensity_ = intensity; }
    void setMaxPercent(int percent) { maxPercent_ = percent; }
    
    int getInsertionCount() const { return insertionCount_; }
    
private:
    float intensity_;
    int maxPercent_;
    int insertionCount_;
    
    // Insert bogus code blocks
    bool insertBogusCode(llvm::Function &F);
    
    // Create bogus basic block
    llvm::BasicBlock* createBogusBlock(llvm::Function &F,
                                      llvm::IRBuilder<>& builder);
    
    // Generate random bogus instructions
    void generateBogusInstructions(llvm::BasicBlock* BB,
                                  llvm::IRBuilder<>& builder,
                                  int count);
    
    // Create dead code that looks realistic
    void createDeadCode(llvm::BasicBlock* BB,
                       llvm::IRBuilder<>& builder);
};

} // namespace obfuscator

#endif // BOGUS_CODE_INJECTION_H
