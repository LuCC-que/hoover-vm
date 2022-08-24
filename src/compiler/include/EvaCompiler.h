#ifndef EvaCompiler_h
#define EvaCompiler_h

#include "../../byteCode/include/OpCode.h"
#include "../../logger/include/Logger.h"
#include "../../parser/include/EvaParser.h"
#include "../../vm/include/EvaValue.h"

class EvaCompiler {
   private:
    void emit(uint8_t code);
    size_t numericConstIdx(double value);
    size_t stringConstIdx(const std::string& value);

   public:
    EvaCompiler(){};

    CodeObject* compile(const Exp& exp);

    void gen(const Exp& exp);

    CodeObject* co;
};

#endif