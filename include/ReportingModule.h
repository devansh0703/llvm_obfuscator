#ifndef REPORTING_MODULE_H
#define REPORTING_MODULE_H

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace obfuscator {

struct PassMetrics {
    std::string passName;
    int insertions = 0;
    int modifications = 0;
    int deletions = 0;
    double executionTimeMs = 0.0;
    std::map<std::string, int> customMetrics;
};

struct ObfuscationReport {
    // Input information
    std::string inputFile;
    size_t inputFileSize = 0;
    int inputInstructionCount = 0;
    int inputFunctionCount = 0;
    int inputBasicBlockCount = 0;
    
    // Output information
    std::string outputFile;
    size_t outputFileSize = 0;
    int outputInstructionCount = 0;
    int outputFunctionCount = 0;
    int outputBasicBlockCount = 0;
    
    // Obfuscation parameters
    int obfuscationPasses = 0;
    int obfuscationCycles = 0;
    
    // Transformation statistics
    int bogusCodeInsertions = 0;
    int fakeLoopsGenerated = 0;
    int stringObfuscationCount = 0;
    int opaquePredicatesAdded = 0;
    int controlFlowFlattened = 0;
    int instructionSubstitutions = 0;
    int antiDebugChecks = 0;
    int jitStubsCreated = 0;
    int watermarksEmbedded = 0;
    
    // Metrics
    double totalObfuscationTimeMs = 0.0;
    double codeEntropyBefore = 0.0;
    double codeEntropyAfter = 0.0;
    double entropyDelta = 0.0;
    
    // AI profiler data
    std::string aiObfuscationStrategy;
    std::map<std::string, float> aiConfidenceScores;
    
    // Per-pass metrics
    std::vector<PassMetrics> passMetrics;
    
    // Timestamp
    std::chrono::system_clock::time_point timestamp;
    
    // Additional metadata
    std::map<std::string, std::string> metadata;
};

class ReportingModule {
public:
    ReportingModule();
    ~ReportingModule();
    
    // Generate reports in various formats
    bool generateJSON(const ObfuscationReport& report, const std::string& path);
    bool generateCSV(const ObfuscationReport& report, const std::string& path);
    bool generateHTML(const ObfuscationReport& report, const std::string& path);
    bool generatePDF(const ObfuscationReport& report, const std::string& path);
    
    // Update report with metrics
    void updateReport(ObfuscationReport& report, const PassMetrics& metrics);
    
    // Calculate entropy
    static double calculateEntropy(const std::vector<uint8_t>& data);
    static double calculateEntropy(const std::string& filePath);
    
private:
    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp);
    std::string escapeJSON(const std::string& str);
    std::string escapeHTML(const std::string& str);
};

} // namespace obfuscator

#endif // REPORTING_MODULE_H
