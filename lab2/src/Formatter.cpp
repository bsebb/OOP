#include <cstdlib>
#include <ctime>
#include <vector>
#include "Formatter.h"

std::string Formatter::format = "[%t] [%l] %n: %m";

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

Formatter::Formatter(const std::string& fmt) {
    format = fmt;
}

std::string Formatter::apply(LogLevel lvl, const std::string& name, const std::string& message) {
    std::string output = format;
    output = replaceAll(output, "%t", getCurrentTime());
    output = replaceAll(output, "%l", levelToString(lvl));
    output = replaceAll(output, "%n", name);
    output = replaceAll(output, "%r", getRandomMessage(lvl));
    // %m goes last so a message with %n or %t in it is printed as is
    output = replaceAll(output, "%m", message);
    return output;
}
