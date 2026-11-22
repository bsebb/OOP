#pragma once

#include <initializer_list>
#include <string>
#include "ConsoleSink.h"
#include "Formatter.h"
#include "LogLevel.h"

class Logger {
private:
    std::string name;
    Formatter* formatter;
    ConsoleSink* sink;
    bool ownsParts;

public:
    static LogLevel level;

    Logger(const std::string& loggerName);
    Logger(const std::string& loggerName, Formatter* f, ConsoleSink* s);
    ~Logger();

    static void setLevel(LogLevel newLevel);

    void log(LogLevel msgLevel, const std::string& message, std::initializer_list<std::string> args = {}) const;

    void trace(const std::string& msg, std::initializer_list<std::string> args = {}) const;
    void info(const std::string& msg, std::initializer_list<std::string> args = {}) const;
    void warn(const std::string& msg, std::initializer_list<std::string> args = {}) const;
    void error(const std::string& msg, std::initializer_list<std::string> args = {}) const;
};
