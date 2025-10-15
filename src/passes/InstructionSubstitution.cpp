#include "passes/InstructionSubstitution.h"
#include "utils/Random.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include <vector>

namespace obfuscator {

char InstructionSubstitution::ID = 0;

InstructionSubstitution::InstructionSubstitution()
    : FunctionPass(ID), substitutionCount_(0) {}

bool InstructionSubstitution::runOnFunction(llvm::Function &F) {
    if (F.isDeclaration()) {
        return false;
    }
    
    Logger::info("Running Instruction Substitution on: " + F.getName().str());
    
    substitutionCount_ = 0;
    std::vector<llvm::Instruction*> toSubstitute;
    
    // Collect instructions to substitute
    for (auto& BB : F) {
        for (auto& I : BB) {
            if (auto* binOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
                if (Random::probability() > 0.7f) {
                    continue;
                }
                toSubstitute.push_back(binOp);
            }
        }
    }
    
    // Perform substitutions
    for (auto* inst : toSubstitute) {
        auto* binOp = llvm::cast<llvm::BinaryOperator>(inst);
        
        switch (binOp->getOpcode()) {
            case llvm::Instruction::Add:
                substituteAdd(binOp);
                break;
            case llvm::Instruction::Sub:
                substituteSub(binOp);
                break;
            case llvm::Instruction::Xor:
                substituteXor(binOp);
                break;
            case llvm::Instruction::And:
                substituteAnd(binOp);
                break;
            case llvm::Instruction::Or:
                substituteOr(binOp);
                break;
            default:
                break;
        }
    }
    
    if (substitutionCount_ > 0) {
        Logger::info("Substituted " + std::to_string(substitutionCount_) + 
                    " instructions in: " + F.getName().str());
    }
    
    return substitutionCount_ > 0;
}

bool InstructionSubstitution::substituteAdd(llvm::BinaryOperator* inst) {
    llvm::IRBuilder<> builder(inst);
    
    // Replace: a + b
    // With: (a - (-b))
    llvm::Value* lhs = inst->getOperand(0);
    llvm::Value* rhs = inst->getOperand(1);
    
    llvm::Value* newResult = createComplexAdd(builder, lhs, rhs);
    
    inst->replaceAllUsesWith(newResult);
    substitutionCount_++;
    
    return true;
}

bool InstructionSubstitution::substituteSub(llvm::BinaryOperator* inst) {
    llvm::IRBuilder<> builder(inst);
    
    // Replace: a - b
    // With: (a + (-b))
    llvm::Value* lhs = inst->getOperand(0);
    llvm::Value* rhs = inst->getOperand(1);
    
    llvm::Value* newResult = createComplexSub(builder, lhs, rhs);
    
    inst->replaceAllUsesWith(newResult);
    substitutionCount_++;
    
    return true;
}

bool InstructionSubstitution::substituteXor(llvm::BinaryOperator* inst) {
    llvm::IRBuilder<> builder(inst);
    
    // Replace: a ^ b
    // With: (a | b) & ~(a & b)
    llvm::Value* lhs = inst->getOperand(0);
    llvm::Value* rhs = inst->getOperand(1);
    
    llvm::Value* newResult = createComplexXor(builder, lhs, rhs);
    
    inst->replaceAllUsesWith(newResult);
    substitutionCount_++;
    
    return true;
}

bool InstructionSubstitution::substituteAnd(llvm::BinaryOperator* inst) {
    llvm::IRBuilder<> builder(inst);
    
    // Replace: a & b
    // With: ~(~a | ~b)
    llvm::Value* lhs = inst->getOperand(0);
    llvm::Value* rhs = inst->getOperand(1);
    
    llvm::Value* notLhs = builder.CreateNot(lhs, "not_lhs");
    llvm::Value* notRhs = builder.CreateNot(rhs, "not_rhs");
    llvm::Value* orVal = builder.CreateOr(notLhs, notRhs, "or_val");
    llvm::Value* result = builder.CreateNot(orVal, "and_result");
    
    inst->replaceAllUsesWith(result);
    substitutionCount_++;
    
    return true;
}

bool InstructionSubstitution::substituteOr(llvm::BinaryOperator* inst) {
    llvm::IRBuilder<> builder(inst);
    
    // Replace: a | b
    // With: ~(~a & ~b)
    llvm::Value* lhs = inst->getOperand(0);
    llvm::Value* rhs = inst->getOperand(1);
    
    llvm::Value* notLhs = builder.CreateNot(lhs, "not_lhs");
    llvm::Value* notRhs = builder.CreateNot(rhs, "not_rhs");
    llvm::Value* andVal = builder.CreateAnd(notLhs, notRhs, "and_val");
    llvm::Value* result = builder.CreateNot(andVal, "or_result");
    
    inst->replaceAllUsesWith(result);
    substitutionCount_++;
    
    return true;
}

llvm::Value* InstructionSubstitution::createComplexAdd(llvm::IRBuilder<>& builder,
                                                       llvm::Value* lhs, llvm::Value* rhs) {
    // a + b = (a - (-b)) = (a ^ b) + 2 * (a & b)
    llvm::Value* xorVal = builder.CreateXor(lhs, rhs, "xor_val");
    llvm::Value* andVal = builder.CreateAnd(lhs, rhs, "and_val");
    llvm::Value* shifted = builder.CreateShl(andVal, 
        llvm::ConstantInt::get(andVal->getType(), 1), "shifted");
    llvm::Value* result = builder.CreateAdd(xorVal, shifted, "add_result");
    
    return result;
}

llvm::Value* InstructionSubstitution::createComplexSub(llvm::IRBuilder<>& builder,
                                                       llvm::Value* lhs, llvm::Value* rhs) {
    // a - b = a + (~b + 1)
    llvm::Value* notRhs = builder.CreateNot(rhs, "not_rhs");
    llvm::Value* plusOne = builder.CreateAdd(notRhs, 
        llvm::ConstantInt::get(notRhs->getType(), 1), "plus_one");
    llvm::Value* result = builder.CreateAdd(lhs, plusOne, "sub_result");
    
    return result;
}

llvm::Value* InstructionSubstitution::createComplexXor(llvm::IRBuilder<>& builder,
                                                       llvm::Value* lhs, llvm::Value* rhs) {
    // a ^ b = (a | b) & ~(a & b)
    llvm::Value* orVal = builder.CreateOr(lhs, rhs, "or_val");
    llvm::Value* andVal = builder.CreateAnd(lhs, rhs, "and_val");
    llvm::Value* notAnd = builder.CreateNot(andVal, "not_and");
    llvm::Value* result = builder.CreateAnd(orVal, notAnd, "xor_result");
    
    return result;
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::InstructionSubstitution> X(
    "instruction-substitution", "Instruction Substitution Pass", false, false);
