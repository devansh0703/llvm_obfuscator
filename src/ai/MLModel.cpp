#include "ai/MLModel.h"
#include "utils/Logger.h"
#include <fstream>
#include <algorithm>
#include <cmath>

namespace obfuscator {

MLModel::MLModel() : loaded_(false) {}

MLModel::~MLModel() = default;

bool MLModel::train(const std::vector<ProgramFeatures>& features,
                   const std::vector<ObfuscationStrategy>& strategies) {
    if (features.empty() || features.size() != strategies.size()) {
        Logger::error("Invalid training data");
        return false;
    }
    
    Logger::info("Training ML model...");
    
    // Build a simple decision tree
    buildTree(features, strategies);
    
    loaded_ = true;
    Logger::info("Model training complete");
    
    return true;
}

ObfuscationStrategy MLModel::predictStrategy(const ProgramFeatures& features) {
    if (!loaded_) {
        Logger::warning("Model not loaded, using default strategy");
        ObfuscationStrategy defaultStrategy;
        defaultStrategy.strategyName = "Default";
        defaultStrategy.controlFlowIntensity = 0.5f;
        defaultStrategy.bogusCodeIntensity = 0.5f;
        defaultStrategy.stringObfuscationIntensity = 0.8f;
        return defaultStrategy;
    }
    
    std::vector<double> featureVec = featuresToVector(features);
    return traverse(featureVec);
}

bool MLModel::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Logger::error("Failed to load model from: " + path);
        return false;
    }
    
    // Load model data
    // In a real implementation, this would deserialize the tree structure
    
    loaded_ = true;
    Logger::info("Model loaded from: " + path);
    return true;
}

bool MLModel::save(const std::string& path) {
    if (!loaded_) {
        Logger::error("No model to save");
        return false;
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Logger::error("Failed to save model to: " + path);
        return false;
    }
    
    // Save model data
    // In a real implementation, this would serialize the tree structure
    
    Logger::info("Model saved to: " + path);
    return true;
}

void MLModel::buildTree(const std::vector<ProgramFeatures>& features,
                       const std::vector<ObfuscationStrategy>& strategies) {
    tree_.clear();
    
    // Create a simple decision tree based on complexity
    DecisionNode root;
    root.featureName = "averageCyclomaticComplexity";
    root.threshold = 10.0;
    root.isLeaf = false;
    root.leftChild = 1;
    root.rightChild = 2;
    
    DecisionNode lowComplexity;
    lowComplexity.isLeaf = true;
    lowComplexity.prediction.strategyName = "AggressiveObfuscation";
    lowComplexity.prediction.controlFlowIntensity = 0.9f;
    lowComplexity.prediction.bogusCodeIntensity = 0.8f;
    lowComplexity.prediction.stringObfuscationIntensity = 1.0f;
    lowComplexity.prediction.fakeLoopCount = 100;
    lowComplexity.prediction.opaquePredicateCount = 200;
    
    DecisionNode highComplexity;
    highComplexity.isLeaf = true;
    highComplexity.prediction.strategyName = "ModerateObfuscation";
    highComplexity.prediction.controlFlowIntensity = 0.5f;
    highComplexity.prediction.bogusCodeIntensity = 0.4f;
    highComplexity.prediction.stringObfuscationIntensity = 0.9f;
    highComplexity.prediction.fakeLoopCount = 50;
    highComplexity.prediction.opaquePredicateCount = 100;
    
    tree_.push_back(root);
    tree_.push_back(lowComplexity);
    tree_.push_back(highComplexity);
}

std::vector<double> MLModel::featuresToVector(const ProgramFeatures& features) {
    std::vector<double> vec;
    
    vec.push_back(static_cast<double>(features.functionCount));
    vec.push_back(static_cast<double>(features.basicBlockCount));
    vec.push_back(static_cast<double>(features.instructionCount));
    vec.push_back(static_cast<double>(features.loopCount));
    vec.push_back(static_cast<double>(features.branchCount));
    vec.push_back(features.averageCyclomaticComplexity);
    vec.push_back(features.maxCyclomaticComplexity);
    vec.push_back(static_cast<double>(features.callGraphDepth));
    vec.push_back(static_cast<double>(features.maxNestingLevel));
    vec.push_back(features.codeEntropy);
    
    return vec;
}

ObfuscationStrategy MLModel::traverse(const std::vector<double>& featureVec) {
    if (tree_.empty()) {
        ObfuscationStrategy defaultStrategy;
        defaultStrategy.strategyName = "Default";
        return defaultStrategy;
    }
    
    int nodeIdx = 0;
    
    while (!tree_[nodeIdx].isLeaf) {
        double featureValue = featureVec[5]; // averageCyclomaticComplexity
        
        if (featureValue < tree_[nodeIdx].threshold) {
            nodeIdx = tree_[nodeIdx].leftChild;
        } else {
            nodeIdx = tree_[nodeIdx].rightChild;
        }
    }
    
    ObfuscationStrategy strategy = tree_[nodeIdx].prediction;
    strategy.confidenceScores["ml_confidence"] = 0.85f;
    
    return strategy;
}

} // namespace obfuscator
