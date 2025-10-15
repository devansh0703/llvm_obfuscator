#include "passes/CodeWatermarking.h"
#include "utils/Logger.h"
#include "utils/Crypto.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include <vector>

namespace obfuscator {

char CodeWatermarking::ID = 0;

CodeWatermarking::CodeWatermarking()
    : ModulePass(ID), payload_(""), watermarkCount_(0) {}

CodeWatermarking::CodeWatermarking(const std::string& payload)
    : ModulePass(ID), payload_(payload), watermarkCount_(0) {}

bool CodeWatermarking::runOnModule(llvm::Module &M) {
    if (payload_.empty()) {
        Logger::warning("No watermark payload specified");
        return false;
    }
    
    Logger::info("Running Code Watermarking on module: " + M.getName().str());
    
    watermarkCount_ = 0;
    
    bool modified = embedWatermark(M);
    
    if (modified) {
        Logger::info("Embedded " + std::to_string(watermarkCount_) + " watermarks");
    }
    
    return modified;
}

bool CodeWatermarking::embedWatermark(llvm::Module &M) {
    // Create watermark data
    llvm::GlobalVariable* watermarkGV = createWatermarkData(M);
    
    if (watermarkGV) {
        watermarkCount_++;
    }
    
    // Embed structural watermark
    if (embedStructuralWatermark(M)) {
        watermarkCount_++;
    }
    
    return watermarkCount_ > 0;
}

llvm::GlobalVariable* CodeWatermarking::createWatermarkData(llvm::Module &M) {
    llvm::LLVMContext& ctx = M.getContext();
    
    // Encrypt the watermark payload
    std::vector<uint8_t> payloadBytes(payload_.begin(), payload_.end());
    uint32_t key = Crypto::generateKey();
    std::vector<uint8_t> encrypted = Crypto::xorEncrypt(payloadBytes, key);
    
    // Create constant array
    std::vector<llvm::Constant*> constants;
    for (uint8_t byte : encrypted) {
        constants.push_back(llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx), byte));
    }
    
    llvm::ArrayType* arrayType = llvm::ArrayType::get(
        llvm::Type::getInt8Ty(ctx), encrypted.size());
    llvm::Constant* watermarkData = llvm::ConstantArray::get(arrayType, constants);
    
    // Create global variable
    llvm::GlobalVariable* watermarkGV = new llvm::GlobalVariable(
        M,
        arrayType,
        true,  // isConstant
        llvm::GlobalValue::PrivateLinkage,
        watermarkData,
        "__watermark_" + std::to_string(key)
    );
    
    // Make it harder to find
    watermarkGV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    watermarkGV->setSection(".rodata");
    
    Logger::info("Created watermark data: " + std::to_string(encrypted.size()) + " bytes");
    
    return watermarkGV;
}

bool CodeWatermarking::embedStructuralWatermark(llvm::Module &M) {
    llvm::LLVMContext& ctx = M.getContext();
    
    // Embed watermark in code structure using specific patterns
    // Convert payload to binary and encode in basic block graph
    
    std::vector<uint8_t> payloadBytes(payload_.begin(), payload_.end());
    uint32_t hash = Crypto::hash32(payloadBytes);
    
    // Create a function that encodes the watermark hash in its structure
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(ctx), false);
    
    llvm::Function* watermarkFunc = llvm::Function::Create(
        funcType,
        llvm::Function::InternalLinkage,
        "__watermark_func_" + std::to_string(hash),
        M
    );
    
    // Create basic blocks that encode the watermark
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", watermarkFunc);
    llvm::IRBuilder<> builder(entry);
    
    // Encode hash bits in the control flow
    llvm::Value* var = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx));
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), hash), var);
    
    // Create a series of operations that encode the watermark
    for (int i = 0; i < 32; ++i) {
        if (hash & (1 << i)) {
            llvm::Value* val = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var);
            llvm::Value* shifted = builder.CreateShl(val, 
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));
            builder.CreateStore(shifted, var);
        } else {
            llvm::Value* val = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), var);
            llvm::Value* shifted = builder.CreateLShr(val, 
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));
            builder.CreateStore(shifted, var);
        }
    }
    
    builder.CreateRetVoid();
    
    Logger::info("Created structural watermark function");
    
    return true;
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::CodeWatermarking> X(
    "code-watermarking", "Code Watermarking Pass", false, false);
