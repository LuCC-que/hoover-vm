#ifndef EvaCompiler_h
#define EvaCompiler_h

#include <map>
#include <string>

#include "../../byteCode/include/OpCode.h"
#include "../../disassembler/include/EvaDisassembler.h"
#include "../../logger/include/Logger.h"
#include "../../parser/include/EvaParser.h"
#include "../../vm/include/EvaValue.h"

#define ALLOC_CONST(tester, converter, allocator, value)                        \
    for (auto itr = co->constants.begin(); itr != co->constants.end(); ++itr) { \
        if (!tester(*itr)) {                                                    \
            continue;                                                           \
        }                                                                       \
                                                                                \
        if (converter(*itr) == value) {                                         \
            return std::distance(co->constants.begin(), itr);                   \
        }                                                                       \
    }                                                                           \
    co->addConst(allocator(value));

#define GEN_BINARY_OP(op) \
    gen(exp.list[1]);     \
    gen(exp.list[2]);     \
    emit(op);

class EvaCompiler {
   private:
    void emit(uint8_t code);
    size_t numericConstIdx(double value);
    size_t stringConstIdx(const std::string& value);
    size_t booleanConstIdx(const bool value);
    size_t getOffset();
    void patchJumpAddress(size_t offset, uint16_t value);
    void writeByteOffset(size_t offset, uint8_t value);
    std::shared_ptr<Global> global;
    std::unique_ptr<EvaDisassembler> disassembler;
    void scopeEnter();
    void scopeExit();
    bool isGlobalScope();
    bool isDeclaration(const Exp& exp);
    bool isVarDeclaration(const Exp& exp);
    bool isTaggedList(const Exp& exp,
                      const std::string& tag);
    size_t getVarsCountOnScopeExit();
    EvaValue creatCodeObjectValue(const std::string& name, size_t arity = 0);
    bool isBlock(const Exp& exp);
    bool isFunctionBody();

    FunctionObject* main;

   public:
    EvaCompiler(std::shared_ptr<Global> global)
        : global(global),
          disassembler(std::make_unique<EvaDisassembler>(global)){};

    void disassembleByteCode();

    CodeObject* compile(const Exp& exp);

    void gen(const Exp& exp);

    CodeObject* co;
    std::vector<CodeObject*> codeObjects_;
};

static std::map<std::string, uint8_t> compareOps_{
    {"<", 0},
    {">", 1},
    {"==", 2},
    {">=", 3},
    {"<=", 4},
    {"!=", 5},
};

#endif