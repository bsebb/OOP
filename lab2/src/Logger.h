#pragma once

#include <string>
#include "LogLevel.h"

class Logger {
private:
    std::string name;

public:
    static LogLevel level;
    static std::string format;

    Logger(const std::string& loggerName);

    static void setLevel(LogLevel newLevel);
    static void setFormat(const std::string& newFormat);

    void log(LogLevel msgLevel, const std::string& message) const;

    void trace(const std::string& msg) const;
    void info(const std::string& msg) const;
    void warn(const std::string& msg) const;
    void error(const std::string& msg) const;
};
