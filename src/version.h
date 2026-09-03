#ifndef VERSION_H
#define VERSION_H

#include <iostream>

#define MINIMSYS_NAME "minimsys"
#define MINIMSYS_VERSION "v0.2.0"

inline void printVersion()
{
    std::cout
        << MINIMSYS_NAME
        << " "
        << MINIMSYS_VERSION
        << "\n";
}

#endif
