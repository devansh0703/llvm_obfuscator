#include "PluginLoader.h"
#include "utils/Logger.h"
#include <dlfcn.h>  // For POSIX systems

namespace obfuscator {

PluginLoader::PluginLoader() {
    Logger::info("PluginLoader initialized");
}

PluginLoader::~PluginLoader() {
    unloadAll();
}

bool PluginLoader::loadPlugin(const std::string& path) {
    Logger::info("Loading plugin: " + path);
    
    // Load shared library
    void* handle = loadLibrary(path);
    
    if (!handle) {
        Logger::error("Failed to load plugin library: " + path);
        return false;
    }
    
    // Get plugin creation function
    typedef IObfuscationPlugin* (*CreatePluginFunc)();
    CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(
        getSymbol(handle, "createPlugin"));
    
    if (!createFunc) {
        Logger::error("Plugin does not export createPlugin function");
        unloadLibrary(handle);
        return false;
    }
    
    // Create plugin instance
    IObfuscationPlugin* plugin = createFunc();
    
    if (!plugin) {
        Logger::error("Failed to create plugin instance");
        unloadLibrary(handle);
        return false;
    }
    
    // Get plugin info
    PluginInfo info = plugin->getInfo();
    Logger::info("Loaded plugin: " + std::string(info.name) + " v" + info.version);
    
    // Initialize plugin
    std::map<std::string, std::string> config;
    if (!plugin->initialize(config)) {
        Logger::error("Plugin initialization failed");
        delete plugin;
        unloadLibrary(handle);
        return false;
    }
    
    // Store plugin
    PluginHandle pluginHandle;
    pluginHandle.libraryHandle = handle;
    pluginHandle.plugin.reset(plugin);
    pluginHandle.path = path;
    
    plugins_[info.name] = std::move(pluginHandle);
    
    Logger::info("Plugin loaded successfully: " + std::string(info.name));
    
    return true;
}

bool PluginLoader::unloadPlugin(const std::string& name) {
    auto it = plugins_.find(name);
    
    if (it == plugins_.end()) {
        Logger::warning("Plugin not found: " + name);
        return false;
    }
    
    Logger::info("Unloading plugin: " + name);
    
    // Cleanup plugin
    it->second.plugin->cleanup();
    it->second.plugin.reset();
    
    // Unload library
    unloadLibrary(it->second.libraryHandle);
    
    plugins_.erase(it);
    
    return true;
}

void PluginLoader::unloadAll() {
    Logger::info("Unloading all plugins");
    
    std::vector<std::string> pluginNames;
    for (const auto& [name, handle] : plugins_) {
        pluginNames.push_back(name);
    }
    
    for (const auto& name : pluginNames) {
        unloadPlugin(name);
    }
}

std::vector<std::string> PluginLoader::getLoadedPlugins() const {
    std::vector<std::string> names;
    
    for (const auto& [name, handle] : plugins_) {
        names.push_back(name);
    }
    
    return names;
}

IObfuscationPlugin* PluginLoader::getPlugin(const std::string& name) {
    auto it = plugins_.find(name);
    
    if (it == plugins_.end()) {
        return nullptr;
    }
    
    return it->second.plugin.get();
}

llvm::Pass* PluginLoader::createPassFromPlugin(const std::string& pluginName) {
    IObfuscationPlugin* plugin = getPlugin(pluginName);
    
    if (!plugin) {
        Logger::error("Plugin not found: " + pluginName);
        return nullptr;
    }
    
    return plugin->createPass();
}

void* PluginLoader::loadLibrary(const std::string& path) {
#ifdef _WIN32
    return LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}

void* PluginLoader::getSymbol(void* handle, const std::string& symbol) {
#ifdef _WIN32
    return GetProcAddress((HMODULE)handle, symbol.c_str());
#else
    return dlsym(handle, symbol.c_str());
#endif
}

void PluginLoader::unloadLibrary(void* handle) {
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

} // namespace obfuscator
