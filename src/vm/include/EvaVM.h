/**
 * @file EvaVM.h
 *
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-08-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef EvaVM_h
#define EvaVM_h

#include <array>
#include <string>
#include <vector>

#include "../../compiler/include/EvaCompiler.h"
#include "../../parser/include/EvaParser.h"
#include "EvaValue.h"
using syntax::EvaParser;

#define READ_BYTE() *ip++
#define STACK_LIMIT 512
#define GET_CONST() co->constants[READ_BYTE()]

#define BINARY_OP(op)                \
    do {                             \
        auto op2 = AS_NUMBER(pop()); \
        auto op1 = AS_NUMBER(pop()); \
        push(NUMBER(op1 op op2));    \
    } while (false)  // only execute once

#define BINARY_ADD()                               \
    auto op2 = pop();                              \
    auto op1 = pop();                              \
    if (IS_NUMBER(op1) && IS_NUMBER(op2)) {        \
        auto v1 = AS_NUMBER(op1);                  \
        auto v2 = AS_NUMBER(op2);                  \
        push(NUMBER(v1 + v2));                     \
    } else if (IS_STRING(op1) && IS_STRING(op2)) { \
        auto s1 = AS_CPPSTRING(op1);               \
        auto s2 = AS_CPPSTRING(op2);               \
        push(ALLOC_STRING(s1 + s2));               \
    }

class EvaVM {
   private:
    /* data */
   public:
    EvaVM(/* args */)
        : parser(std::make_unique<EvaParser>()),
          compiler(std::make_unique<EvaCompiler>()){};
    /**
     * @brief
     *
     * @param value
     *
     * pushes a value onto the stack
     */
    void push(const EvaValue &value);

    EvaValue pop();

    // the start
    EvaValue exec(const std::string &program);

    /**
     * @brief
     * Main eval loop
     *
     */

    EvaValue eval();

    /**
     * @brief
     * Instruction pointer (as well as PC)
     */

    uint8_t *ip;

    /**
     * @brief
     * Stack pointer
     */
    EvaValue *sp;

    /**
     * @brief
     * Operand Stack
     */
    std::array<EvaValue, STACK_LIMIT> stack;

    /**
     * @brief
     * Constant pool
     */
    std::vector<EvaValue> constants;

    /**
     * @brief
     * Parser
     */

    std::unique_ptr<EvaParser> parser;

    std::unique_ptr<EvaCompiler> compiler;

    CodeObject *co;
};

#endif