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

void Logger::log(LogLevel msgLevel, const std::string& message, std::initializer_list<std::string> args) const {
    if (static_cast<int>(msgLevel) < static_cast<int>(level)) {
        return;
    }

    // placeholders are only filled once we know the line gets printed
    std::string msg = "";
    int used = 0;
    for (int i = 0; i < (int)message.size(); i++) {
        if (message[i] != '%') {
            msg += message[i];
        } else if (i + 1 < (int)message.size() && message[i + 1] == '%') {
            msg += '%';
            i++;
        } else if (used < (int)args.size()) {
            msg += args.begin()[used];
            used++;
        } else {
            msg += '%'; // nothing left to put here
        }
    }

    std::string line = formatter->apply(msgLevel, name, msg);
    sink->write(line);

    if (used < (int)args.size()) {
        log(LogLevel::WARN, std::to_string((int)args.size() - used) + " surplus argument(s) ignored");
    }
}

void Logger::trace(const std::string& msg, std::initializer_list<std::string> args) const { log(LogLevel::TRACE, msg, args); }
void Logger::info(const std::string& msg, std::initializer_list<std::string> args) const  { log(LogLevel::INFO,  msg, args); }
void Logger::warn(const std::string& msg, std::initializer_list<std::string> args) const  { log(LogLevel::WARN,  msg, args); }
void Logger::error(const std::string& msg, std::initializer_list<std::string> args) const { log(LogLevel::ERROR, msg, args); }
