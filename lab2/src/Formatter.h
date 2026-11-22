#pragma once

#include <string>
#include "LogLevel.h"

class Formatter {
public:
    static std::string format;

    Formatter(const std::string& fmt);

    std::string apply(LogLevel lvl, const std::string& name, const std::string& message);
};
