#include <iostream>
#include "ConsoleSink.h"

void ConsoleSink::write(const std::string& line) {
    std::cout << line << '\n';
}
