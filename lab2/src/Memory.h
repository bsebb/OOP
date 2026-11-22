#pragma once

#include <vector>
#include "Logger.h"
#include "Message.h"

class Memory {
private:
    int capacity;
    Logger logger;

public:
    std::vector<Message> msgs;

    Memory(int cap);

    void add(const Message& m);

    static int tier1();
    static int tier2();
    static int tier3();
};
