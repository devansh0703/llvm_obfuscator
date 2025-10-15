#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

namespace obfuscator {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void init(const std::string& logFile = "");
    static void shutdown();
    
    static void setLevel(LogLevel level);
    static void setConsoleOutput(bool enable);
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);
    
    static void log(LogLevel level, const std::string& message);
    
private:
    static void writeLog(LogLevel level, const std::string& message);
    static std::string levelToString(LogLevel level);
    static std::string getCurrentTime();
    
    static LogLevel currentLevel_;
    static bool consoleOutput_;
    static std::ofstream logFile_;
    static std::mutex mutex_;
};

} // namespace obfuscator

#endif // UTILS_LOGGER_H
