#ifndef Scope_h
#define Scope_h

#include <map>
#include <memory>
#include <set>
#include <string>

#include "../../byteCode/include/OpCode.h"
#include "../../logger/include/Logger.h"

enum class ScopeType {
    GLOBAL,
    FUNCTION,
    BLOCK,
    CLASS,
};

enum class AllocType {
    GLOBAL,
    LOCAL,
    CELL,
};

struct Scope {
    Scope(ScopeType type, std::shared_ptr<Scope> parent)
        : scopeType(type), parent(parent) {}

    void addLocal(const std::string& name);

    void addCell(const std::string& name);

    void addFree(const std::string& name);

    void maybePromote(const std::string& name);

    void promote(const std::string& name, Scope* ownerScope);

    uint8_t getNameGetter(const std::string& name) const;

    uint8_t getNameSetter(const std::string& name) const;

    std::pair<Scope*, AllocType> resolve(
        const std::string& name,
        AllocType allocType);

    /* data */
    ScopeType scopeType;

    /**
     * @brief
     * parent scope
     */
    std::shared_ptr<Scope> parent;

    /**
     * @brief
     * Allocation info
     */
    std::map<std::string, AllocType> allocInfo;

    /**
     * @brief
     * Set of free vars
     */
    std::set<std::string> free;

    /**
     * @brief
     * Set of own cells
     */
    std::set<std::string> cells;
};

// void analyze(const Exp& exp, std::shared_ptr<Scope> scope);
#endif