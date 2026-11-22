#pragma once

#include <string>
#include "Logger.h"

class ParenthesesValidator {
private:
    Logger logger;

public:
    ParenthesesValidator();
    ParenthesesValidator(Formatter* f, ConsoleSink* s);

    bool isValid(const std::string& input);
};
