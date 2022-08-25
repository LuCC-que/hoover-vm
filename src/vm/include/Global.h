#ifndef Global_h
#define Global_h

#include <vector>

#include "EvaValue.h"
struct GlobalVar {
    std::string Name;
    EvaValue value;
};

struct Global {
    std::vector<GlobalVar> globals;

    void set(size_t index, const EvaValue& value);

    void define(const std::string& name);
    int getGlobalIndex(const std::string& name);

    void addConst(const std::string& name, double value);

    bool exists(const std::string& name);

    GlobalVar get(const size_t index);
};

#endif