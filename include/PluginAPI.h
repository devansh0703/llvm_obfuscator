#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include <string>
#include <map>

#define OBFUSCATOR_PLUGIN_API_VERSION 1

namespace obfuscator {

// Plugin information structure
struct PluginInfo {
    const char* name;
    const char* version;
    const char* author;
    const char* description;
    int apiVersion;
};

// Plugin interface
class IObfuscationPlugin {
public:
    virtual ~IObfuscationPlugin() = default;
    
    // Get plugin information
    virtual PluginInfo getInfo() const = 0;
    
    // Create pass instance
    virtual llvm::Pass* createPass() = 0;
    
    // Initialize plugin with configuration
    virtual bool initialize(const std::map<std::string, std::string>& config) = 0;
    
    // Cleanup
    virtual void cleanup() = 0;
};

} // namespace obfuscator

// Plugin export functions (must be implemented by plugins)
extern "C" {
    obfuscator::IObfuscationPlugin* createPlugin();
    void destroyPlugin(obfuscator::IObfuscationPlugin* plugin);
}

#endif // PLUGIN_API_H
