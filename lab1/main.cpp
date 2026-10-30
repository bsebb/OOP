#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cassert>

enum class LogLevel {
    TRACE = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

inline std::string levelToString(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

inline std::string getRandomMessage(LogLevel lvl) {
    static const std::vector<std::string> tracePool = {
        "Stepping into execution branch",
        "Inspecting inner parser state",
        "Trace checkpoint reached"
    };
    static const std::vector<std::string> infoPool = {
        "Operation completed",
        "Everything looks good",
        "Here is your result"
    };
    static const std::vector<std::string> warnPool = {
        "Caution advised",
        "Potential anomaly detected",
        "Suspicious pattern encountered"
    };
    static const std::vector<std::string> errorPool = {
        "Operation aborted",
        "Invalid input state encountered",
        "Critical validation issue found"
    };

    const auto& pool = (lvl == LogLevel::TRACE) ? tracePool :
                       (lvl == LogLevel::INFO)  ? infoPool  :
                       (lvl == LogLevel::WARN)  ? warnPool  : errorPool;

    return pool[std::rand() % pool.size()];
}

inline std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm tmNow = *std::localtime(&now);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tmNow);
    return std::string(buffer);
}

inline std::string replaceAll(std::string text, const std::string& token, const std::string& value) {
    size_t pos = 0;
    while ((pos = text.find(token, pos)) != std::string::npos) {
        text.replace(pos, token.length(), value);
        pos += value.length();
    }
    return text;
}

class Logger {
private:
    std::string name;

public:
    static LogLevel level;
    static std::string format;

    explicit Logger(std::string loggerName) : name(std::move(loggerName)) {}

    static void setLevel(LogLevel newLevel) {
        level = newLevel;
    }

    static void setFormat(std::string newFormat) {
        format = std::move(newFormat);
    }

    void log(LogLevel msgLevel, const std::string& message) const {
        if (static_cast<int>(msgLevel) < static_cast<int>(level)) {
            return;
        }

        std::string output = format;
        output = replaceAll(output, "%t", getCurrentTime());
        output = replaceAll(output, "%l", levelToString(msgLevel));
        output = replaceAll(output, "%n", name);
        output = replaceAll(output, "%m", message);
        output = replaceAll(output, "%r", getRandomMessage(msgLevel));

        std::cout << output << '\n';
    }

    void trace(const std::string& msg) const { log(LogLevel::TRACE, msg); }
    void info(const std::string& msg) const  { log(LogLevel::INFO,  msg); }
    void warn(const std::string& msg) const  { log(LogLevel::WARN,  msg); }
    void error(const std::string& msg) const { log(LogLevel::ERROR, msg); }
};

LogLevel Logger::level = LogLevel::INFO;
std::string Logger::format = "[%t] [%l] %n: %m";

class ParenthesesValidator {
private:
    Logger logger;

public:
    ParenthesesValidator() : logger("Parentheses") {}
    explicit ParenthesesValidator(Logger customLogger) : logger(std::move(customLogger)) {}

    bool isValid(const std::string& input) {
        std::vector<int> stack;

        for (int i = 0; i < static_cast<int>(input.length()); ++i) {
            char ch = input[i];
            if (ch == '(') {
                stack.push_back(i);
                logger.trace("Found opening parenthesis at index " + std::to_string(i));
            } else if (ch == ')') {
                if (stack.empty()) {
                    logger.warn("Unmatched closing parenthesis at index " + std::to_string(i));
                    logger.error("Validation failed for '" + input + "': closing parenthesis at index " +
                                 std::to_string(i) + " has no open match");
                    logger.info("Validation result for '" + input + "': false");
                    return false;
                }
                int matchedIdx = stack.back();
                stack.pop_back();
                logger.info("Matched closing parenthesis at index " + std::to_string(i) +
                            " with opening parenthesis at index " + std::to_string(matchedIdx));
            }
        }

        if (!stack.empty()) {
            logger.error("Validation failed for '" + input + "': " + std::to_string(stack.size()) +
                         " opening parenthesis/es unclosed");
            logger.info("Validation result for '" + input + "': false");
            return false;
        }

        logger.info("Validation result for '" + input + "': true");
        return true;
    }
};

void runTests() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    ParenthesesValidator validator;

    struct TestCase {
        std::string input;
        bool expected;
    };

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

    for (const auto& tc : cases) {
        bool result = validator.isValid(tc.input);
        assert(result == tc.expected);
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

    for (const auto& fmt : formats) {
        Logger::setFormat(fmt);
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
