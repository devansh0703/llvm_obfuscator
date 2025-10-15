#include "ReportingModule.h"
#include "utils/Logger.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <map>
#include <algorithm>

namespace obfuscator {

ReportingModule::ReportingModule() {
    Logger::info("ReportingModule initialized");
}

ReportingModule::~ReportingModule() = default;

bool ReportingModule::generateJSON(const ObfuscationReport& report, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to open JSON report file: " + path);
        return false;
    }
    
    file << "{\n";
    file << "  \"metadata\": {\n";
    file << "    \"timestamp\": \"" << formatTimestamp(report.timestamp) << "\",\n";
    file << "    \"version\": \"1.0.0\"\n";
    file << "  },\n";
    
    file << "  \"input\": {\n";
    file << "    \"file\": \"" << escapeJSON(report.inputFile) << "\",\n";
    file << "    \"size\": " << report.inputFileSize << ",\n";
    file << "    \"functions\": " << report.inputFunctionCount << ",\n";
    file << "    \"basicBlocks\": " << report.inputBasicBlockCount << ",\n";
    file << "    \"instructions\": " << report.inputInstructionCount << ",\n";
    file << "    \"entropy\": " << report.codeEntropyBefore << "\n";
    file << "  },\n";
    
    file << "  \"output\": {\n";
    file << "    \"file\": \"" << escapeJSON(report.outputFile) << "\",\n";
    file << "    \"size\": " << report.outputFileSize << ",\n";
    file << "    \"functions\": " << report.outputFunctionCount << ",\n";
    file << "    \"basicBlocks\": " << report.outputBasicBlockCount << ",\n";
    file << "    \"instructions\": " << report.outputInstructionCount << ",\n";
    file << "    \"entropy\": " << report.codeEntropyAfter << "\n";
    file << "  },\n";
    
    file << "  \"obfuscation\": {\n";
    file << "    \"passes\": " << report.obfuscationPasses << ",\n";
    file << "    \"cycles\": " << report.obfuscationCycles << ",\n";
    file << "    \"timeMs\": " << report.totalObfuscationTimeMs << ",\n";
    file << "    \"entropyDelta\": " << report.entropyDelta << "\n";
    file << "  },\n";
    
    file << "  \"transformations\": {\n";
    file << "    \"bogusCodeInsertions\": " << report.bogusCodeInsertions << ",\n";
    file << "    \"fakeLoops\": " << report.fakeLoopsGenerated << ",\n";
    file << "    \"stringObfuscations\": " << report.stringObfuscationCount << ",\n";
    file << "    \"opaquePredicates\": " << report.opaquePredicatesAdded << ",\n";
    file << "    \"controlFlowFlattened\": " << report.controlFlowFlattened << ",\n";
    file << "    \"instructionSubstitutions\": " << report.instructionSubstitutions << ",\n";
    file << "    \"antiDebugChecks\": " << report.antiDebugChecks << ",\n";
    file << "    \"jitStubs\": " << report.jitStubsCreated << ",\n";
    file << "    \"watermarks\": " << report.watermarksEmbedded << "\n";
    file << "  },\n";
    
    file << "  \"aiProfiler\": {\n";
    file << "    \"strategy\": \"" << escapeJSON(report.aiObfuscationStrategy) << "\",\n";
    file << "    \"confidenceScores\": {\n";
    bool first = true;
    for (const auto& [key, value] : report.aiConfidenceScores) {
        if (!first) file << ",\n";
        file << "      \"" << escapeJSON(key) << "\": " << value;
        first = false;
    }
    file << "\n    }\n";
    file << "  },\n";
    
