#include "Memory.h"

Memory::Memory(int cap) : capacity(cap), logger("Memory") {}

void Memory::add(const Message& m) {
    if ((int)msgs.size() >= capacity) {
        if (!msgs.empty()) {
            msgs.erase(msgs.begin());
            logger.warn("Memory full (capacity %), dropped the oldest message", {std::to_string(capacity)});
        }
    }
    msgs.push_back(m);
    logger.trace("Message added from %, % in memory", {m.role, std::to_string(msgs.size())});
}

int Memory::tier1() { return 6; }
int Memory::tier2() { return 20; }
int Memory::tier3() { return 60; }
