#include "passes/StringObfuscation.h"
#include "utils/Random.h"
#include "utils/Crypto.h"
#include "utils/Logger.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include <vector>

namespace obfuscator {

char StringObfuscation::ID = 0;

StringObfuscation::StringObfuscation()
    : ModulePass(ID), intensity_(1.0f), obfuscationCount_(0) {}

StringObfuscation::StringObfuscation(float intensity)
    : ModulePass(ID), intensity_(intensity), obfuscationCount_(0) {}

bool StringObfuscation::runOnModule(llvm::Module &M) {
    Logger::info("Running String Obfuscation on module: " + M.getName().str());
    
    obfuscationCount_ = 0;
    
    // Find all string constants
    std::vector<llvm::GlobalVariable*> strings = findStrings(M);
    
    if (strings.empty()) {
        Logger::info("No strings found to obfuscate");
        return false;
    }
    
    // Create decryption function once
    llvm::Function* decryptFunc = createDecryptionFunction(M);
    
    // Obfuscate each string based on intensity
    for (auto* strVar : strings) {
        if (Random::probability() <= intensity_) {
            if (obfuscateString(M, strVar, decryptFunc)) {
                obfuscationCount_++;
            }
        }
    }
    
    Logger::info("Obfuscated " + std::to_string(obfuscationCount_) + " strings");
    
    return obfuscationCount_ > 0;
}

std::vector<llvm::GlobalVariable*> StringObfuscation::findStrings(llvm::Module &M) {
    std::vector<llvm::GlobalVariable*> strings;
    
    for (auto& GV : M.globals()) {
        if (GV.hasInitializer()) {
            if (auto* init = llvm::dyn_cast<llvm::ConstantDataArray>(GV.getInitializer())) {
                if (init->isString()) {
                    strings.push_back(&GV);
                }
            }
        }
    }
    
    return strings;
}

std::vector<uint8_t> StringObfuscation::encryptString(const std::string& str, uint32_t& key) {
    key = Random::randomInt(1, 0xFFFFFFFF);
    return Crypto::xorEncrypt(std::vector<uint8_t>(str.begin(), str.end()), key);
}

llvm::Function* StringObfuscation::createDecryptionFunction(llvm::Module &M) {
    llvm::LLVMContext& ctx = M.getContext();
    
    // Check if function already exists
    if (auto* existingFunc = M.getFunction("__obf_decrypt_string")) {
        return existingFunc;
    }
    
    // Create function: i8* __obf_decrypt_string(i8* encrypted, i32 length, i32 key)
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(ctx),
        {
            llvm::PointerType::getUnqual(ctx),  // encrypted data
            llvm::Type::getInt32Ty(ctx),        // length
            llvm::Type::getInt32Ty(ctx)         // key
        },
        false
    );
    
    llvm::Function* decryptFunc = llvm::Function::Create(
        funcType,
        llvm::Function::InternalLinkage,
        "__obf_decrypt_string",
        M
    );
    
    // Get arguments
    auto args = decryptFunc->arg_begin();
    llvm::Value* encrypted = &*args++;
    llvm::Value* length = &*args++;
    llvm::Value* key = &*args;
    
    encrypted->setName("encrypted");
    length->setName("length");
    key->setName("key");
    
    // Create basic blocks
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", decryptFunc);
    llvm::BasicBlock* loopHeader = llvm::BasicBlock::Create(ctx, "loop_header", decryptFunc);
    llvm::BasicBlock* loopBody = llvm::BasicBlock::Create(ctx, "loop_body", decryptFunc);
    llvm::BasicBlock* loopEnd = llvm::BasicBlock::Create(ctx, "loop_end", decryptFunc);
    llvm::BasicBlock* ret = llvm::BasicBlock::Create(ctx, "return", decryptFunc);
    
    llvm::IRBuilder<> builder(entry);
    
