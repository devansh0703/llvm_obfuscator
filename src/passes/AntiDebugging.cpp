#include "passes/AntiDebugging.h"
#include "utils/Logger.h"
#include "utils/Random.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"

namespace obfuscator {

char AntiDebugging::ID = 0;

AntiDebugging::AntiDebugging()
    : ModulePass(ID), checkCount_(0) {}

bool AntiDebugging::runOnModule(llvm::Module &M) {
    Logger::info("Running Anti-Debugging on module: " + M.getName().str());
    
    checkCount_ = 0;
    
    // Create debugger check function
    llvm::Function* checkFunc = createDebuggerCheckFunction(M);
    
    // Create timing check function
    llvm::Function* timingFunc = createTimingCheckFunction(M);
    
    // Insert checks into functions
    bool modified = insertAntiDebugChecks(M, checkFunc);
    
    if (modified) {
        Logger::info("Inserted " + std::to_string(checkCount_) + " anti-debug checks");
    }
    
    return modified;
}

llvm::Function* AntiDebugging::createDebuggerCheckFunction(llvm::Module &M) {
    llvm::LLVMContext& ctx = M.getContext();
    
    // Create function: i1 __anti_debug_check()
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt1Ty(ctx), false);
    
    llvm::Function* checkFunc = llvm::Function::Create(
        funcType,
        llvm::Function::InternalLinkage,
        "__anti_debug_check",
        M
    );
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", checkFunc);
    llvm::BasicBlock* checkPtrace = llvm::BasicBlock::Create(ctx, "check_ptrace", checkFunc);
    llvm::BasicBlock* detected = llvm::BasicBlock::Create(ctx, "detected", checkFunc);
    llvm::BasicBlock* notDetected = llvm::BasicBlock::Create(ctx, "not_detected", checkFunc);
    
    llvm::IRBuilder<> builder(entry);
    
    // Check for debugger presence using various methods
    // Method 1: Check if ptrace is attached (Linux)
    if (M.getTargetTriple().find("linux") != std::string::npos) {
        // In a real implementation, we would use inline assembly or call ptrace
        // For now, we'll create a placeholder
        builder.CreateBr(checkPtrace);
        
        builder.SetInsertPoint(checkPtrace);
        
        // Simulate ptrace check
        llvm::Value* ptraceResult = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0);
        
        llvm::Value* isDebugged = builder.CreateICmpNE(
            ptraceResult,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0),
            "is_debugged"
        );
        
        builder.CreateCondBr(isDebugged, detected, notDetected);
    } else {
        // Windows or other platforms
        builder.CreateBr(notDetected);
    }
    
    // Detected branch
    builder.SetInsertPoint(detected);
    builder.CreateRet(llvm::ConstantInt::getTrue(ctx));
    
    // Not detected branch
    builder.SetInsertPoint(notDetected);
    builder.CreateRet(llvm::ConstantInt::getFalse(ctx));
    
    return checkFunc;
}

llvm::Function* AntiDebugging::createTimingCheckFunction(llvm::Module &M) {
    llvm::LLVMContext& ctx = M.getContext();
    
    // Create function: i1 __timing_check()
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt1Ty(ctx), false);
    
    llvm::Function* timingFunc = llvm::Function::Create(
        funcType,
        llvm::Function::InternalLinkage,
        "__timing_check",
        M
    );
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", timingFunc);
    llvm::IRBuilder<> builder(entry);
    
    // Timing-based detection using RDTSC (x86) or similar
    // This would measure time and detect if execution is too slow (debugger present)
    
    // For now, return false (not detected)
    builder.CreateRet(llvm::ConstantInt::getFalse(ctx));
    
    return timingFunc;
}

bool AntiDebugging::insertAntiDebugChecks(llvm::Module &M, llvm::Function* checkFunc) {
    std::vector<llvm::Function*> functions;
    
    for (auto& F : M) {
        if (!F.isDeclaration() && &F != checkFunc) {
            functions.push_back(&F);
        }
    }
    
    if (functions.empty()) {
        return false;
    }
    
    llvm::LLVMContext& ctx = M.getContext();
    
    // Insert checks in random functions
    for (auto* F : functions) {
        if (Random::probability() > 0.7f) {
            continue; // Skip some functions
        }
        
        // Insert at function entry
        llvm::BasicBlock& entry = F->getEntryBlock();
        llvm::IRBuilder<> builder(&*entry.getFirstInsertionPt());
        
        // Call debugger check
        llvm::Value* isDebugged = builder.CreateCall(checkFunc, {}, "dbg_check");
        
        // Create exit block for when debugger is detected
        llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(
            ctx, "anti_debug_exit", F);
        llvm::IRBuilder<> exitBuilder(exitBlock);
        
        if (F->getReturnType()->isVoidTy()) {
            exitBuilder.CreateRetVoid();
        } else {
            exitBuilder.CreateRet(llvm::Constant::getNullValue(F->getReturnType()));
        }
        
        // Split block and insert conditional branch
        llvm::BasicBlock* continueBlock = entry.splitBasicBlock(
            builder.GetInsertPoint(), "continue");
        
        entry.getTerminator()->eraseFromParent();
        llvm::IRBuilder<> branchBuilder(&entry);
        branchBuilder.CreateCondBr(isDebugged, exitBlock, continueBlock);
        
        checkCount_++;
    }
    
    return checkCount_ > 0;
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::AntiDebugging> X(
    "anti-debugging", "Anti-Debugging Pass", false, false);
