#pragma once

#include <string>
#include <vector>
#include "Logger.h"
#include "Message.h"

struct LMResponse {
    std::string text;
    int promptTokens;
    int completionTokens;
    double seconds;
};

class LMStudioClient {
private:
    Logger logger;

    std::string escape(const std::string& s);
    std::string buildBody(const std::vector<Message>& msgs);
    std::string readString(const std::string& json, size_t pos);
    int readNumber(const std::string& json, const std::string& key);

public:
    LMStudioClient();

    LMResponse send(const std::vector<Message>& msgs);
};
