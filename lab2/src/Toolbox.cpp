#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "Toolbox.h"

Toolbox::Toolbox() : logger("Toolbox") {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

std::string Toolbox::getTime() {
    std::time_t now = std::time(nullptr);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&now));
    return std::string(buffer);
}

std::string Toolbox::getDate() {
    std::time_t now = std::time(nullptr);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&now));
    return std::string(buffer);
}

std::string Toolbox::calculator(const std::string& input) {
    std::string expr = "";
    for (int i = 0; i < (int)input.size(); i++) {
        if (input[i] != ' ') {
            expr += input[i];
        }
    }

    // start at 1 so a minus sign on the first number is not taken as the operator
    size_t pos = expr.find_first_of("+-*/x", 1);
    if (pos == std::string::npos) {
        logger.error("Calculator could not find an operator in '%'", {input});
        return "";
    }

    char op = expr[pos];
    double a = 0;
    double b = 0;
    try {
        a = std::stod(expr.substr(0, pos));
        b = std::stod(expr.substr(pos + 1));
    } catch (std::exception& e) {
        logger.error("Calculator could not read numbers from '%'", {input});
        return "";
    }

    double result = 0;
    if (op == '+') {
        result = a + b;
    } else if (op == '-') {
        result = a - b;
    } else if (op == '*' || op == 'x') { // the model sometimes writes 12x7
        result = a * b;
    } else if (op == '/') {
        if (b == 0) {
            logger.error("Calculator was asked to divide by zero");
            return "";
        }
        result = a / b;
    }

    std::ostringstream out;
    if (result == std::floor(result) && std::fabs(result) < 1e15) {
        out << (long long)result;
    } else {
        out << result;
    }
    return out.str();
}

std::string Toolbox::rollDice(const std::string& input) {
    int sides = std::atoi(input.c_str());
    if (sides < 2) {
        sides = 6;
    }
    return std::to_string(std::rand() % sides + 1);
}

std::string Toolbox::run(const std::string& tool, const std::string& input) {
    logger.info("Running tool % with input '%'", {tool, input});
    if (tool == "get_time") {
        return getTime();
    } else if (tool == "get_date") {
        return getDate();
    } else if (tool == "calculator") {
        return calculator(input);
    } else if (tool == "roll_dice") {
        return rollDice(input);
    }
    logger.error("Unknown tool: %", {tool});
    return "";
}