    file << "  \"passMetrics\": [\n";
    for (size_t i = 0; i < report.passMetrics.size(); ++i) {
        const auto& pm = report.passMetrics[i];
        file << "    {\n";
        file << "      \"name\": \"" << escapeJSON(pm.passName) << "\",\n";
        file << "      \"insertions\": " << pm.insertions << ",\n";
        file << "      \"modifications\": " << pm.modifications << ",\n";
        file << "      \"deletions\": " << pm.deletions << ",\n";
        file << "      \"executionTimeMs\": " << pm.executionTimeMs << "\n";
        file << "    }";
        if (i < report.passMetrics.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n";
    
    file << "}\n";
    file.close();
    
    Logger::info("JSON report generated: " + path);
    return true;
}

bool ReportingModule::generateCSV(const ObfuscationReport& report, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to open CSV report file: " + path);
        return false;
    }
    
    file << "Metric,Value\n";
    file << "Timestamp," << formatTimestamp(report.timestamp) << "\n";
    file << "Input File," << report.inputFile << "\n";
    file << "Input Size," << report.inputFileSize << "\n";
    file << "Input Functions," << report.inputFunctionCount << "\n";
    file << "Input Basic Blocks," << report.inputBasicBlockCount << "\n";
    file << "Input Instructions," << report.inputInstructionCount << "\n";
    file << "Input Entropy," << report.codeEntropyBefore << "\n";
    file << "Output File," << report.outputFile << "\n";
    file << "Output Size," << report.outputFileSize << "\n";
    file << "Output Functions," << report.outputFunctionCount << "\n";
    file << "Output Basic Blocks," << report.outputBasicBlockCount << "\n";
    file << "Output Instructions," << report.outputInstructionCount << "\n";
    file << "Output Entropy," << report.codeEntropyAfter << "\n";
    file << "Entropy Delta," << report.entropyDelta << "\n";
    file << "Obfuscation Passes," << report.obfuscationPasses << "\n";
    file << "Obfuscation Cycles," << report.obfuscationCycles << "\n";
    file << "Total Time (ms)," << report.totalObfuscationTimeMs << "\n";
    file << "Bogus Code Insertions," << report.bogusCodeInsertions << "\n";
    file << "Fake Loops Generated," << report.fakeLoopsGenerated << "\n";
    file << "String Obfuscations," << report.stringObfuscationCount << "\n";
    file << "Opaque Predicates," << report.opaquePredicatesAdded << "\n";
    file << "Control Flow Flattened," << report.controlFlowFlattened << "\n";
    file << "Instruction Substitutions," << report.instructionSubstitutions << "\n";
    file << "Anti-Debug Checks," << report.antiDebugChecks << "\n";
    file << "JIT Stubs," << report.jitStubsCreated << "\n";
    file << "Watermarks," << report.watermarksEmbedded << "\n";
    file << "AI Strategy," << report.aiObfuscationStrategy << "\n";
    
    file.close();
    Logger::info("CSV report generated: " + path);
    return true;
}

bool ReportingModule::generateHTML(const ObfuscationReport& report, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to open HTML report file: " + path);
        return false;
    }
    
    file << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    file << "  <meta charset=\"UTF-8\">\n";
    file << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    file << "  <title>Obfuscation Report</title>\n";
    file << "  <style>\n";
    file << "    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n";
    file << "    .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n";
    file << "    h1 { color: #333; border-bottom: 3px solid #4CAF50; padding-bottom: 10px; }\n";
    file << "    h2 { color: #555; margin-top: 30px; border-bottom: 2px solid #ddd; padding-bottom: 8px; }\n";
    file << "    table { width: 100%; border-collapse: collapse; margin: 20px 0; }\n";
    file << "    th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }\n";
    file << "    th { background-color: #4CAF50; color: white; }\n";
    file << "    tr:hover { background-color: #f5f5f5; }\n";
    file << "    .metric { display: inline-block; margin: 10px 20px 10px 0; }\n";
    file << "    .metric-value { font-size: 24px; font-weight: bold; color: #4CAF50; }\n";
    file << "    .metric-label { font-size: 12px; color: #777; }\n";
    file << "    .positive { color: #4CAF50; }\n";
    file << "    .highlight { background-color: #ffffcc; }\n";
    file << "  </style>\n</head>\n<body>\n";
    
    file << "  <div class=\"container\">\n";
    file << "    <h1>Adaptive Obfuscator Report</h1>\n";
    file << "    <p><strong>Generated:</strong> " << formatTimestamp(report.timestamp) << "</p>\n";
    
