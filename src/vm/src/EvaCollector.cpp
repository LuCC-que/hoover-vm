#include "EvaCollector.h"

void EvaCollector::gc(const std::set<Traceable *> &roots) {
    mark(roots);
    sweep();
}

void EvaCollector::mark(const std::set<Traceable *> &roots) {
    std::vector<Traceable *> worklist(roots.begin(), roots.end());

    while (!worklist.empty()) {
        auto object = worklist.back();
        worklist.pop_back();

        if (!object->marked) {
            object->marked = true;
            for (auto &p : getPointers(object)) {
                worklist.push_back(p);
            }
        }
    }
}

std::set<Traceable *> EvaCollector::getPointers(const Traceable *object) {
    std::set<Traceable *> pointers;
    const auto evaValue = OBJECT((Object *)object);

    if (IS_FUNCTION(evaValue)) {
        const auto fn = AS_FUNCTION(evaValue);
        for (auto &cell : fn->cells) {
            pointers.insert((Traceable *)cell);
        }
    }

    if (IS_INSTANCE(evaValue)) {
        auto instance = AS_INSTANCE(evaValue);
        for (const auto &prop : instance->properties) {
            if (IS_OBJECT(prop.second)) {
                pointers.insert((Traceable *)AS_OBJECT(prop.second));
            }
        }
    }

    return pointers;
}

void EvaCollector::sweep() {
    auto it = Traceable::objects.begin();
    while (it != Traceable::objects.end()) {
        auto object = (Traceable *)*it;
        if (object->marked) {
            object->marked = false;
            ++it;
        } else {
            it = Traceable::objects.erase(it);
            delete object;
        }
    }
}