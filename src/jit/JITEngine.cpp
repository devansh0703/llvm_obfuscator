#include "JITEngine.h"
#include "jit/RuntimeObfuscator.h"
#include "jit/StubGenerator.h"
#include "utils/Logger.h"
#include "utils/Crypto.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"

namespace obfuscator {

JITEngine::JITEngine()
    : runtimeObfuscator_(std::make_unique<RuntimeObfuscator>()),
      stubGenerator_(std::make_unique<StubGenerator>()) {
    Logger::info("JITEngine initialized");
    
    // Initialize LLVM JIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

JITEngine::~JITEngine() = default;

bool JITEngine::initialize(llvm::Module& module) {
    Logger::info("Initializing JIT for module: " + module.getName().str());
    return true;
}

bool JITEngine::createJITStubs(llvm::Module& module,
                              const std::vector<std::string>& functionNames) {
    Logger::info("Creating JIT stubs for " + std::to_string(functionNames.size()) + " functions");
    
    for (const auto& funcName : functionNames) {
        if (!createJITStub(module, funcName)) {
            Logger::warning("Failed to create JIT stub for: " + funcName);
        }
    }
    
    return !stubs_.empty();
}

bool JITEngine::createJITStub(llvm::Module& module, const std::string& functionName) {
    llvm::Function* originalFunc = module.getFunction(functionName);
    
    if (!originalFunc || originalFunc->isDeclaration()) {
        Logger::warning("Function not found or is declaration: " + functionName);
        return false;
    }
    
    Logger::info("Creating JIT stub for: " + functionName);
    
    // Create wrapper function
    llvm::Function* wrapper = createWrapperFunction(module, originalFunc);
    
    if (!wrapper) {
        return false;
    }
    
    // Encrypt original function code
    uint32_t key;
    std::vector<uint8_t> encrypted = encryptFunctionCode(originalFunc, key);
    
    // Create stub info
    JITStubInfo stubInfo;
    stubInfo.originalFunctionName = functionName;
    stubInfo.stubFunctionName = wrapper->getName().str();
    stubInfo.stubSize = wrapper->getInstructionCount();
    stubInfo.encryptedCode = encrypted;
    stubInfo.decryptionKey = key;
    
    stubs_.push_back(stubInfo);
    
    Logger::info("Created JIT stub: " + stubInfo.stubFunctionName);
    
    return true;
}

bool JITEngine::injectRuntimeObfuscation(llvm::Module& module) {
    Logger::info("Injecting runtime obfuscation");
    return runtimeObfuscator_->inject(module);
}

bool JITEngine::generateRuntimeLibrary(const std::string& outputPath) {
    Logger::info("Generating runtime library: " + outputPath);
    
    // Generate C++ runtime library for JIT stub execution
    std::ofstream runtimeFile(outputPath);
    
    if (!runtimeFile.is_open()) {
        Logger::error("Failed to create runtime library file");
        return false;
    }
    
    runtimeFile << "// Auto-generated JIT Runtime Library\n\n";
    runtimeFile << "#include <cstdint>\n";
    runtimeFile << "#include <vector>\n";
    runtimeFile << "#include <cstring>\n\n";
    
    runtimeFile << "namespace jit_runtime {\n\n";
    
    runtimeFile << "// XOR decryption function\n";
    runtimeFile << "void decrypt_code(uint8_t* code, size_t size, uint32_t key) {\n";
    runtimeFile << "    for (size_t i = 0; i < size; ++i) {\n";
    runtimeFile << "        uint8_t key_byte = (key >> ((i % 4) * 8)) & 0xFF;\n";
    runtimeFile << "        code[i] ^= key_byte;\n";
    runtimeFile << "    }\n";
    runtimeFile << "}\n\n";
    
    runtimeFile << "// Execute decrypted code\n";
    runtimeFile << "typedef void (*FunctionPtr)();\n\n";
    runtimeFile << "void execute_stub(const uint8_t* encrypted, size_t size, uint32_t key) {\n";
    runtimeFile << "    std::vector<uint8_t> code(encrypted, encrypted + size);\n";
    runtimeFile << "    decrypt_code(code.data(), size, key);\n";
    runtimeFile << "    // In production: use proper memory protection and execution\n";
    runtimeFile << "}\n\n";
    
    runtimeFile << "} // namespace jit_runtime\n";
    
    runtimeFile.close();
    
    Logger::info("Runtime library generated successfully");
    
    return true;
}

llvm::Function* JITEngine::createWrapperFunction(llvm::Module& module,
                                                llvm::Function* original) {
    llvm::LLVMContext& ctx = module.getContext();
    
    // Create wrapper with same signature
    llvm::FunctionType* funcType = original->getFunctionType();
    llvm::Function* wrapper = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        original->getName() + "_jit_wrapper",
        module
    );
    
    // Copy attributes
    wrapper->copyAttributesFrom(original);
    
    // Create basic block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", wrapper);
    llvm::IRBuilder<> builder(entry);
    
    // For now, just call the original function
    // In production, this would decrypt and execute the JIT code
    std::vector<llvm::Value*> args;
    for (auto& arg : wrapper->args()) {
        args.push_back(&arg);
    }
    
    llvm::Value* result = builder.CreateCall(original, args);
    
    if (funcType->getReturnType()->isVoidTy()) {
        builder.CreateRetVoid();
    } else {
        builder.CreateRet(result);
    }
    
    return wrapper;
}

std::vector<uint8_t> JITEngine::encryptFunctionCode(llvm::Function* func, uint32_t& outKey) {
    // Simulate encrypting function code
    // In production, this would serialize the LLVM IR or machine code
    
    std::string funcName = func->getName().str();
    std::vector<uint8_t> data(funcName.begin(), funcName.end());
    
    outKey = Crypto::generateKey();
    return Crypto::xorEncrypt(data, outKey);
}

} // namespace obfuscator
