#include "EvaDisassembler.h"

void EvaDisassembler::disassemble(CodeObject* co) {
    DIVDIER()
    size_t offset = 0;
    while (offset < co->code.size()) {
        offset = disassembleInstruction(co, offset);
        std::cout << "\n";
    }
    DIVDIER()
}

size_t EvaDisassembler::disassembleInstruction(CodeObject* co, size_t offset) {
    std::ios_base::fmtflags f(std::cout.flags());

    // print bytecode offset;
    std::cout << std::uppercase
              << std::hex
              << std::setw(4)
              << std::setfill('0')
              << offset
              << "   ";

    auto opcode = co->code[offset];
    switch (opcode) {
        case OP_HALT:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_POP:
            return disassembleSimple(co, opcode, offset);
        case OP_SCOPE_EXIT:
            return disassembleWord(co, opcode, offset);
        case OP_CONST:
            return disassembleConst(co, opcode, offset);
        case OP_COMPARE:
            return disassembleCompare(co, opcode, offset);
        case OP_JMP_IF_FALSE:
        case OP_JUMP:
            return disassembleJump(co, opcode, offset);
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL:
            return disassembleGlobal(co, opcode, offset);
        case OP_GET_LOCAL:
        case OP_SET_LOCAL:
            return disassembleLocal(co, opcode, offset);
        default:
            DIE << "disassembleInstruction: no disassembly for" << opcodeToString(opcode);
    }
    std::cout.flags(f);

    return 0;
}
size_t EvaDisassembler::disassembleSimple(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 1);
    printOpCode(opcode);
    return offset + 1;
}
size_t EvaDisassembler::disassembleConst(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpCode(opcode);
    auto constIndex = co->code[offset + 1];
    std::cout << (int)constIndex
              << " ("
              << evaValueToConstantString(co->constants[constIndex])
              << ")";

    return offset + 2;
}

size_t EvaDisassembler::disassembleCompare(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpCode(opcode);
    auto compareOp = co->code[offset + 1];
    std::cout << (int)compareOp << " (";
    std::cout << inverseCompareOps_[compareOp] << ")";
    return offset + 2;
}

size_t EvaDisassembler::disassembleJump(CodeObject* co, uint8_t opcode, size_t offset) {
    std::ios_base::fmtflags f(std::cout.flags());

    dumpBytes(co, offset, 3);
    printOpCode(opcode);
    uint16_t address = readWordAtOffset(co, offset + 1);

    std::cout << std::uppercase
              << std::hex
              << std::setfill('0')
              << std::setw(4)
              << (int)address
              << " ";

    std::cout.flags(f);

    return offset + 3;
}

size_t EvaDisassembler::disassembleGlobal(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpCode(opcode);

    auto globalIndex = co->code[offset + 1];

    std::cout << (int)globalIndex
              << " ("
              << global->get(globalIndex).Name
              << ")";

    return offset + 2;
}

size_t EvaDisassembler::disassembleLocal(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpCode(opcode);

    auto localIndex = co->code[offset + 1];

    std::cout << (int)localIndex
              << " ("
              << co->locals[localIndex].name
              << ")";

    return offset + 2;
}

size_t EvaDisassembler::disassembleWord(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpCode(opcode);
    std::cout << (int)co->code[offset + 1];
    return offset + 2;
}

uint16_t EvaDisassembler::readWordAtOffset(CodeObject* co, size_t offset) {
    return (uint16_t)((co->code[offset] << 8) | co->code[offset + 1]);
}

void EvaDisassembler::dumpBytes(CodeObject* co, size_t offset, size_t count) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::stringstream ss;
    for (auto i = 0; i < count; ++i) {
        ss << std::uppercase
           << std::hex
           << std::setfill('0')
           << std::setw(2)
           << (((int)co->code[offset + i]) & 0xFF) << " ";
    }

    std::cout << std::left
              << std::setfill(' ')
              << std::setw(12)
              << ss.str();

    std::cout.flags(f);
}
void EvaDisassembler::printOpCode(uint8_t opcode) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << std::left
              << std::setfill(' ')
              << std::setw(20)
              << opcodeToString(opcode) << " ";

    std::cout.flags(f);
}
