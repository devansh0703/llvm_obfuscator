#include "passes/FakeLoopGeneration.h"
#include "utils/Random.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"

namespace obfuscator {

char FakeLoopGeneration::ID = 0;

FakeLoopGeneration::FakeLoopGeneration()
    : FunctionPass(ID), targetCount_(50), generatedCount_(0) {}

FakeLoopGeneration::FakeLoopGeneration(int count)
    : FunctionPass(ID), targetCount_(count), generatedCount_(0) {}

bool FakeLoopGeneration::runOnFunction(llvm::Function &F) {
    if (F.isDeclaration()) {
        return false;
    }
    
    Logger::info("Running Fake Loop Generation on: " + F.getName().str());
    
    generatedCount_ = 0;
    
    std::vector<llvm::BasicBlock*> blocks;
    for (auto& BB : F) {
        blocks.push_back(&BB);
    }
    
    if (blocks.empty()) {
        return false;
    }
    
    int loopsPerFunction = std::min(targetCount_ / 10, 10);
    
    for (int i = 0; i < loopsPerFunction && !blocks.empty(); ++i) {
        int idx = Random::randomInt(0, blocks.size() - 1);
        llvm::BasicBlock* insertPoint = blocks[idx];
        
        if (insertFakeLoop(F, insertPoint)) {
            generatedCount_++;
        }
    }
    
    if (generatedCount_ > 0) {
        Logger::info("Generated " + std::to_string(generatedCount_) + 
                    " fake loops in: " + F.getName().str());
    }
    
    return generatedCount_ > 0;
}

bool FakeLoopGeneration::insertFakeLoop(llvm::Function &F, llvm::BasicBlock* insertPoint) {
    llvm::LLVMContext& ctx = F.getContext();
    llvm::IRBuilder<> builder(ctx);
    
    // Create basic blocks for fake loop
    llvm::BasicBlock* loopHeader = llvm::BasicBlock::Create(ctx, "fake_loop_header", &F);
    llvm::BasicBlock* loopBody = llvm::BasicBlock::Create(ctx, "fake_loop_body", &F);
    llvm::BasicBlock* loopExit = llvm::BasicBlock::Create(ctx, "fake_loop_exit", &F);
    
    // Set insertion point at the beginning of insert point block
    builder.SetInsertPoint(&*insertPoint->getFirstInsertionPt());
    
    // Create loop counter
    llvm::AllocaInst* counter = builder.CreateAlloca(
        llvm::Type::getInt32Ty(ctx), nullptr, "fake_counter");
    
    // Initialize counter with a value that makes the loop condition always false
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 100), counter);
    
    // Branch to loop header
    // We'll create the fake loop but make it unreachable via opaque predicate
    llvm::Value* neverTrue = builder.CreateICmpEQ(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1),
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0),
        "never_true"
    );
    
    builder.CreateCondBr(neverTrue, loopHeader, insertPoint);
    
    // Loop header: check condition
    builder.SetInsertPoint(loopHeader);
    llvm::Value* i = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), counter, "i");
    llvm::Value* cond = builder.CreateICmpSLT(
        i, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 10), "cond");
    builder.CreateCondBr(cond, loopBody, loopExit);
    
    // Loop body: some fake operations
    builder.SetInsertPoint(loopBody);
    createFakeLoopBody(loopBody, builder);
    
    // Increment counter
    llvm::Value* nextI = builder.CreateAdd(i, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));
    builder.CreateStore(nextI, counter);
    builder.CreateBr(loopHeader);
    
    // Loop exit: just makes the CFG valid
    builder.SetInsertPoint(loopExit);
    builder.CreateUnreachable();
    
    return true;
}

void FakeLoopGeneration::createFakeLoopBody(llvm::BasicBlock* loopBody,
                                           llvm::IRBuilder<>& builder) {
    llvm::LLVMContext& ctx = loopBody->getContext();
    
    // Create some fake computation
    llvm::Value* temp1 = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "temp1");
    llvm::Value* temp2 = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "temp2");
    
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 
        Random::randomInt(1, 100)), temp1);
    
    llvm::Value* val1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), temp1);
    llvm::Value* result = builder.CreateMul(val1, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(2, 5)));
    result = builder.CreateAdd(result, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(10, 50)));
    
    builder.CreateStore(result, temp2);
    
    // Add more fake operations
    llvm::Value* val2 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), temp2);
    llvm::Value* shifted = builder.CreateShl(val2, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 2));
    builder.CreateStore(shifted, temp1);
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::FakeLoopGeneration> X(
    "fake-loop-generation", "Fake Loop Generation Pass", false, false);
