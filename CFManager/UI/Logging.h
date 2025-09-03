#pragma once
#include <string>

enum class LogLevel {
    Info,
    Success,
    Warning,
    Error
};

class Logger {
public:
    static void Log(LogLevel level, const std::string& msg);

    static void Info(const std::string& msg);
    static void Success(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);

    // Optional: set a file path to mirror console logs to disk
    static void SetLogFile(const std::string& filePath);

    // Write the in-memory ring buffer to a file (for crash diagnostics)
    static void WriteRingToFile(const char* path);
};

// Expose crash-ring writer for use in callers (e.g., top-level exception filter)
extern "C" void Logger_WriteRingToFile(const char* path);

