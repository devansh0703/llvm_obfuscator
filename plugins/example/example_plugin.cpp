#include "PluginAPI.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

// Example custom obfuscation pass
class ExamplePass : public llvm::FunctionPass {
public:
    static char ID;
    
    ExamplePass() : FunctionPass(ID) {}
    
    bool runOnFunction(llvm::Function &F) override {
        // Example: Insert a NOP at the beginning of each function
        llvm::LLVMContext& ctx = F.getContext();
        llvm::BasicBlock& entry = F.getEntryBlock();
        llvm::IRBuilder<> builder(&*entry.getFirstInsertionPt());
        
        // Add a dummy computation
        llvm::Value* dummy = builder.CreateAlloca(llvm::Type::getInt32Ty(ctx));
        builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), dummy);
        
        return true;
    }
    
    llvm::StringRef getPassName() const override {
        return "Example Custom Pass";
    }
};

char ExamplePass::ID = 0;

// Plugin implementation
class ExamplePlugin : public obfuscator::IObfuscationPlugin {
public:
    obfuscator::PluginInfo getInfo() const override {
        obfuscator::PluginInfo info;
        info.name = "ExamplePlugin";
        info.version = "1.0.0";
        info.author = "Your Name";
        info.description = "Example obfuscation plugin";
        info.apiVersion = OBFUSCATOR_PLUGIN_API_VERSION;
        return info;
    }
    
    llvm::Pass* createPass() override {
        return new ExamplePass();
    }
    
    bool initialize(const std::map<std::string, std::string>& config) override {
        // Initialize plugin with configuration
        return true;
    }
    
    void cleanup() override {
        // Cleanup resources
    }
};

// Plugin export functions
extern "C" {
    obfuscator::IObfuscationPlugin* createPlugin() {
        return new ExamplePlugin();
    }
    
    void destroyPlugin(obfuscator::IObfuscationPlugin* plugin) {
        delete plugin;
    }
}
