#include "jit/StubGenerator.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"

namespace obfuscator {

StubGenerator::StubGenerator() = default;
StubGenerator::~StubGenerator() = default;

llvm::Function* StubGenerator::generateStub(llvm::Module& module, llvm::Function* original) {
    Logger::info("Generating stub for: " + original->getName().str());
    
    llvm::LLVMContext& ctx = module.getContext();
    
    // Create stub function
    llvm::Function* stub = llvm::Function::Create(
        original->getFunctionType(),
        llvm::Function::InternalLinkage,
        original->getName() + "_stub",
        module
    );
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", stub);
    llvm::IRBuilder<> builder(entry);
    
    // Create prologue
    createPrologue(stub, builder);
    
    // Call original function
    std::vector<llvm::Value*> args;
    for (auto& arg : stub->args()) {
        args.push_back(&arg);
    }
    
    llvm::Value* result = builder.CreateCall(original, args);
    
    // Create epilogue
    createEpilogue(stub, builder);
    
    // Return
    if (stub->getReturnType()->isVoidTy()) {
        builder.CreateRetVoid();
    } else {
        builder.CreateRet(result);
    }
    
    return stub;
}

void StubGenerator::createPrologue(llvm::Function* stub, llvm::IRBuilder<>& builder) {
    // Add prologue code (anti-debugging, checksums, etc.)
    llvm::LLVMContext& ctx = stub->getContext();
    
    // Example: create a dummy operation
    llvm::Value* dummy = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx));
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), dummy);
}

void StubGenerator::createEpilogue(llvm::Function* stub, llvm::IRBuilder<>& builder) {
    // Add epilogue code
    // Could include cleanup, verification, etc.
}

} // namespace obfuscator
