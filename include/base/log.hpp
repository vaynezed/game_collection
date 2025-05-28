#pragma once
#include <cstdio>
#include <string>

class Log {
private:
    FILE* log_file;

public:
    void debug(std::string& msg);
};
