#ifndef CODE_WATERMARKING_H
#define CODE_WATERMARKING_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include <string>

namespace obfuscator {

class CodeWatermarking : public llvm::ModulePass {
public:
    static char ID;
    
    CodeWatermarking();
    explicit CodeWatermarking(const std::string& payload);
    
    bool runOnModule(llvm::Module &M) override;
    
    llvm::StringRef getPassName() const override {
        return "Code Watermarking";
    }
    
    void setPayload(const std::string& payload) { payload_ = payload; }
    
    int getWatermarkCount() const { return watermarkCount_; }
    
private:
    std::string payload_;
    int watermarkCount_;
    
    // Embed watermark in module
    bool embedWatermark(llvm::Module &M);
    
    // Create watermark as global data
    llvm::GlobalVariable* createWatermarkData(llvm::Module &M);
    
    // Embed watermark in code structure
    bool embedStructuralWatermark(llvm::Module &M);
};

} // namespace obfuscator

#endif // CODE_WATERMARKING_H
