#pragma once

#include <string>

class Message {
public:
    std::string role;
    std::string content;

    Message(const std::string& r, const std::string& c);
};
