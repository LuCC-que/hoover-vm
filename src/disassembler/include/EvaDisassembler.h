#ifndef EvaDisassembler_h
#define EvaDisassembler_h
#include <array>
#include <iomanip>
#include <iostream>
#include <memory>

#include "../../byteCode/include/OpCode.h"
#include "../../logger/include/Logger.h"
#include "../../vm/include/EvaValue.h"
#include "../../vm/include/Global.h"

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
    size_t disassembleCompare(CodeObject* co, uint8_t opcode, size_t offset);
    size_t disassembleJump(CodeObject* co, uint8_t opcode, size_t offset);
    size_t disassembleGlobal(CodeObject* co, uint8_t opcode, size_t offset);
    void dumpBytes(CodeObject* co, size_t offset, size_t count);
    void printOpCode(uint8_t opcode);
    uint16_t readWordAtOffset(CodeObject* co, size_t offset);
    std::shared_ptr<Global> global;

   public:
    void disassemble(CodeObject* co);
    EvaDisassembler(std::shared_ptr<Global> go) : global(go){};
};

static std::array<std::string, 6> inverseCompareOps_{
    "<",
    ">",
    "==",
    ">=",
    "<=",
    "!=",
};
#endif