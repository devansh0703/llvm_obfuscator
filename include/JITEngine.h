#ifndef JIT_ENGINE_H
#define JIT_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"

namespace obfuscator {

struct JITStubInfo {
    std::string originalFunctionName;
    std::string stubFunctionName;
    size_t stubSize = 0;
    std::vector<uint8_t> encryptedCode;
    uint32_t decryptionKey = 0;
};

class RuntimeObfuscator;
class StubGenerator;

class JITEngine {
public:
    JITEngine();
    ~JITEngine();
    
    // Initialize JIT for target module
    bool initialize(llvm::Module& module);
    
    // Create JIT stubs for specified functions
    bool createJITStubs(llvm::Module& module,
                       const std::vector<std::string>& functionNames);
    
    // Create JIT stub for single function
    bool createJITStub(llvm::Module& module,
                      const std::string& functionName);
    
    // Inject runtime obfuscation code
    bool injectRuntimeObfuscation(llvm::Module& module);
    
    // Get stub information
    const std::vector<JITStubInfo>& getStubInfo() const { return stubs_; }
    
    // Generate runtime library
    bool generateRuntimeLibrary(const std::string& outputPath);
    
private:
    std::unique_ptr<RuntimeObfuscator> runtimeObfuscator_;
    std::unique_ptr<StubGenerator> stubGenerator_;
    std::vector<JITStubInfo> stubs_;
    
    // Create wrapper function
    llvm::Function* createWrapperFunction(llvm::Module& module,
                                         llvm::Function* original);
    
    // Encrypt function code
    std::vector<uint8_t> encryptFunctionCode(llvm::Function* func,
                                            uint32_t& outKey);
};

} // namespace obfuscator

#endif // JIT_ENGINE_H
