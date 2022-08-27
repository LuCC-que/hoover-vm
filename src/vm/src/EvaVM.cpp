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
    auto ast = parser->parse("(begin " + program + ")");
    log(ast.number);

    // 2. Compaile Program to Eva bytecode
    compiler->compile(ast);
    // constants.push_back(ALLOC_STRING("Hello, "));
    // constants.push_back(ALLOC_STRING("World!"));
    // code = {OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT};

    fn = compiler->getMainFunction();

    // Set instruction pointer to the begining
    ip = &fn->co->code[0];
    sp = &stack[0];

    bp = sp;

    compiler->disassembleByteCode();

    return eval();
}

EvaValue EvaVM::eval() {
    for (;;) {
        auto opcode = READ_BYTE();
        DebugDumpStack(opcode);
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

            case OP_GET_GLOBAL: {
                auto globalIndex = READ_BYTE();
                push(global->get(globalIndex).value);
                break;
            }

            case OP_SET_GLOBAL: {
                auto globalIndex = READ_BYTE();
                // result on the stack, in this case
                // usually the top

                // opt: pop the global automatically
                auto value = pop();
                global->set(globalIndex, value);
                break;
            }

            case OP_POP:
                pop();
                break;

            case OP_GET_LOCAL: {
                auto localIndex = READ_BYTE();
                if (localIndex < 0 || localIndex >= stack.size()) {
                    DIE << "OP_GET_LOCAL: invalide variable index: " << (int)localIndex;
                }

                push(bp[localIndex]);
                break;
            }

            case OP_SET_LOCAL: {
                auto localIndex = READ_BYTE();
                auto value = peek(0);
                if (localIndex < 0 || localIndex >= stack.size()) {
                    DIE << "OP_GET_LOCAL: invalide variable index: " << (int)localIndex;
                }
                bp[localIndex] = value;
                break;
            }
            case OP_GET_CELL: {
                auto cellIndex = READ_BYTE();
                push(fn->cells[cellIndex]->value);
                break;
            }

            case OP_LOAD_CELL: {
                auto cellIndex = READ_BYTE();
                push(CELL(fn->cells[cellIndex]));
                break;
            }

            case OP_SET_CELL: {
                auto cellIndex = READ_BYTE();
                auto value = peek(0);

                if (fn->cells.size() <= cellIndex) {
                    fn->cells.push_back(AS_CELL(ALLOC_CELL(value)));
                } else {
                    fn->cells[cellIndex]->value = value;
                }
                break;
            }

            case OP_MAKE_FUNCTION: {
                auto co = AS_CODE(pop());
                auto cellsCount = READ_BYTE();

                auto fnValue = ALLOC_FUNCTION(co);
                auto fn = AS_FUNCTION(fnValue);

                for (auto i = 0; i < cellsCount; ++i) {
                    fn->cells.push_back(AS_CELL(pop()));
                }
                push(fnValue);
                break;
            }

            case OP_SCOPE_EXIT: {
                auto count = READ_BYTE();

                // save the result to the top
                *(sp - 1 - count) = peek(0);
                popN(count);
                break;
            }

            case OP_CALL: {
                auto argsCount = READ_BYTE();
                EvaValue fnValue = peek(argsCount);  // get the function

                // Native function
                if (IS_NATIVE(fnValue)) {
                    AS_NATIVE(fnValue)->function();
                    auto result = pop();
                    popN(argsCount + 1);
                    push(result);
                    break;
                }

                // user defined function:
                auto callee = AS_FUNCTION(fnValue);

                // Save the current frame, wait for return
                callStack.push(Frame{ip, bp, fn});

                // set to the current calling function
                fn = callee;

                fn->cells.resize(fn->co->freeCount);

                // set the base pointer for the callee as a frame
                bp = sp - argsCount - 1;

                // jump to the initial instruction of the function
                ip = &callee->co->code[0];

                break;
            }
            case OP_RETURN: {
                auto callerFrame = callStack.top();

                ip = callerFrame.ra;
                bp = callerFrame.bp;
                fn = callerFrame.fn;

                callStack.pop();
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

EvaValue EvaVM::peek(size_t offset) {
    if (stack.size() == 0) {
        DIE << "peek(): empty stack.\n";
    }

    return *(sp - 1 - offset);
}

void EvaVM::push(const EvaValue &value) {
    if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
        std::cout << "test" << std::endl;
        DIE << "push(): Stack overflow.\n";
    }

    *sp = value;
    sp++;
}

void EvaVM::setGlobalVariables() {
    global->addNativeFunction(
        "native-square",
        [&]() {
            auto x = AS_NUMBER(peek(0));
            push(NUMBER(x * x));
        },
        1);

    global->addNativeFunction(
        "sum",
        [&]() {
            auto v2 = AS_NUMBER(peek(0));
            auto v1 = AS_NUMBER(peek(1));
            push(NUMBER(v1 + v2));
        },
        2);

    global->addConst("VERSION", 1);
}

void EvaVM::popN(size_t count) {
    if (stack.size() == 0) {
        DIE << "popN(): empty stack.\n";
    }

    sp -= count;
}

void EvaVM::DebugDumpStack(uint8_t op) {
    std::cout << "\n========= Stack ===========\n";
    std::cout << ">> op: OP_" << opcodeToString(op) << std::endl;
    if (sp == stack.begin()) {
        std::cout << "(empty)";
    }

    auto csp = sp - 1;

    while (csp >= stack.begin()) {
        std::cout << *csp-- << "\n";
    }

    std::cout << "\n";
}