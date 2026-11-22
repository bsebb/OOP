#pragma once

#include <string>
#include "Logger.h"

class Toolbox {
private:
    Logger logger;

    std::string getTime();
    std::string getDate();
    std::string calculator(const std::string& input);
    std::string rollDice(const std::string& input);

public:
    Toolbox();

    std::string run(const std::string& tool, const std::string& input);
};
