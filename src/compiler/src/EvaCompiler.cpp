#include "EvaCompiler.h"

CodeObject* EvaCompiler::compile(const Exp& exp) {
    co = AS_CODE(ALLOC_CODE("main"));
    gen(exp);
    emit(OP_HALT);
    return co;
}

void EvaCompiler::gen(const Exp& exp) {
    switch (exp.type) {
            //---------------------------
            // Number
        case ExpType::NUMBER:
            emit(OP_CONST);
            emit(numericConstIdx(exp.number));
            break;

        case ExpType::STRING:
            emit(OP_CONST);
            emit(stringConstIdx(exp.string));
            break;

        case ExpType::SYMBOL:
            if (exp.string == "true" || exp.string == "false") {
                emit(OP_CONST);
                emit(booleanConstIdx(exp.string == "true" ? true : false));
            } else {
                //---------------------------------------------
                // variable

                if (!global->exists(exp.string)) {
                    DIE << "[EvaCompiler]: Reference error:" << exp.string;
                }

                emit(OP_GET_GLOBAL);
                emit(global->getGlobalIndex(exp.string));
            }
            break;

        /**
         * @brief
         *  Lists
         */
        case ExpType::LIST:
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL) {
                auto op = tag.string;
                //--------------------
                // Binary math operations
                if (op == "+") {
                    GEN_BINARY_OP(OP_ADD);
                }

                else if (op == "-") {
                    GEN_BINARY_OP(OP_SUB);
                }

                else if (op == "*") {
                    GEN_BINARY_OP(OP_MUL);
                }

                else if (op == "/") {
                    GEN_BINARY_OP(OP_DIV);
                }

                //--------------------
                // Compare operations (> 5 10)
                else if (compareOps_.count(op) != 0) {
                    gen(exp.list[1]);
                    gen(exp.list[2]);
                    emit(OP_COMPARE);
                    emit(compareOps_[op]);
                }

                //-------------------------
                // Branch Instruction:

                if (op == "if") {
                    gen(exp.list[1]);
                    emit(OP_JMP_IF_FALSE);

                    // dont know the brach address yet
                    emit(0);
                    emit(0);

                    // record where to save it
                    auto elseJmpAddr = getOffset() - 2;

                    // Emit <consequent>
                    gen(exp.list[2]);
                    emit(OP_JUMP);

                    // dont know end address yet
                    emit(0);
                    emit(0);

                    auto endAddr = getOffset() - 2;

                    // now see the address, the idx of last element + 1
                    auto elseBranchAddr = getOffset();
                    patchJumpAddress(elseJmpAddr, elseBranchAddr);

                    // put code to that position
                    if (exp.list.size() == 4) {
                        gen(exp.list[3]);
                    }

                    auto endBranchAddr = getOffset();
                    patchJumpAddress(endAddr, endBranchAddr);
                }

                else if (op == "var") {
                    // 1 is variable
                    global->define(exp.list[1].string);

                    // 2 is exp
                    gen(exp.list[2]);

                    emit(OP_SET_GLOBAL);
                    emit(global->getGlobalIndex(exp.list[1].string));
                }
            }
            break;
    }
}

void EvaCompiler::writeByteOffset(size_t offset, uint8_t value) {
    co->code[offset] = value;
}

void EvaCompiler::patchJumpAddress(size_t offset, uint16_t value) {
    writeByteOffset(offset, (value >> 8) & 0xff);
    writeByteOffset(offset + 1, value & 0xff);
}

size_t EvaCompiler::getOffset() {
    return co->code.size();
}

void EvaCompiler::emit(uint8_t code) {
    co->code.push_back(code);
}

size_t EvaCompiler::numericConstIdx(const double value) {
    ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);

    return co->constants.size() - 1;
}

size_t EvaCompiler::stringConstIdx(const std::string& value) {
    ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
    return co->constants.size() - 1;
}

size_t EvaCompiler::booleanConstIdx(const bool value) {
    ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);

    return co->constants.size() - 1;
}

void EvaCompiler::disassembleByteCode() {
    disassembler->disassemble(co);
}
