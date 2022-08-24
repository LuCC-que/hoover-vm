#include "EvaVM.h"

#include <iostream>

#include "../../byteCode/include/OpCode.h"
#include "../../compiler/include/EvaCompiler.h"
#include "../../logger/include/Logger.h"
#include "../../parser/include/EvaParser.h"
#include "EvaValue.h"

using syntax::EvaParser;

EvaValue EvaVM::exec(const std::string &program) {
    // 1. Parse the program
    auto ast = parser->parse(program);
    log(ast.number);

    // 2. Compaile Program to Eva bytecode
    co = compiler->compile(ast);
    std::cout << "where?" << std::endl;
    // constants.push_back(ALLOC_STRING("Hello, "));
    // constants.push_back(ALLOC_STRING("World!"));
    // code = {OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT};

    // Set instruction pointer to the begining
    ip = &co->code[0];
    std::cout << "where?" << std::endl;
    sp = &stack[0];

    compiler->disassembleByteCode();

    return eval();
}

EvaValue EvaVM::eval() {
    for (;;) {
        auto opcode = READ_BYTE();
        switch (opcode) {
            case OP_HALT:
                return pop();

            case OP_CONST:
                // auto constIndex = READ_BYTE();
                // auto constant = constants[constIndex];
                // constants[READ_BYTE()]
                EvaVM::push(GET_CONST());
                break;

            // Math ops
            case OP_ADD: {
                auto op2 = pop();
                auto op1 = pop();
                if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                    auto v1 = AS_NUMBER(op1);
                    auto v2 = AS_NUMBER(op2);
                    push(NUMBER(v1 + v2));
                } else if (IS_STRING(op1) && IS_STRING(op2)) {
                    auto s1 = AS_CPPSTRING(op1);
                    auto s2 = AS_CPPSTRING(op2);
                    push(ALLOC_STRING(s1 + s2));
                }
                break;
            }

            case OP_SUB: {
                BINARY_OP(-);
                break;
            }

            case OP_MUL: {
                BINARY_OP(*);
                break;
            }

            case OP_DIV: {
                BINARY_OP(/);
                break;
            }

            //-----------------
            // Comparison:
            case OP_COMPARE: {
                auto op = READ_BYTE();
                auto op2 = pop();
                auto op1 = pop();

                if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                    auto v1 = AS_NUMBER(op1);
                    auto v2 = AS_NUMBER(op2);
                    COMPARE_VALUES(op, v1, v2);
                }

                else if (IS_STRING(op1) && IS_STRING(op2)) {
                    auto s1 = AS_STRING(op1);
                    auto s2 = AS_STRING(op2);
                    COMPARE_VALUES(op, s1, s2);
                }
                break;
            }

            case OP_JMP_IF_FALSE: {
                auto cond = AS_BOOLEAN(pop());
                auto address = READ_SHORT();

                if (!cond) {
                    ip = TO_ADDRESS(address);
                }
                break;
            }

            case OP_JUMP: {
                ip = TO_ADDRESS(READ_SHORT());
                break;
            }

            default:
                DIE << "Unknown Opcode: " << std::hex << opcode;
        }
    }
}

EvaValue EvaVM::pop() {
    if (sp == stack.begin()) {
        DIE << "pop(): empty stack.\n";
    }
    --sp;
    return *sp;
}

void EvaVM::push(const EvaValue &value) {
    if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
        std::cout << "test" << std::endl;
        DIE << "push(): Stack overflow.\n";
    }

    *sp = value;
    sp++;
}
