#ifndef EvaDisassembler_h
#define EvaDisassembler_h
#include <iomanip>
#include <iostream>

#include "../../vm/include/EvaValue.h"

#define DIVDIER()                               \
    std::cout << "\n-------------Disassembly: " \
              << co->name                       \
              << " ---------------------\n"     \
              << std::endl;

class EvaDisassembler {
   private:
    size_t disassembleInstruction(CodeObject* co, size_t offset);
    size_t disassembleSimple(CodeObject* co, uint8_t opcode, size_t offset);
    size_t disassembleConst(CodeObject* co, uint8_t opcode, size_t offset);
    void dumpBytes(CodeObject* co, size_t offset, size_t count);
    void printOpCode(uint8_t opcode);

   public:
    void disassemble(CodeObject* co);
};
#endif