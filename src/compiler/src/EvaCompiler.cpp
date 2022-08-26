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
                auto varName = exp.string;
                auto localIndex = co->getLocalIndex(varName);

                if (localIndex != -1) {
                    emit(OP_GET_LOCAL);
                    emit(localIndex);
                } else {
                    if (!global->exists(exp.string)) {
                        DIE << "[EvaCompiler]: Global Values Reference error:" << exp.string;
                    }

                    emit(OP_GET_GLOBAL);
                    emit(global->getGlobalIndex(exp.string));
                }
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

                else if (op == "if") {
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

                else if (op == "while") {
                    auto loopStartAddr = getOffset();

                    gen(exp.list[1]);
                    emit(OP_JMP_IF_FALSE);

                    // end address
                    emit(0);
                    emit(0);

                    auto loopEndJmpAddr = getOffset() - 2;

                    gen(exp.list[2]);

                    // iterate
                    emit(OP_JUMP);
                    emit(0);  // address
                    emit(0);
                    // set address
                    patchJumpAddress(getOffset() - 2, loopStartAddr);

                    auto loopEndAddr = getOffset() + 1;

                    // set end address
                    patchJumpAddress(loopEndJmpAddr, loopEndAddr);
                }

                else if (op == "for") {
                    // initialize
                    gen(exp.list[1]);

                    auto loopStartAddr = getOffset();
                    // condition
                    gen(exp.list[2]);

                    emit(OP_JMP_IF_FALSE);

                    // end address
                    emit(0);
                    emit(0);

                    auto loopEndJmpAddr = getOffset() - 2;

                    // body
                    gen(exp.list[4]);

                    // update
                    gen(exp.list[3]);

                    // iterate
                    emit(OP_JUMP);
                    emit(0);  // address
                    emit(0);
                    // set address
                    patchJumpAddress(getOffset() - 2, loopStartAddr);

                    auto loopEndAddr = getOffset() + 1;

                    // set end address
                    patchJumpAddress(loopEndJmpAddr, loopEndAddr);

                }

                else if (op == "var") {
                    // 1 is variable, set the var here
                    const auto varName = exp.list[1].string;
                    // gen the result first
                    gen(exp.list[2]);

                    if (isGlobalScope()) {
                        global->define(varName);
                        emit(OP_SET_GLOBAL);
                        // put the index into the stack
                        emit(global->getGlobalIndex(varName));
                    }

                    //---------------------
                    // locals

                    else {
                        co->addLocal(varName);
                        emit(OP_SET_LOCAL);
                        emit(co->getLocalIndex(varName));
                    }
                }

                else if (op == "set") {
                    const auto varName = exp.list[1].string;

                    /**
                     * @brief
                     * the result will always put to the top of stack
                     * if it is constant, CONST IDX
                     * it it is expression, CONST L CONST R OP
                     */
                    gen(exp.list[2]);
                    auto localIndex = co->getLocalIndex(varName);
                    if (localIndex != -1) {
                        emit(OP_SET_LOCAL);
                        emit(localIndex);
                    } else {
                        auto globalIndex = global->getGlobalIndex(varName);
                        if (globalIndex == -1) {
                            DIE << "Reference Error: " << varName << " is not defined.";
                        }
                        emit(OP_SET_GLOBAL);
                        emit(globalIndex);
                    }

                }

                else if (op == "begin") {
                    scopeEnter();
                    for (auto i = 1; i < exp.list.size(); ++i) {
                        bool isLast = i == exp.list.size() - 1;

                        auto isLocalDeclaration =
                            isDeclaration(exp.list[i]) && !isGlobalScope();

                        gen(exp.list[i]);

                        // Global var already has the val
                        // not need to have them in stack anymore
                        // save memory
                        if (!isLast && !isLocalDeclaration) {
                            emit(OP_POP);
                        }
                    }
                    scopeExit();
                }
                //------------------------------
                // Function calls:
                // (square 2)
                else {
                    // set up the function
                    gen(exp.list[0]);
                    for (auto i = 1; i < exp.list.size(); ++i) {
                        // set up parameters
                        gen(exp.list[i]);
                    }

                    emit(OP_CALL);

                    // set up how many arguments
                    emit(exp.list.size() - 1);
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

void EvaCompiler::scopeEnter() {
    co->scopeLevel++;
}
void EvaCompiler::scopeExit() {
    auto varsCount = getVarsCountOnScopeExit();

    if (varsCount > 0) {
        emit(OP_SCOPE_EXIT);
        emit(varsCount);
    }
    co->scopeLevel--;
}

bool EvaCompiler::isGlobalScope() {
    return co->name == "main" && co->scopeLevel == 1;
}

bool EvaCompiler::isDeclaration(const Exp& exp) {
    return isVarDeclaration(exp);
}

bool EvaCompiler::isVarDeclaration(const Exp& exp) {
    return isTaggedList(exp, "var");
}

bool EvaCompiler::isTaggedList(const Exp& exp,
                               const std::string& tag) {
    return (exp.type == ExpType::LIST &&
            exp.list[0].type == ExpType::SYMBOL &&
            exp.list[0].string == tag);
}

size_t EvaCompiler::getVarsCountOnScopeExit() {
    auto varsCount = 0;

    if (co->locals.size() > 0) {
        while (co->locals.back().scopeLevel == co->scopeLevel) {
            // remove the locals from the co here
            // vm doesnt need them
            co->locals.pop_back();
            varsCount++;
        }
    }

    return varsCount;
}