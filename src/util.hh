#ifndef UTIL_HH
#define UTIL_HH

#include <iostream>
#include <string>
#include <cstdlib>

namespace util {
    [[noreturn]] inline void error (const std::string& message)
    {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

#endif
