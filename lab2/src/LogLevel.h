#pragma once

#include <string>

enum class LogLevel {
    TRACE = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

inline std::string levelToString(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}