    file << "    <h2>Summary</h2>\n";
    file << "    <div>\n";
    file << "      <div class=\"metric\">\n";
    file << "        <div class=\"metric-value\">" << report.obfuscationPasses << "</div>\n";
    file << "        <div class=\"metric-label\">Passes Applied</div>\n";
    file << "      </div>\n";
    file << "      <div class=\"metric\">\n";
    file << "        <div class=\"metric-value\">" << report.totalObfuscationTimeMs << " ms</div>\n";
    file << "        <div class=\"metric-label\">Total Time</div>\n";
    file << "      </div>\n";
    file << "      <div class=\"metric\">\n";
    file << "        <div class=\"metric-value positive\">+" << std::fixed << std::setprecision(1) << (report.entropyDelta * 100) << "%</div>\n";
    file << "        <div class=\"metric-label\">Entropy Increase</div>\n";
    file << "      </div>\n";
    file << "    </div>\n";
    
    file << "    <h2>Input/Output Comparison</h2>\n";
    file << "    <table>\n";
    file << "      <tr><th>Metric</th><th>Input</th><th>Output</th><th>Delta</th></tr>\n";
    file << "      <tr><td>File Size</td><td>" << report.inputFileSize << "</td><td>" << report.outputFileSize << "</td><td class=\"positive\">+" << (report.outputFileSize - report.inputFileSize) << "</td></tr>\n";
    file << "      <tr><td>Functions</td><td>" << report.inputFunctionCount << "</td><td>" << report.outputFunctionCount << "</td><td class=\"positive\">+" << (report.outputFunctionCount - report.inputFunctionCount) << "</td></tr>\n";
    file << "      <tr><td>Basic Blocks</td><td>" << report.inputBasicBlockCount << "</td><td>" << report.outputBasicBlockCount << "</td><td class=\"positive\">+" << (report.outputBasicBlockCount - report.inputBasicBlockCount) << "</td></tr>\n";
    file << "      <tr><td>Instructions</td><td>" << report.inputInstructionCount << "</td><td>" << report.outputInstructionCount << "</td><td class=\"positive\">+" << (report.outputInstructionCount - report.inputInstructionCount) << "</td></tr>\n";
    file << "      <tr><td>Entropy</td><td>" << std::fixed << std::setprecision(4) << report.codeEntropyBefore << "</td><td>" << report.codeEntropyAfter << "</td><td class=\"positive\">+" << report.entropyDelta << "</td></tr>\n";
    file << "    </table>\n";
    
    file << "    <h2>Transformations Applied</h2>\n";
    file << "    <table>\n";
    file << "      <tr><th>Transformation</th><th>Count</th></tr>\n";
    file << "      <tr><td>Bogus Code Insertions</td><td>" << report.bogusCodeInsertions << "</td></tr>\n";
    file << "      <tr><td>Fake Loops Generated</td><td>" << report.fakeLoopsGenerated << "</td></tr>\n";
    file << "      <tr><td>String Obfuscations</td><td>" << report.stringObfuscationCount << "</td></tr>\n";
    file << "      <tr><td>Opaque Predicates</td><td>" << report.opaquePredicatesAdded << "</td></tr>\n";
    file << "      <tr><td>Control Flow Flattened</td><td>" << report.controlFlowFlattened << "</td></tr>\n";
    file << "      <tr><td>Instruction Substitutions</td><td>" << report.instructionSubstitutions << "</td></tr>\n";
    file << "      <tr><td>Anti-Debug Checks</td><td>" << report.antiDebugChecks << "</td></tr>\n";
    file << "      <tr><td>JIT Stubs Created</td><td>" << report.jitStubsCreated << "</td></tr>\n";
    file << "      <tr><td>Watermarks Embedded</td><td>" << report.watermarksEmbedded << "</td></tr>\n";
    file << "    </table>\n";
    
    if (!report.aiObfuscationStrategy.empty()) {
        file << "    <h2>AI Profiler</h2>\n";
        file << "    <p><strong>Strategy:</strong> " << escapeHTML(report.aiObfuscationStrategy) << "</p>\n";
    }
    
