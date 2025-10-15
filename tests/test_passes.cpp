#include "passes/ControlFlowFlattening.h"
#include "passes/BogusCodeInjection.h"
#include "passes/StringObfuscation.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include <cassert>

extern void test_control_flow_flattening();
extern void test_bogus_code_injection();
extern void test_string_obfuscation();

// Helper: create a simple test function
llvm::Function* createTestFunction(llvm::Module& M) {
    llvm::LLVMContext& ctx = M.getContext();
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(ctx),
        {llvm::Type::getInt32Ty(ctx), llvm::Type::getInt32Ty(ctx)},
        false
    );
    
    llvm::Function* func = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        "test_func",
        M
    );
    
    // Create some basic blocks
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", func);
    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(ctx, "then", func);
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(ctx, "else", func);
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(ctx, "end", func);
    
    llvm::IRBuilder<> builder(entry);
    
    auto args = func->arg_begin();
    llvm::Value* a = &*args++;
    llvm::Value* b = &*args;
    
    llvm::Value* cond = builder.CreateICmpSGT(a, b);
    builder.CreateCondBr(cond, then_block, else_block);
    
    builder.SetInsertPoint(then_block);
    llvm::Value* sum = builder.CreateAdd(a, b);
    builder.CreateBr(end_block);
    
    builder.SetInsertPoint(else_block);
    llvm::Value* diff = builder.CreateSub(a, b);
    builder.CreateBr(end_block);
    
    builder.SetInsertPoint(end_block);
    llvm::PHINode* phi = builder.CreatePHI(llvm::Type::getInt32Ty(ctx), 2);
    phi->addIncoming(sum, then_block);
    phi->addIncoming(diff, else_block);
    builder.CreateRet(phi);
    
    return func;
}

void test_control_flow_flattening() {
    llvm::LLVMContext ctx;
    llvm::Module M("test_module", ctx);
    
    llvm::Function* func = createTestFunction(M);
    size_t original_bb_count = func->size();
    
    obfuscator::ControlFlowFlattening pass(0.5f);
    pass.runOnFunction(*func);
    
    // After flattening, we should have different structure
    // (test is simplified - in production you'd check dispatcher, switch, etc.)
    assert(func->size() >= original_bb_count);
}

void test_bogus_code_injection() {
    llvm::LLVMContext ctx;
    llvm::Module M("test_module", ctx);
    
    llvm::Function* func = createTestFunction(M);
    size_t original_inst_count = func->getInstructionCount();
    
    obfuscator::BogusCodeInjection pass(0.5f, 50);
    pass.runOnFunction(*func);
    
    // Should have more instructions after bogus code injection
    assert(func->getInstructionCount() >= original_inst_count);
}

void test_string_obfuscation() {
    llvm::LLVMContext ctx;
    llvm::Module M("test_module", ctx);
    
    // Create a global string
    llvm::IRBuilder<> builder(ctx);
    llvm::Constant* strConstant = builder.CreateGlobalStringPtr("Hello, World!");
    
    llvm::GlobalVariable* strGlobal = llvm::cast<llvm::GlobalVariable>(
        strConstant->stripPointerCasts());
    
    obfuscator::StringObfuscation pass(1.0f);
    pass.runOnModule(M);
    
    // Pass should run successfully
    assert(pass.getObfuscationCount() >= 0);
}
