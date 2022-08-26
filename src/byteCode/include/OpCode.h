#ifndef OpCode_h
#define OpCode_h

#include <string>

#include "../../logger/include/Logger.h"
/**
 * @brief
 * Stops the program
 */

// Stops the program
#define OP_HALT 0x00

// Pushes a const onto the stack
#define OP_CONST 0x01

// Math instruction
#define OP_ADD 0x02
#define OP_SUB 0x03
#define OP_MUL 0x04
#define OP_DIV 0x05

/**
 * @brief
 * Comparsion
 */
#define OP_COMPARE 0x06

#define OP_JMP_IF_FALSE 0x07

#define OP_JUMP 0x08

#define OP_GET_GLOBAL 0x09

#define OP_SET_GLOBAL 0x10

// pop value from stack
#define OP_POP 0x11

#define OP_GET_LOCAL 0x12

#define OP_SET_LOCAL 0x13

#define OP_SCOPE_EXIT 0x14

#define OP_CALL 0x15

//#op -> string var
//##op -> connect var name to others
#define OP_STR(op) \
    case OP_##op:  \
        return #op

inline std::string opcodeToString(uint8_t opcode) {
    switch (opcode) {
        OP_STR(HALT);
        OP_STR(CONST);
        OP_STR(ADD);
        OP_STR(SUB);
        OP_STR(MUL);
        OP_STR(DIV);
        OP_STR(COMPARE);
        OP_STR(JMP_IF_FALSE);
        OP_STR(JUMP);
        OP_STR(GET_GLOBAL);
        OP_STR(SET_GLOBAL);
        OP_STR(POP);
        OP_STR(GET_LOCAL);
        OP_STR(SET_LOCAL);
        OP_STR(SCOPE_EXIT);
        OP_STR(CALL);
        default:
            DIE << "opcodeToString: Unknown opcode: " << (int)opcode;
    }

    return "UnKnown";
}

#endif