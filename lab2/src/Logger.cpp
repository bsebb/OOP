#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include "Logger.h"

LogLevel Logger::level = LogLevel::INFO;
std::string Logger::format = "[%t] [%l] %n: %m";

static std::string getRandomMessage(LogLevel lvl) {
    static const std::vector<std::string> tracePool = {
        "Stepping into execution branch",
        "Inspecting inner parser state",
        "Trace checkpoint reached"
    };
    static const std::vector<std::string> infoPool = {
        "Operation completed",
        "Everything looks good",
        "Here is your result"
    };
    static const std::vector<std::string> warnPool = {
        "Caution advised",
        "Potential anomaly detected",
        "Suspicious pattern encountered"
    };
    static const std::vector<std::string> errorPool = {
        "Operation aborted",
        "Invalid input state encountered",
        "Critical validation issue found"
    };

    if (lvl == LogLevel::TRACE) {
        return tracePool[std::rand() % tracePool.size()];
    } else if (lvl == LogLevel::INFO) {
        return infoPool[std::rand() % infoPool.size()];
    } else if (lvl == LogLevel::WARN) {
        return warnPool[std::rand() % warnPool.size()];
    }
    return errorPool[std::rand() % errorPool.size()];
}

static std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm tmNow = *std::localtime(&now);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tmNow);
    return std::string(buffer);
}

static std::string replaceAll(std::string text, const std::string& token, const std::string& value) {
    size_t pos = 0;
    while ((pos = text.find(token, pos)) != std::string::npos) {
        text.replace(pos, token.length(), value);
        pos += value.length();
    }
    return text;
}

Logger::Logger(const std::string& loggerName) : name(loggerName) {}

void Logger::setLevel(LogLevel newLevel) {
    level = newLevel;
}

void Logger::setFormat(const std::string& newFormat) {
    format = newFormat;
}

void Logger::log(LogLevel msgLevel, const std::string& message) const {
    if (static_cast<int>(msgLevel) < static_cast<int>(level)) {
        return;
    }

    std::string output = format;
    output = replaceAll(output, "%t", getCurrentTime());
    output = replaceAll(output, "%l", levelToString(msgLevel));
    output = replaceAll(output, "%n", name);
    output = replaceAll(output, "%m", message);
    output = replaceAll(output, "%r", getRandomMessage(msgLevel));

    std::cout << output << '\n';
}

void Logger::trace(const std::string& msg) const { log(LogLevel::TRACE, msg); }
void Logger::info(const std::string& msg) const  { log(LogLevel::INFO,  msg); }
void Logger::warn(const std::string& msg) const  { log(LogLevel::WARN,  msg); }
void Logger::error(const std::string& msg) const { log(LogLevel::ERROR, msg); }
