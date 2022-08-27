#include "Scope.h"

void Scope::addLocal(const std::string& name) {
    allocInfo[name] = scopeType == ScopeType::GLOBAL
                          ? AllocType::GLOBAL
                          : AllocType::LOCAL;
}

void Scope::addCell(const std::string& name) {
    cells.insert(name);
    allocInfo[name] = AllocType::CELL;
}

void Scope::addFree(const std::string& name) {
    free.insert(name);
    allocInfo[name] = AllocType::CELL;
}

// alter the potential cell from local
void Scope::maybePromote(const std::string& name) {
    auto initAllocType = scopeType == ScopeType::GLOBAL
                             ? AllocType::GLOBAL
                             : AllocType::LOCAL;

    if (allocInfo.count(name) != 0) {
        initAllocType = allocInfo[name];
    }

    auto [ownerScope, allocType] = resolve(name, initAllocType);

    allocInfo[name] = allocType;

    if (allocType == AllocType::CELL) {
        promote(name, ownerScope);
    }
}

void Scope::promote(const std::string& name, Scope* ownerScope) {
    ownerScope->addCell(name);

    auto scope = this;
    while (scope != ownerScope) {
        scope->addFree(name);
        scope = scope->parent.get();
    }
}

std::pair<Scope*, AllocType> Scope::resolve(
    const std::string& name,
    AllocType allocType) {
    if (allocInfo.count(name) != 0) {
        return std::make_pair(this, allocType);
    }

    if (scopeType == ScopeType::FUNCTION) {
        allocType = AllocType::CELL;
    }

    if (parent == nullptr) {
        DIE << "[Scope] Reference error: " << name << " is not defined.";
    }

    if (parent->scopeType == ScopeType::GLOBAL) {
        allocType = AllocType::GLOBAL;
    }

    return parent->resolve(name, allocType);
}

// void analyze(const Exp& exp,
//              std::shared_ptr<Scope> scope,
//              std::map<const Exp*, std::shared_ptr<Scope>>& scopeInfo_) {
// }