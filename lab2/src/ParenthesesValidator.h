#pragma once

#include <string>
#include "Logger.h"

class ParenthesesValidator {
private:
    Logger logger;

public:
    ParenthesesValidator();
    ParenthesesValidator(Logger customLogger);

    bool isValid(const std::string& input);
};
