#include "Global.h"

void Global::set(size_t index, const EvaValue& value) {
    if (index >= globals.size()) {
        DIE << "GLOBAL " << index << " doesn't exits.";
    }

    globals[index].value = value;
}

void Global::define(const std::string& name) {
    auto index = getGlobalIndex(name);

    if (index != -1) {
        return;
    }

    // auto initialize to 0
    globals.push_back({name, NUMBER(0)});
}

int Global::getGlobalIndex(const std::string& name) {
    if (globals.size() > 0) {
        for (auto itr = globals.begin(); itr != globals.end(); ++itr) {
            if (itr->Name == name) {
                return std::distance(globals.begin(), itr);
            }
        }
    }

    return -1;
}

bool Global::exists(const std::string& name) {
    return getGlobalIndex(name) != -1;
}

GlobalVar Global::get(const size_t index) {
    return globals[index];
}

void Global::addConst(const std::string& name, double value) {
    if (exists(name)) {
        return;
    }

    globals.push_back({name, NUMBER(value)});
}

void Global::addNativeFunction(const std::string& name,
                               std::function<void()> fn,
                               size_t arity) {
    if (exists(name)) {
        return;
    }

    globals.push_back({name, ALLOC_NATIVE(fn, name, arity)});
}