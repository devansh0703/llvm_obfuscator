#ifndef RUNTIME_OBFUSCATOR_H
#define RUNTIME_OBFUSCATOR_H

#include "llvm/IR/Module.h"

namespace obfuscator {

class RuntimeObfuscator {
public:
    RuntimeObfuscator();
    ~RuntimeObfuscator();
    
    // Inject runtime obfuscation code
    bool inject(llvm::Module& module);
    
private:
    // Create runtime decryption function
    llvm::Function* createDecryptionRuntime(llvm::Module& module);
    
    // Create self-modifying code stubs
    bool createSelfModifyingStubs(llvm::Module& module);
};

} // namespace obfuscator

#endif // RUNTIME_OBFUSCATOR_H
