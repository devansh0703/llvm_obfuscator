#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

namespace obfuscator {

class InputHandler {
public:
    InputHandler();
    ~InputHandler();
    
    // Load module from various formats
    std::unique_ptr<llvm::Module> loadFromFile(const std::string& path,
                                              llvm::LLVMContext& context);
    
    // Load from LLVM IR
    std::unique_ptr<llvm::Module> loadFromIR(const std::string& path,
                                            llvm::LLVMContext& context);
    
    // Load from bitcode
    std::unique_ptr<llvm::Module> loadFromBitcode(const std::string& path,
                                                 llvm::LLVMContext& context);
    
    // Load from C/C++ source (compile to IR first)
    std::unique_ptr<llvm::Module> loadFromSource(const std::string& path,
                                                llvm::LLVMContext& context);
    
private:
    // Detect file format
    enum class FileFormat {
        IR,
        Bitcode,
        Source,
        Unknown
    };
    
    FileFormat detectFormat(const std::string& path);
};

} // namespace obfuscator

#endif // INPUT_HANDLER_H
