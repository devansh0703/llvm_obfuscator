#include "InputHandler.h"
#include "utils/Logger.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include <fstream>

namespace obfuscator {

InputHandler::InputHandler() = default;
InputHandler::~InputHandler() = default;

std::unique_ptr<llvm::Module> InputHandler::loadFromFile(const std::string& path,
                                                        llvm::LLVMContext& context) {
    Logger::info("Loading file: " + path);
    
    FileFormat format = detectFormat(path);
    
    switch (format) {
        case FileFormat::IR:
            return loadFromIR(path, context);
        case FileFormat::Bitcode:
            return loadFromBitcode(path, context);
        case FileFormat::Source:
            return loadFromSource(path, context);
        default:
            Logger::error("Unknown file format: " + path);
            return nullptr;
    }
}

std::unique_ptr<llvm::Module> InputHandler::loadFromIR(const std::string& path,
                                                      llvm::LLVMContext& context) {
    Logger::info("Loading LLVM IR: " + path);
    
    llvm::SMDiagnostic err;
    std::unique_ptr<llvm::Module> module = llvm::parseIRFile(path, err, context);
    
    if (!module) {
        Logger::error("Failed to parse IR: " + err.getMessage().str());
        return nullptr;
    }
    
    Logger::info("IR loaded successfully");
    return module;
}

std::unique_ptr<llvm::Module> InputHandler::loadFromBitcode(const std::string& path,
                                                           llvm::LLVMContext& context) {
    Logger::info("Loading LLVM bitcode: " + path);
    
    auto bufferOrErr = llvm::MemoryBuffer::getFile(path);
    
    if (!bufferOrErr) {
        Logger::error("Failed to read bitcode file: " + path);
        return nullptr;
    }
    
    auto moduleOrErr = llvm::parseBitcodeFile(bufferOrErr.get()->getMemBufferRef(), context);
    
    if (!moduleOrErr) {
        Logger::error("Failed to parse bitcode");
        return nullptr;
    }
    
    Logger::info("Bitcode loaded successfully");
    return std::move(moduleOrErr.get());
}

std::unique_ptr<llvm::Module> InputHandler::loadFromSource(const std::string& path,
                                                          llvm::LLVMContext& context) {
    Logger::info("Compiling source to IR: " + path);
    Logger::error("Source compilation not yet implemented");
    Logger::info("Please compile to LLVM IR first using: clang -S -emit-llvm " + path);
    
    return nullptr;
}

InputHandler::FileFormat InputHandler::detectFormat(const std::string& path) {
    // Check file extension
    auto hasExtension = [](const std::string& str, const std::string& ext) {
        return str.size() >= ext.size() && 
               str.compare(str.size() - ext.size(), ext.size(), ext) == 0;
    };
    
    if (hasExtension(path, ".ll")) {
        return FileFormat::IR;
    } else if (hasExtension(path, ".bc")) {
        return FileFormat::Bitcode;
    } else if (hasExtension(path, ".c") || hasExtension(path, ".cpp") || 
               hasExtension(path, ".cc") || hasExtension(path, ".cxx")) {
        return FileFormat::Source;
    }
    
    // Try to detect by reading file header
    std::ifstream file(path, std::ios::binary);
    
    if (!file.is_open()) {
        return FileFormat::Unknown;
    }
    
    char magic[4];
    file.read(magic, 4);
    
    // Check for bitcode magic number
    if (magic[0] == 'B' && magic[1] == 'C') {
        return FileFormat::Bitcode;
    }
    
    // Default to IR
    return FileFormat::IR;
}

} // namespace obfuscator
