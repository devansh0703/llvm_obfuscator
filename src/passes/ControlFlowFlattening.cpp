#include "passes/ControlFlowFlattening.h"
#include "utils/Random.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>
#include <algorithm>

namespace obfuscator {

char ControlFlowFlattening::ID = 0;

ControlFlowFlattening::ControlFlowFlattening()
    : FunctionPass(ID), intensity_(0.7f), modificationCount_(0) {}

ControlFlowFlattening::ControlFlowFlattening(float intensity)
    : FunctionPass(ID), intensity_(intensity), modificationCount_(0) {}

bool ControlFlowFlattening::runOnFunction(llvm::Function &F) {
    if (F.isDeclaration() || F.size() < 2) {
        return false;
    }
    
    Logger::info("Running Control Flow Flattening on: " + F.getName().str());
    
    modificationCount_ = 0;
    bool modified = false;
    
    // Decide whether to flatten based on intensity
    if (Random::probability() > intensity_) {
        return false;
    }
    
    modified = flattenFunction(F);
    
    if (modified) {
        Logger::info("Control flow flattened: " + F.getName().str() + 
                    " (" + std::to_string(modificationCount_) + " modifications)");
    }
    
    return modified;
}

bool ControlFlowFlattening::flattenFunction(llvm::Function &F) {
    // Collect all basic blocks except entry
    std::vector<llvm::BasicBlock*> blocks;
    llvm::BasicBlock* entry = &F.getEntryBlock();
    
    for (auto& BB : F) {
        if (&BB != entry && !BB.isLandingPad()) {
            blocks.push_back(&BB);
        }
    }
    
    if (blocks.empty()) {
        return false;
    }
    
    // Create new entry block with switch variable
    llvm::LLVMContext& ctx = F.getContext();
    llvm::BasicBlock* newEntry = llvm::BasicBlock::Create(ctx, "flattened_entry", &F, entry);
    
    // Create switch variable
    llvm::IRBuilder<> builder(newEntry);
    llvm::AllocaInst* switchVar = builder.CreateAlloca(
        llvm::Type::getInt32Ty(ctx), nullptr, "switch_var");
    
    // Initialize with first block's case value
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), switchVar);
    
    // Create dispatcher block
    llvm::BasicBlock* dispatcher = createDispatcher(F, blocks);
    builder.CreateBr(dispatcher);
    
    // Assign case values to blocks
    for (size_t i = 0; i < blocks.size(); ++i) {
        int caseValue = static_cast<int>(i);
        convertToSwitchCase(blocks[i], switchVar, caseValue);
        modificationCount_++;
    }
    
    // Update original entry to jump to new entry
    llvm::Instruction* terminator = entry->getTerminator();
    if (terminator) {
        terminator->eraseFromParent();
    }
    llvm::IRBuilder<> entryBuilder(entry);
    entryBuilder.CreateBr(newEntry);
    
    return true;
}

llvm::BasicBlock* ControlFlowFlattening::createDispatcher(llvm::Function &F,
                                                          std::vector<llvm::BasicBlock*>& blocks) {
    llvm::LLVMContext& ctx = F.getContext();
    llvm::BasicBlock* dispatcher = llvm::BasicBlock::Create(ctx, "dispatcher", &F);
    
    llvm::IRBuilder<> builder(dispatcher);
    
    // Load switch variable
    llvm::AllocaInst* switchVar = nullptr;
    for (auto& inst : F.getEntryBlock()) {
        if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(&inst)) {
            if (alloca->getName() == "switch_var") {
                switchVar = alloca;
                break;
            }
        }
    }
    
    if (!switchVar) {
        switchVar = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "switch_var");
    }
    
    llvm::Value* switchValue = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), switchVar, "sw");
    
    // Create default case (return or unreachable)
    llvm::BasicBlock* defaultCase = llvm::BasicBlock::Create(ctx, "default", &F);
    llvm::IRBuilder<> defaultBuilder(defaultCase);
    
    if (F.getReturnType()->isVoidTy()) {
        defaultBuilder.CreateRetVoid();
    } else {
        defaultBuilder.CreateUnreachable();
    }
    
    // Create switch instruction
    llvm::SwitchInst* switchInst = builder.CreateSwitch(switchValue, defaultCase, blocks.size());
    
    // Add cases for each block
    for (size_t i = 0; i < blocks.size(); ++i) {
        switchInst->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), i), blocks[i]);
    }
    
    return dispatcher;
}

void ControlFlowFlattening::convertToSwitchCase(llvm::BasicBlock* BB,
                                                llvm::Value* switchVar,
                                                int caseValue) {
    llvm::LLVMContext& ctx = BB->getContext();
    llvm::Function* F = BB->getParent();
    
    // Find dispatcher
    llvm::BasicBlock* dispatcher = nullptr;
    for (auto& block : *F) {
        if (block.getName() == "dispatcher") {
            dispatcher = &block;
            break;
        }
    }
    
    if (!dispatcher) {
        return;
    }
    
    // Modify terminator to update switch variable and jump to dispatcher
    llvm::Instruction* terminator = BB->getTerminator();
    if (!terminator) {
        return;
    }
    
    llvm::IRBuilder<> builder(terminator);
    
    // If it's a branch, update switch variable accordingly
    if (auto* br = llvm::dyn_cast<llvm::BranchInst>(terminator)) {
        if (br->isUnconditional()) {
            llvm::BasicBlock* successor = br->getSuccessor(0);
            
            // Find case value for successor
            int nextCase = -1;
            size_t idx = 0;
            for (auto& block : *F) {
                if (&block == successor) {
                    nextCase = static_cast<int>(idx);
                    break;
                }
                if (!block.isLandingPad() && &block != &F->getEntryBlock()) {
                    idx++;
                }
            }
            
            if (nextCase >= 0) {
                builder.CreateStore(
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), nextCase),
                    llvm::cast<llvm::AllocaInst>(switchVar));
            }
            
            br->eraseFromParent();
            builder.CreateBr(dispatcher);
        } else {
            // Conditional branch - use select to compute next case
            llvm::BasicBlock* trueBlock = br->getSuccessor(0);
            llvm::BasicBlock* falseBlock = br->getSuccessor(1);
            llvm::Value* condition = br->getCondition();
            
            // Find case values
            int trueCase = Random::randomInt(0, 1000);
            int falseCase = Random::randomInt(0, 1000);
            
            llvm::Value* nextCase = builder.CreateSelect(
                condition,
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), trueCase),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), falseCase));
            
            builder.CreateStore(nextCase, llvm::cast<llvm::AllocaInst>(switchVar));
            br->eraseFromParent();
            builder.CreateBr(dispatcher);
        }
    }
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::ControlFlowFlattening> X(
    "control-flow-flattening", "Control Flow Flattening Pass", false, false);
