#pragma once

#include <string>
#include "ConsoleSink.h"
#include "Formatter.h"
#include "LMStudioClient.h"
#include "Logger.h"
#include "Memory.h"
#include "Toolbox.h"

class Agent {
private:
    std::string name;
    std::string personality;
    Memory* memory;
    LMStudioClient* client;
    Toolbox* toolbox;
    Logger logger;
    int toolCalls;

    std::string systemMessage();
    bool findTool(const std::string& text, std::string& tool, std::string& input);
    std::string jsonValue(const std::string& json, const std::string& key, bool& found);

public:
    Agent(const std::string& n, const std::string& p, Memory* m, LMStudioClient* c, Toolbox* t,
          Formatter* f, ConsoleSink* s);

    std::string chat(const std::string& question);
};
