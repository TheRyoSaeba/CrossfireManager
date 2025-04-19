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
};

