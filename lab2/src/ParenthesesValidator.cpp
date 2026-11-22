#include <vector>
#include "ParenthesesValidator.h"

ParenthesesValidator::ParenthesesValidator() : logger("Parentheses") {}
// used to take a Logger by value, but copying one now copies the raw pointers too
ParenthesesValidator::ParenthesesValidator(Formatter* f, ConsoleSink* s) : logger("Parentheses", f, s) {}

bool ParenthesesValidator::isValid(const std::string& input) {
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
