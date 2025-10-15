#ifndef CONTROL_FLOW_FLATTENING_H
#define CONTROL_FLOW_FLATTENING_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

namespace obfuscator {

class ControlFlowFlattening : public llvm::FunctionPass {
public:
    static char ID;
    
    ControlFlowFlattening();
    explicit ControlFlowFlattening(float intensity);
    
    bool runOnFunction(llvm::Function &F) override;
    
    llvm::StringRef getPassName() const override {
        return "Control Flow Flattening";
    }
    
    void setIntensity(float intensity) { intensity_ = intensity; }
    
    int getModificationCount() const { return modificationCount_; }
    
private:
    float intensity_;
    int modificationCount_;
    
    // Flatten function control flow
    bool flattenFunction(llvm::Function &F);
    
    // Create switch dispatcher
    llvm::BasicBlock* createDispatcher(llvm::Function &F,
                                      std::vector<llvm::BasicBlock*>& blocks);
    
    // Convert branches to switch cases
    void convertToSwitchCase(llvm::BasicBlock* BB,
                            llvm::Value* switchVar,
                            int caseValue);
};

} // namespace obfuscator

#endif // CONTROL_FLOW_FLATTENING_H
