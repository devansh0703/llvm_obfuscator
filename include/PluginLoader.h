#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "PluginAPI.h"

namespace obfuscator {

class PluginLoader {
public:
    PluginLoader();
    ~PluginLoader();
    
    // Load plugin from shared library
    bool loadPlugin(const std::string& path);
    
    // Unload specific plugin
    bool unloadPlugin(const std::string& name);
    
    // Unload all plugins
    void unloadAll();
    
    // Get loaded plugins
    std::vector<std::string> getLoadedPlugins() const;
    
    // Get plugin by name
    IObfuscationPlugin* getPlugin(const std::string& name);
    
    // Create pass from plugin
    llvm::Pass* createPassFromPlugin(const std::string& pluginName);
    
private:
    struct PluginHandle {
        void* libraryHandle;
        std::unique_ptr<IObfuscationPlugin> plugin;
        std::string path;
    };
    
    std::map<std::string, PluginHandle> plugins_;
    
    // Platform-specific loading
    void* loadLibrary(const std::string& path);
    void* getSymbol(void* handle, const std::string& symbol);
    void unloadLibrary(void* handle);
};

} // namespace obfuscator

#endif // PLUGIN_LOADER_H
