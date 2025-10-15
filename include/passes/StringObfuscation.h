#ifndef STRING_OBFUSCATION_H
#define STRING_OBFUSCATION_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"

namespace obfuscator {

class StringObfuscation : public llvm::ModulePass {
public:
    static char ID;
    
    StringObfuscation();
    explicit StringObfuscation(float intensity);
    
    bool runOnModule(llvm::Module &M) override;
    
    llvm::StringRef getPassName() const override {
        return "String Obfuscation";
    }
    
    void setIntensity(float intensity) { intensity_ = intensity; }
    
    int getObfuscationCount() const { return obfuscationCount_; }
    
private:
    float intensity_;
    int obfuscationCount_;
    
    // Find all string constants
    std::vector<llvm::GlobalVariable*> findStrings(llvm::Module &M);
    
    // Encrypt string
    std::vector<uint8_t> encryptString(const std::string& str, uint32_t& key);
    
    // Create decryption function
    llvm::Function* createDecryptionFunction(llvm::Module &M);
    
    // Replace string with encrypted version
    bool obfuscateString(llvm::Module &M,
                        llvm::GlobalVariable* strVar,
                        llvm::Function* decryptFunc);
};

} // namespace obfuscator

#endif // STRING_OBFUSCATION_H
