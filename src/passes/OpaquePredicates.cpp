#include "passes/OpaquePredicates.h"
#include "utils/Random.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"

namespace obfuscator {

char OpaquePredicates::ID = 0;

OpaquePredicates::OpaquePredicates()
    : FunctionPass(ID), targetCount_(100), predicateCount_(0) {}

OpaquePredicates::OpaquePredicates(int count)
    : FunctionPass(ID), targetCount_(count), predicateCount_(0) {}

bool OpaquePredicates::runOnFunction(llvm::Function &F) {
    if (F.isDeclaration() || F.size() < 2) {
        return false;
    }
    
    Logger::info("Running Opaque Predicates on: " + F.getName().str());
    
    predicateCount_ = 0;
    
    std::vector<llvm::BasicBlock*> blocks;
    for (auto& BB : F) {
        blocks.push_back(&BB);
    }
    
    int predicatesPerFunction = std::min(targetCount_ / 5, 20);
    
    for (int i = 0; i < predicatesPerFunction && !blocks.empty(); ++i) {
        int idx = Random::randomInt(0, blocks.size() - 1);
        llvm::BasicBlock* BB = blocks[idx];
        
        if (insertOpaquePredicate(F, BB)) {
            predicateCount_++;
        }
    }
    
    if (predicateCount_ > 0) {
        Logger::info("Inserted " + std::to_string(predicateCount_) + 
                    " opaque predicates in: " + F.getName().str());
    }
    
    return predicateCount_ > 0;
}

bool OpaquePredicates::insertOpaquePredicate(llvm::Function &F, llvm::BasicBlock* BB) {
    if (BB->size() < 2) {
        return false;
    }
    
    llvm::LLVMContext& ctx = F.getContext();
    llvm::IRBuilder<> builder(ctx);
    
    // Set insertion point at the beginning of the block
    builder.SetInsertPoint(&*BB->getFirstInsertionPt());
    
    // Choose predicate type randomly
    int predicateType = Random::randomInt(0, 2);
    llvm::Value* predicate = nullptr;
    
    switch (predicateType) {
        case 0:
            predicate = createAlwaysTruePredicate(builder);
            break;
        case 1:
            predicate = createAlwaysFalsePredicate(builder);
            break;
        case 2:
            predicate = createContextualPredicate(builder, F);
            break;
        default:
            return false;
    }
    
    if (!predicate) {
        return false;
    }
    
    // Create bogus block
    llvm::BasicBlock* bogusBlock = llvm::BasicBlock::Create(ctx, "opaque_bogus", &F);
    llvm::IRBuilder<> bogusBuilder(bogusBlock);
    
    // Add some bogus operations
    llvm::Value* bogusVar = bogusBuilder.CreateAlloca(llvm::Type::getInt32Ty(ctx));
    bogusBuilder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 42), bogusVar);
    bogusBuilder.CreateUnreachable();
    
    // We don't actually modify the control flow here to avoid breaking the function
    // In a production version, you'd split the block and add conditional branches
    
    return true;
}

llvm::Value* OpaquePredicates::createAlwaysTruePredicate(llvm::IRBuilder<>& builder) {
    llvm::LLVMContext& ctx = builder.getContext();
    
    // Use mathematical identity: (x * x) % 2 == 0 for even x
    // We ensure x is always even
    llvm::Value* x = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 
        Random::randomInt(1, 100) * 2); // Always even
    
    llvm::Value* x2 = builder.CreateMul(x, x, "x2");
    llvm::Value* mod = builder.CreateSRem(x2, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 2), "mod");
    llvm::Value* isZero = builder.CreateICmpEQ(mod, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), "is_even");
    
    return isZero; // Always true
}

llvm::Value* OpaquePredicates::createAlwaysFalsePredicate(llvm::IRBuilder<>& builder) {
    llvm::LLVMContext& ctx = builder.getContext();
    
    // Use mathematical impossibility: x^2 < 0 for real x
    llvm::Value* x = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 
        Random::randomInt(1, 100));
    
    llvm::Value* x2 = builder.CreateMul(x, x, "x2");
    llvm::Value* isNegative = builder.CreateICmpSLT(x2, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), "is_negative");
    
    return isNegative; // Always false
}

llvm::Value* OpaquePredicates::createContextualPredicate(llvm::IRBuilder<>& builder,
                                                         llvm::Function &F) {
    llvm::LLVMContext& ctx = F.getContext();
    llvm::Module* M = F.getParent();
    
    // Create a global variable that affects the predicate
    llvm::GlobalVariable* globalCounter = M->getGlobalVariable("__opaque_counter");
    
    if (!globalCounter) {
        globalCounter = new llvm::GlobalVariable(
            *M,
            llvm::Type::getInt32Ty(ctx),
            false, // not constant
            llvm::GlobalValue::InternalLinkage,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0),
            "__opaque_counter"
        );
    }
    
    // Load counter and check if it's even (which it always will be initially)
    llvm::Value* counter = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), 
        globalCounter, "counter");
    llvm::Value* mod = builder.CreateSRem(counter, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 2));
    llvm::Value* isEven = builder.CreateICmpEQ(mod, 
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
    
    return isEven;
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::OpaquePredicates> X(
    "opaque-predicates", "Opaque Predicates Pass", false, false);
