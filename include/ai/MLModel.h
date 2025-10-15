#ifndef ML_MODEL_H
#define ML_MODEL_H

#include "AIProfiler.h"
#include <vector>
#include <string>

namespace obfuscator {

class MLModel {
public:
    MLModel();
    ~MLModel();
    
    // Train model with feature-strategy pairs
    bool train(const std::vector<ProgramFeatures>& features,
              const std::vector<ObfuscationStrategy>& strategies);
    
    // Predict strategy for given features
    ObfuscationStrategy predictStrategy(const ProgramFeatures& features);
    
    // Load/save model
    bool load(const std::string& path);
    bool save(const std::string& path);
    
    // Check if model is loaded
    bool isLoaded() const { return loaded_; }
    
private:
    bool loaded_;
    
    // Simple decision tree nodes
    struct DecisionNode {
        std::string featureName;
        double threshold;
        int leftChild;
        int rightChild;
        ObfuscationStrategy prediction;
        bool isLeaf;
    };
    
    std::vector<DecisionNode> tree_;
    
    // Build decision tree
    void buildTree(const std::vector<ProgramFeatures>& features,
                  const std::vector<ObfuscationStrategy>& strategies);
    
    // Normalize features
    std::vector<double> featuresToVector(const ProgramFeatures& features);
    
    // Calculate strategy from decision path
    ObfuscationStrategy traverse(const std::vector<double>& featureVec);
};

} // namespace obfuscator

#endif // ML_MODEL_H
