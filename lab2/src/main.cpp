#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include "Agent.h"
#include "ConsoleSink.h"
#include "Formatter.h"
#include "LMStudioClient.h"
#include "Logger.h"
#include "Memory.h"
#include "ParenthesesValidator.h"
#include "Toolbox.h"

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
    Formatter::format = "[%t] [%l] %n: %m";

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
        Formatter::format = formats[i];
        assert(validator.isValid("()") == true);
    }

    Formatter::format = "[%t] [%l] [%r] %n: %m";
    Logger::setLevel(LogLevel::TRACE);
    validator.isValid("()(");

    Formatter shared("%l | %n | %m");
    ConsoleSink sink;
    ParenthesesValidator second(&shared, &sink);
    Logger::setLevel(LogLevel::INFO);
    assert(second.isValid("(())") == true);

    Formatter::format = "[%t] [%l] %n: %m";
    Logger demo("Demo");
    demo.info("Pair found at % and %", {"3", "7"});
    demo.info("Pair % and %", {"3"});
    demo.info("100%% sure");
    demo.info("Only %", {"1", "2", "3"});
    demo.info("name in message: %n");

    std::cout << "All tests passed.\n";
}

int main() {
    Formatter fmt("[%t] [%l] %n: %m");
    ConsoleSink sink;
    Logger::setLevel(LogLevel::TRACE);

    std::string line;
    std::cout << "Memory tier (1, 2 or 3): ";
    std::getline(std::cin, line);
    int capacity = Memory::tier1();
    if (line == "2") {
        capacity = Memory::tier2();
    } else if (line == "3") {
        capacity = Memory::tier3();
    }
    std::cout << "Memory holds " << capacity << " messages. Type quit to stop.\n";

    Memory memory(capacity);
    LMStudioClient client;
    Toolbox toolbox;
    Agent agent("Pip", "a friendly helper who keeps answers short", &memory, &client, &toolbox, &fmt, &sink);

    while (std::getline(std::cin, line)) {
        if (line == "quit") {
            break;
        }
        if (line.empty()) {
            continue;
        }

        // commands for testing, everything else goes to the agent
        if (line[0] == '/') {
            if (line == "/test") {
                runTests();
                Formatter::format = "[%t] [%l] %n: %m";
                Logger::setLevel(LogLevel::TRACE);
            } else if (line == "/level trace") {
                Logger::setLevel(LogLevel::TRACE);
            } else if (line == "/level info") {
                Logger::setLevel(LogLevel::INFO);
            } else if (line == "/level warn") {
                Logger::setLevel(LogLevel::WARN);
            } else if (line == "/level error") {
                Logger::setLevel(LogLevel::ERROR);
            } else {
                std::cout << "Unknown command\n";
            }
            continue;
        }

        std::string answer = agent.chat(line);
        if (answer.empty()) {
            std::cout << "Pip: (no answer, check the log)\n";
        } else {
            std::cout << "Pip: " << answer << "\n";
        }
    }
    return 0;
}
