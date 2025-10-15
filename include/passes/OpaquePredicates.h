#ifndef OPAQUE_PREDICATES_H
#define OPAQUE_PREDICATES_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

namespace obfuscator {

class OpaquePredicates : public llvm::FunctionPass {
public:
    static char ID;
    
    OpaquePredicates();
    explicit OpaquePredicates(int count);
    
    bool runOnFunction(llvm::Function &F) override;
    
    llvm::StringRef getPassName() const override {
        return "Opaque Predicates";
    }
    
    void setCount(int count) { targetCount_ = count; }
    
    int getPredicateCount() const { return predicateCount_; }
    
private:
    int targetCount_;
    int predicateCount_;
    
    // Insert opaque predicate
    bool insertOpaquePredicate(llvm::Function &F, llvm::BasicBlock* BB);
    
    // Create always-true predicate
    llvm::Value* createAlwaysTruePredicate(llvm::IRBuilder<>& builder);
    
    // Create always-false predicate
    llvm::Value* createAlwaysFalsePredicate(llvm::IRBuilder<>& builder);
    
    // Create contextual predicate (depends on global state)
    llvm::Value* createContextualPredicate(llvm::IRBuilder<>& builder,
                                          llvm::Function &F);
};

} // namespace obfuscator

#endif // OPAQUE_PREDICATES_H