    // Allocate buffer for decrypted string
    llvm::Value* buffer = builder.CreateAlloca(
        llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 4096),
        nullptr,
        "buffer"
    );
    
    // Initialize loop counter
    llvm::Value* counter = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, "i");
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), counter);
    
    builder.CreateBr(loopHeader);
    
    // Loop header: check if i < length
    builder.SetInsertPoint(loopHeader);
    llvm::Value* i = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), counter, "i_val");
    llvm::Value* cond = builder.CreateICmpSLT(i, length, "cmp");
    builder.CreateCondBr(cond, loopBody, loopEnd);
    
    // Loop body: decrypt one byte
    builder.SetInsertPoint(loopBody);
    llvm::Value* iVal = builder.CreateLoad(llvm::Type::getInt32Ty(ctx), counter);
    
    // Load encrypted byte
    llvm::Value* encryptedPtr = builder.CreateGEP(llvm::Type::getInt8Ty(ctx), encrypted, iVal, "enc_ptr");
    llvm::Value* encryptedByte = builder.CreateLoad(llvm::Type::getInt8Ty(ctx), encryptedPtr, "enc_byte");
    
    // Compute key byte (key rotation)
    llvm::Value* keyByte = builder.CreateTrunc(
        builder.CreateLShr(key, builder.CreateAnd(iVal, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 3))),
        llvm::Type::getInt8Ty(ctx),
        "key_byte"
    );
    
    // XOR to decrypt
    llvm::Value* decryptedByte = builder.CreateXor(encryptedByte, keyByte, "dec_byte");
    
    // Store in buffer
    llvm::Value* bufferPtr = builder.CreateGEP(
        llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 4096),
        buffer,
        {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), iVal},
        "buf_ptr"
    );
    builder.CreateStore(decryptedByte, bufferPtr);
    
    // Increment counter
    llvm::Value* nextI = builder.CreateAdd(iVal, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));
    builder.CreateStore(nextI, counter);
    builder.CreateBr(loopHeader);
    
    // Loop end: null-terminate and return
    builder.SetInsertPoint(loopEnd);
    llvm::Value* nullPtr = builder.CreateGEP(
        llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 4096),
        buffer,
        {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), length},
        "null_ptr"
    );
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx), 0), nullPtr);
    builder.CreateBr(ret);
    
    // Return buffer pointer
    builder.SetInsertPoint(ret);
    builder.CreateRet(buffer);
    
    return decryptFunc;
}

bool StringObfuscation::obfuscateString(llvm::Module &M,
                                       llvm::GlobalVariable* strVar,
                                       llvm::Function* decryptFunc) {
    llvm::LLVMContext& ctx = M.getContext();
    
    // Get original string
    auto* init = llvm::dyn_cast<llvm::ConstantDataArray>(strVar->getInitializer());
    if (!init) {
        return false;
    }
    
    std::string originalStr = init->getAsString().str();
    if (originalStr.empty()) {
        return false;
    }
    
    // Encrypt the string
    uint32_t key;
    std::vector<uint8_t> encrypted = encryptString(originalStr, key);
    
    // Create new global variable for encrypted data
    llvm::ArrayType* arrayType = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), encrypted.size());
    
    std::vector<llvm::Constant*> encryptedConstants;
    for (uint8_t byte : encrypted) {
        encryptedConstants.push_back(llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx), byte));
    }
    
    llvm::Constant* encryptedArray = llvm::ConstantArray::get(arrayType, encryptedConstants);
    
    llvm::GlobalVariable* encryptedVar = new llvm::GlobalVariable(
        M,
        arrayType,
        true,
        llvm::GlobalValue::PrivateLinkage,
        encryptedArray,
        ".enc_str"
    );
    
    // Replace all uses of original string with decryption call
    std::vector<llvm::User*> users(strVar->user_begin(), strVar->user_end());
    
    for (auto* user : users) {
        if (auto* inst = llvm::dyn_cast<llvm::Instruction>(user)) {
            llvm::IRBuilder<> builder(inst);
            
            // Create decryption call
            llvm::Value* lengthVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), encrypted.size());
            llvm::Value* keyVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), key);
            
            llvm::Value* decrypted = builder.CreateCall(
                decryptFunc,
                {encryptedVar, lengthVal, keyVal},
                "decrypted_str"
            );
            
            // Replace use
            inst->replaceUsesOfWith(strVar, decrypted);
        }
    }
    
    return true;
}

} // namespace obfuscator

static llvm::RegisterPass<obfuscator::StringObfuscation> X(
    "string-obfuscation", "String Obfuscation Pass", false, false);
