#ifndef FAKE_LOOP_GENERATION_H
#define FAKE_LOOP_GENERATION_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

namespace obfuscator {

class FakeLoopGeneration : public llvm::FunctionPass {
public:
    static char ID;
    
    FakeLoopGeneration();
    explicit FakeLoopGeneration(int count);
    
    bool runOnFunction(llvm::Function &F) override;
    
    llvm::StringRef getPassName() const override {
        return "Fake Loop Generation";
    }
    
    void setCount(int count) { targetCount_ = count; }
    
    int getGeneratedCount() const { return generatedCount_; }
    
private:
    int targetCount_;
    int generatedCount_;
    
    // Create fake loop that never executes
    bool insertFakeLoop(llvm::Function &F, llvm::BasicBlock* insertPoint);
    
    // Create fake loop body
    void createFakeLoopBody(llvm::BasicBlock* loopBody, llvm::IRBuilder<>& builder);
};

} // namespace obfuscator

#endif // FAKE_LOOP_GENERATION_H
