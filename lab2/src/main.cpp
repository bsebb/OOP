#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include "Logger.h"
#include "ParenthesesValidator.h"

struct TestCase {
    std::string input;
    bool expected;
};

void runTests() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    ParenthesesValidator validator;

    std::vector<TestCase> cases = {
        {"()", true},
        {")((()))", false},
        {"(", false},
        {"(())((()())())", true},
        {"", true},
        {"((()))", true},
        {"(()", false},
        {"())", false},
        {")(", false}
    };

    Logger::setLevel(LogLevel::TRACE);
    Logger::setFormat("[%t] [%l] %n: %m");

    for (int i = 0; i < (int)cases.size(); i++) {
        bool result = validator.isValid(cases[i].input);
        assert(result == cases[i].expected);
    }

    Logger::setLevel(LogLevel::INFO);
    assert(validator.isValid("()") == true);

    Logger::setLevel(LogLevel::ERROR);
    assert(validator.isValid("(") == false);

    std::vector<std::string> formats = {
        "[%l] %m",
        "%n: %m",
        "[%t] %m",
        "%l - %n - %m"
    };

    for (int i = 0; i < (int)formats.size(); i++) {
        Logger::setFormat(formats[i]);
        assert(validator.isValid("()") == true);
    }

    Logger::setFormat("[%t] [%l] [%r] %n: %m");
    Logger::setLevel(LogLevel::TRACE);
    validator.isValid("()(");

    std::cout << "All tests passed.\n";
}

int main() {
    runTests();
    return 0;
}