    file << "  </div>\n</body>\n</html>\n";
    file.close();
    
    Logger::info("HTML report generated: " + path);
    return true;
}

bool ReportingModule::generatePDF(const ObfuscationReport& report, const std::string& path) {
    // For PDF generation, we'll create a LaTeX file and suggest compilation
    std::string texPath = path + ".tex";
    std::ofstream file(texPath);
    
    if (!file.is_open()) {
        Logger::error("Failed to open PDF report file: " + texPath);
        return false;
    }
    
    file << "\\documentclass{article}\n";
    file << "\\usepackage[utf8]{inputenc}\n";
    file << "\\usepackage{booktabs}\n";
    file << "\\usepackage{graphicx}\n";
    file << "\\usepackage{xcolor}\n";
    file << "\\title{Adaptive Obfuscator Report}\n";
    file << "\\date{" << formatTimestamp(report.timestamp) << "}\n";
    file << "\\begin{document}\n";
    file << "\\maketitle\n\n";
    
    file << "\\section{Summary}\n";
    file << "\\begin{itemize}\n";
    file << "  \\item Obfuscation Passes: " << report.obfuscationPasses << "\n";
    file << "  \\item Total Time: " << report.totalObfuscationTimeMs << " ms\n";
    file << "  \\item Entropy Increase: " << (report.entropyDelta * 100) << "\\%\n";
    file << "\\end{itemize}\n\n";
    
    file << "\\section{Input/Output Comparison}\n";
    file << "\\begin{tabular}{lrrr}\n";
    file << "\\toprule\n";
    file << "Metric & Input & Output & Delta \\\\\n";
    file << "\\midrule\n";
    file << "File Size & " << report.inputFileSize << " & " << report.outputFileSize << " & +" << (report.outputFileSize - report.inputFileSize) << " \\\\\n";
    file << "Functions & " << report.inputFunctionCount << " & " << report.outputFunctionCount << " & +" << (report.outputFunctionCount - report.inputFunctionCount) << " \\\\\n";
    file << "Instructions & " << report.inputInstructionCount << " & " << report.outputInstructionCount << " & +" << (report.outputInstructionCount - report.inputInstructionCount) << " \\\\\n";
    file << "\\bottomrule\n";
    file << "\\end{tabular}\n\n";
    
    file << "\\section{Transformations}\n";
    file << "\\begin{itemize}\n";
    file << "  \\item Bogus Code Insertions: " << report.bogusCodeInsertions << "\n";
    file << "  \\item Fake Loops: " << report.fakeLoopsGenerated << "\n";
    file << "  \\item String Obfuscations: " << report.stringObfuscationCount << "\n";
    file << "  \\item JIT Stubs: " << report.jitStubsCreated << "\n";
    file << "\\end{itemize}\n\n";
    
    file << "\\end{document}\n";
    file.close();
    
    Logger::info("LaTeX report generated: " + texPath);
    Logger::info("To generate PDF, run: pdflatex " + texPath);
    return true;
}

void ReportingModule::updateReport(ObfuscationReport& report, const PassMetrics& metrics) {
    report.passMetrics.push_back(metrics);
}

double ReportingModule::calculateEntropy(const std::vector<uint8_t>& data) {
    if (data.empty()) return 0.0;
    
    std::map<uint8_t, size_t> freq;
    for (uint8_t byte : data) {
        freq[byte]++;
    }
    
    double entropy = 0.0;
    double size = static_cast<double>(data.size());
    
    for (const auto& [byte, count] : freq) {
        double probability = count / size;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy;
}

double ReportingModule::calculateEntropy(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return 0.0;
    }
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();
    
    return calculateEntropy(data);
}

std::string ReportingModule::formatTimestamp(const std::chrono::system_clock::time_point& tp) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string ReportingModule::escapeJSON(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

std::string ReportingModule::escapeHTML(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c;
        }
    }
    return result;
}

} // namespace obfuscator
