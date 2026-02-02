#ifndef STATE_HPP
#define STATE_HPP

#include <string>
#include <vector>

struct State
{
    std::vector<std::string> items;
    const char* version = "0.1";
};

#endif