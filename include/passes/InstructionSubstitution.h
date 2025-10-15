#ifndef INSTRUCTION_SUBSTITUTION_H
#define INSTRUCTION_SUBSTITUTION_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

namespace obfuscator {

class InstructionSubstitution : public llvm::FunctionPass {
public:
    static char ID;
    
    InstructionSubstitution();
    
    bool runOnFunction(llvm::Function &F) override;
    
    llvm::StringRef getPassName() const override {
        return "Instruction Substitution";
    }
    
    int getSubstitutionCount() const { return substitutionCount_; }
    
private:
    int substitutionCount_;
    
    // Substitute arithmetic operations
    bool substituteAdd(llvm::BinaryOperator* inst);
    bool substituteSub(llvm::BinaryOperator* inst);
    bool substituteXor(llvm::BinaryOperator* inst);
    bool substituteAnd(llvm::BinaryOperator* inst);
    bool substituteOr(llvm::BinaryOperator* inst);
    
    // Create equivalent but complex instruction sequences
    llvm::Value* createComplexAdd(llvm::IRBuilder<>& builder,
                                 llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* createComplexSub(llvm::IRBuilder<>& builder,
                                 llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* createComplexXor(llvm::IRBuilder<>& builder,
                                 llvm::Value* lhs, llvm::Value* rhs);
};

} // namespace obfuscator

#endif // INSTRUCTION_SUBSTITUTION_H
