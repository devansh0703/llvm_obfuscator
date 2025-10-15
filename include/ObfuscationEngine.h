#ifndef OBFUSCATION_ENGINE_H
#define OBFUSCATION_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "ObfuscationConfig.h"
#include "ReportingModule.h"

namespace obfuscator {

class AIProfiler;
class JITEngine;
class PluginLoader;

class ObfuscationEngine {
public:
    ObfuscationEngine();
    ~ObfuscationEngine();

    // Main obfuscation pipeline
    bool obfuscate(const std::string& inputPath, 
                   const std::string& outputPath,
                   const ObfuscationConfig& config);

    // Get reporting data
    const ObfuscationReport& getReport() const { return report_; }

    // Plugin management
    bool loadPlugin(const std::string& pluginPath);
    void unloadAllPlugins();

private:
    // Initialize passes based on config
    void initializePasses(const ObfuscationConfig& config);
    
    // Run obfuscation cycles
    bool runObfuscationCycles(llvm::Module& module, 
                             const ObfuscationConfig& config);
    
    // Apply individual pass
    bool applyPass(llvm::Module& module, llvm::Pass* pass);
    
    // Collect metrics
    void collectMetrics(const llvm::Module& module);

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<AIProfiler> aiProfiler_;
    std::unique_ptr<JITEngine> jitEngine_;
    std::unique_ptr<PluginLoader> pluginLoader_;
    std::unique_ptr<ReportingModule> reportingModule_;
    
    ObfuscationReport report_;
    std::vector<std::unique_ptr<llvm::Pass>> passes_;
};

} // namespace obfuscator

#endif // OBFUSCATION_ENGINE_H
