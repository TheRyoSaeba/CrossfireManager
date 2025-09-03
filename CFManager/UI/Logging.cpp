#include "Logging.h"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <mutex>
#include <fstream>
#include <deque>

namespace {
    std::mutex logMutex;
    std::ofstream logFile;
    std::string logFilePath;
    constexpr size_t kRingMax = 256;
    std::deque<std::string> ring;

    WORD GetColor(LogLevel level) {
        switch (level) {
        case LogLevel::Info:    return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LogLevel::Success: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LogLevel::Warning: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LogLevel::Error:   return FOREGROUND_RED | FOREGROUND_INTENSITY;
        default: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
        }
    }

    const char* GetLevelTag(LogLevel level) {
        switch (level) {
        case LogLevel::Info:    return "[INFO]";
        case LogLevel::Success: return "[+]";
        case LogLevel::Warning: return "[!]";
        case LogLevel::Error:   return "[-]";
        default: return "[?]";
        }
    }

    std::string GetTimestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();
        std::time_t t = system_clock::to_time_t(now);
        struct tm buf;
        localtime_s(&buf, &t);

        char timeStr[16];
        std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &buf);
        return std::string("[") + timeStr + "]";
    }
}

void Logger::Log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(logMutex);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hConsole, &info);

    SetConsoleTextAttribute(hConsole, GetColor(level));
    std::cout << GetTimestamp() << " " << GetLevelTag(level) << " " << msg << std::endl;

    SetConsoleTextAttribute(hConsole, info.wAttributes);

    // Push into in-memory ring for crash diagnostics
    if (ring.size() >= kRingMax) ring.pop_front();
    ring.push_back(GetTimestamp() + std::string(" ") + GetLevelTag(level) + " " + msg);

    // Mirror to file if configured
    if (logFile.is_open()) {
        logFile << ring.back() << std::endl;
        logFile.flush();
    }
}

void Logger::Info(const std::string& msg) { Log(LogLevel::Info, msg); }
void Logger::Success(const std::string& msg) { Log(LogLevel::Success, msg); }
void Logger::Warn(const std::string& msg) { Log(LogLevel::Warning, msg); }
void Logger::Error(const std::string& msg) { Log(LogLevel::Error, msg); }

void Logger::SetLogFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFilePath = filePath;
    if (logFile.is_open()) logFile.close();
    logFile.open(logFilePath, std::ios::out | std::ios::app);
}

void Logger::WriteRingToFile(const char* path) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream f(path, std::ios::out | std::ios::app);
    if (!f.is_open()) return;
    f << "\n=== Crash Ring Dump ===\n";
    for (const auto& line : ring) f << line << '\n';
    f << "=== End Crash Ring Dump ===\n";
}