#include "Logger.h"

LogLevel Logger::level = LogLevel::INFO;

// format is static so passing it back in keeps whatever is set right now
Logger::Logger(const std::string& loggerName) : name(loggerName) {
    formatter = new Formatter(Formatter::format);
    sink = new ConsoleSink();
    ownsParts = true;
}

Logger::Logger(const std::string& loggerName, Formatter* f, ConsoleSink* s) : name(loggerName) {
    formatter = f;
    sink = s;
    ownsParts = false;
}

Logger::~Logger() {
    if (ownsParts) {
        delete formatter;
        delete sink;
    }
}

void Logger::setLevel(LogLevel newLevel) {
    level = newLevel;
}

void Logger::log(LogLevel msgLevel, const std::string& message) const {
    if (static_cast<int>(msgLevel) < static_cast<int>(level)) {
        return;
    }

    std::string line = formatter->apply(msgLevel, name, message);
    sink->write(line);
}

void Logger::trace(const std::string& msg) const { log(LogLevel::TRACE, msg); }
void Logger::info(const std::string& msg) const  { log(LogLevel::INFO,  msg); }
void Logger::warn(const std::string& msg) const  { log(LogLevel::WARN,  msg); }
void Logger::error(const std::string& msg) const { log(LogLevel::ERROR, msg); }
