#include "passes/BogusCodeInjection.h"
#include "utils/Random.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>

namespace obfuscator {

char BogusCodeInjection::ID = 0;

BogusCodeInjection::BogusCodeInjection()
    : FunctionPass(ID), intensity_(0.5f), maxPercent_(30), insertionCount_(0) {}

BogusCodeInjection::BogusCodeInjection(float intensity, int maxPercent)
    : FunctionPass(ID), intensity_(intensity), maxPercent_(maxPercent), insertionCount_(0) {}

bool BogusCodeInjection::runOnFunction(llvm::Function &F) {
    if (F.isDeclaration()) {
        return false;
    }
    
    Logger::info("Running Bogus Code Injection on: " + F.getName().str());
    
    insertionCount_ = 0;
    bool modified = insertBogusCode(F);
    
    if (modified) {
        Logger::info("Injected " + std::to_string(insertionCount_) + 
                    " bogus code blocks in: " + F.getName().str());
    }
    
    return modified;
}

bool BogusCodeInjection::insertBogusCode(llvm::Function &F) {
    llvm::LLVMContext& ctx = F.getContext();
    
    // Calculate how many bogus blocks to insert
    size_t originalBlockCount = F.size();
    int maxBogusBlocks = (originalBlockCount * maxPercent_) / 100;
    int targetBogusBlocks = static_cast<int>(maxBogusBlocks * intensity_);
    
    if (targetBogusBlocks == 0) {
        targetBogusBlocks = 1;
    }
    
    std::vector<llvm::BasicBlock*> originalBlocks;
    for (auto& BB : F) {
        originalBlocks.push_back(&BB);
    }
    
    llvm::IRBuilder<> builder(ctx);
    
    for (int i = 0; i < targetBogusBlocks && !originalBlocks.empty(); ++i) {
        // Pick a random block to insert after
        int idx = Random::randomInt(0, originalBlocks.size() - 1);
        llvm::BasicBlock* targetBlock = originalBlocks[idx];
        
        // Create bogus block
        llvm::BasicBlock* bogusBlock = createBogusBlock(F, builder);
        
        if (bogusBlock) {
            // Insert after target block
            bogusBlock->moveAfter(targetBlock);
            
            // Create an opaque predicate (always false)
            builder.SetInsertPoint(targetBlock->getTerminator());
            
            // Create a complex always-false condition
            llvm::Value* x = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(1, 100));
            llvm::Value* y = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(1, 100));
            
            // (x * x) % 2 == (y * y) % 2 is statistically false
            llvm::Value* x2 = builder.CreateMul(x, x);
            llvm::Value* y2 = builder.CreateMul(y, y);
            llvm::Value* xMod = builder.CreateSRem(x2, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 2));
            llvm::Value* yMod = builder.CreateSRem(y2, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 2));
            
            // Add more complexity
            llvm::Value* sum = builder.CreateAdd(xMod, yMod);
            llvm::Value* bogusCondition = builder.CreateICmpEQ(
                sum, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 999)); // Always false
            
            // Split the block at terminator
            llvm::Instruction* terminator = targetBlock->getTerminator();
            if (terminator && !llvm::isa<llvm::BranchInst>(terminator)) {
                continue;
            }
            
            // For now, just increment counter
            insertionCount_++;
        }
    }
    
    return insertionCount_ > 0;
}

llvm::BasicBlock* BogusCodeInjection::createBogusBlock(llvm::Function &F,
                                                       llvm::IRBuilder<>& builder) {
    llvm::LLVMContext& ctx = F.getContext();
    llvm::BasicBlock* bogusBlock = llvm::BasicBlock::Create(ctx, "bogus", &F);
    
    builder.SetInsertPoint(bogusBlock);
    
    // Generate random bogus instructions
    int instructionCount = Random::randomInt(5, 20);
    generateBogusInstructions(bogusBlock, builder, instructionCount);
    
    // Add unreachable terminator
    builder.CreateUnreachable();
    
    return bogusBlock;
}

void BogusCodeInjection::generateBogusInstructions(llvm::BasicBlock* BB,
                                                   llvm::IRBuilder<>& builder,
                                                   int count) {
    llvm::LLVMContext& ctx = BB->getContext();
    builder.SetInsertPoint(BB);
    
    // Create some local variables
    llvm::Value* var1 = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "bogus_var1");
    llvm::Value* var2 = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "bogus_var2");
    llvm::Value* var3 = builder.CreateAlloca(llvm::Type::getInt64Ty(ctx), nullptr, "bogus_var3");
    
    // Initialize with random values
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(0, 1000)), var1);
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(0, 1000)), var2);
    
    for (int i = 0; i < count; ++i) {
        int opType = Random::randomInt(0, 7);
        
        switch (opType) {
            case 0: { // Addition
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* v2 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var2);
                llvm::Value* sum = builder.CreateAdd(v1, v2, "bogus_add");
                builder.CreateStore(sum, var1);
                break;
            }
            case 1: { // Multiplication
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* constant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), Random::randomInt(1, 10));
                llvm::Value* mul = builder.CreateMul(v1, constant, "bogus_mul");
                builder.CreateStore(mul, var2);
                break;
            }
            case 2: { // XOR
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* v2 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var2);
                llvm::Value* xorVal = builder.CreateXor(v1, v2, "bogus_xor");
                builder.CreateStore(xorVal, var1);
                break;
            }
            case 3: { // Shift
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* shifted = builder.CreateShl(v1, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1), "bogus_shl");
                builder.CreateStore(shifted, var1);
                break;
            }
            case 4: { // AND
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* constant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0xFF);
                llvm::Value* andVal = builder.CreateAnd(v1, constant, "bogus_and");
                builder.CreateStore(andVal, var2);
                break;
            }
            case 5: { // Type conversion
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* extended = builder.CreateZExt(v1, llvm::Type::getInt64Ty(ctx), "bogus_zext");
                builder.CreateStore(extended, var3);
                break;
            }
            case 6: { // Comparison
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* v2 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var2);
                builder.CreateICmpSGT(v1, v2, "bogus_cmp");
                break;
            }
            case 7: { // Subtraction
                llvm::Value* v1 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var1);
                llvm::Value* v2 = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var2);
                llvm::Value* diff = builder.CreateSub(v1, v2, "bogus_sub");
                builder.CreateStore(diff, var2);
                break;
            }
        }
    }
}

void BogusCodeInjection::createDeadCode(llvm::BasicBlock* BB,
                                       llvm::IRBuilder<>& builder) {
    llvm::LLVMContext& ctx = BB->getContext();
    
    // Create a fake function call
    llvm::FunctionType* fakeType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(ctx),
        {llvm::Type::getInt32Ty(ctx), llvm::Type::getInt32Ty(ctx)},
        false);
    
    // Create fake loop
    llvm::Value* counter = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "fake_counter");
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), counter);
    
    // Add some more realistic dead code patterns
    llvm::Value* tempArray = builder.CreateAlloca(
        llvm::ArrayType::get(llvm::Type::getInt32Ty(ctx), 10),
        nullptr, "fake_array");
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::BogusCodeInjection> X(
    "bogus-code-injection", "Bogus Code Injection Pass", false, false);
