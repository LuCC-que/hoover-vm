#include "EvaCompiler.h"

void EvaCompiler::compile(const Exp& exp) {
    co = AS_CODE(creatCodeObjectValue("main"));
    main = AS_FUNCTION(ALLOC_FUNCTION(co));

    analyze(exp, nullptr);
    gen(exp);
    emit(OP_HALT);
}

void EvaCompiler::analyze(const Exp& exp,
                          std::shared_ptr<Scope> scope) {
    switch (exp.type) {
        case ExpType::SYMBOL: {
            if (exp.string == "true" || exp.string == "false") {
                // do nothing
            } else {
                scope->maybePromote(exp.string);
                log(exp.string);
                log((int)scope->allocInfo[exp.string]);
            }
            break;
        }
        case ExpType::LIST: {
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL) {
                auto op = tag.string;

                if (op == "begin") {
                    auto newScope =
                        std::make_shared<Scope>(scope == nullptr
                                                    ? ScopeType::GLOBAL
                                                    : ScopeType::BLOCK,
                                                scope);

                    scopeInfo_[&exp] = newScope;
                    for (auto i = 1; i < exp.list.size(); ++i) {
                        analyze(exp.list[i], newScope);
                    }
                }

                else if (op == "var") {
                    scope->addLocal(exp.list[1].string);
                    analyze(exp.list[2], scope);
                }

                else if (op == "def") {
                    auto fnName = exp.list[1].string;

                    scope->addLocal(fnName);

                    auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);
                    scopeInfo_[&exp] = newScope;

                    newScope->addLocal(fnName);

                    auto arity = exp.list[2].list.size();

                    for (auto i = 0; i < arity; ++i) {
                        newScope->addLocal(exp.list[2].list[i].string);
                    }

                    analyze(exp.list[3], newScope);
                } else if (op == "lambda") {
                    auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);
                    scopeInfo_[&exp] = newScope;

                    auto arity = exp.list[1].list.size();

                    for (auto i = 0; i < arity; ++i) {
                        newScope->addLocal(exp.list[1].list[i].string);
                    }

                    analyze(exp.list[2], newScope);
                } else {
                    // binary
                    for (auto i = 1; i < exp.list.size(); ++i) {
                        analyze(exp.list[i], scope);
                    }
                }
            } else {
                for (auto i = 0; i < exp.list.size(); ++i) {
                    analyze(exp.list[i], scope);
                }
            }
            break;
        }
    }
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

                auto opCodeGetter = scopeStack_.top()->getNameGetter(varName);

                // cell ? local ? gloabl
                emit(opCodeGetter);

                switch (opCodeGetter) {
                    case OP_GET_LOCAL: {
                        emit(co->getLocalIndex(varName));
                        break;
                    }
                    case OP_GET_CELL: {
                        emit(co->getCellIndex(varName));
                        break;
                    }
                    default: {
                        if (!global->exists(varName)) {
                            DIE << "[EvaCompiler]: Reference error: " << varName;
                        }
                        emit(global->getGlobalIndex(varName));
                        break;
                    }
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

                    auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

                    if (isLambdaDeclaration(exp.list[2])) {
                        // if is lambda gen the function result
                        compileFunction(exp.list[2],
                                        varName,
                                        exp.list[2].list[1],
                                        exp.list[2].list[2]);
                    } else {
                        // gen the exp result
                        gen(exp.list[2]);
                    }

                    switch (opCodeSetter) {
                        case OP_SET_GLOBAL: {
                            global->define(varName);
                            emit(OP_SET_GLOBAL);
                            emit(global->getGlobalIndex(varName));
                            break;
                        }
                        case OP_SET_CELL: {
                            // emit(co->getCellIndex(varName));
                            co->cellNames.push_back(varName);
                            emit(OP_SET_CELL);
                            emit(co->cellNames.size() - 1);
                            emit(OP_POP);
                            break;
                        }
                        default: {
                            co->addLocal(varName);
                        }
                    }

                    // if (isGlobalScope()) {
                    //     global->define(varName);
                    //     emit(OP_SET_GLOBAL);
                    //     emit(global->getGlobalIndex(varName))
                    // }

                }

                else if (op == "set") {
                    const auto varName = exp.list[1].string;

                    auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

                    /**
                     * @brief
                     * the result will always put to the top of stack
                     * if it is constant, CONST IDX
                     * it it is expression, CONST L CONST R OP
                     */
                    gen(exp.list[2]);

                    switch (opCodeSetter) {
                        case OP_SET_LOCAL: {
                            emit(OP_SET_LOCAL);
                            emit(co->getLocalIndex(varName));
                            break;
                        }
                        case OP_SET_CELL: {
                            emit(OP_SET_CELL);
                            emit(co->getCellIndex(varName));
                            break;
                        }
                        default: {
                            auto globalIndex = global->getGlobalIndex(varName);
                            if (globalIndex == -1) {
                                DIE << "Reference Error: " << varName << " is not defined.";
                            }
                            emit(OP_SET_GLOBAL);
                            emit(globalIndex);
                            break;
                        }
                    }

                }

                else if (op == "begin") {
                    scopeStack_.push(scopeInfo_.at(&exp));
                    blockEnter();
                    for (auto i = 1; i < exp.list.size(); ++i) {
                        bool isLast = i == exp.list.size() - 1;

                        // auto isLocalDeclaration =
                        //     isDeclaration(exp.list[i]) && !isGlobalScope();

                        bool isDecl = isFuncDeclaration(exp.list[i]) ||
                                      isDeclaration(exp.list[i]);
                        gen(exp.list[i]);

                        // Global var already has the val
                        // not need to have them in stack anymore
                        // save memory

                        // after opt, only case set will be pop
                        // as set loads the value on the top of stack
                        // and set to the previous index
                        if (!isLast && !isDecl) {
                            emit(OP_POP);
                        }
                    }
                    blockExit();
                    scopeStack_.pop();
                }
                //------------------------------
                // user defined function:
                // (square 2)
                else if (op == "def") {
                    auto fnName = exp.list[1].string;
                    compileFunction(
                        /*exp*/ exp,
                        /*name*/ fnName,
                        /*params*/ exp.list[2],
                        /*body*/ exp.list[3]);

                    // define the function as a variable in out co:
                    /**
                     * @brief
                     * if (isGlobalScope()) {
                        global->define(varName);
                        emit(OP_SET_GLOBAL);
                        emit(global->getGlobalIndex(varName));
                    } else {
                        co->addLocal(varName);  // add local to count the position

                        //-------------------
                        // initializer is already in the right position
                        // emit(OP_SET_LOCAL);
                        // emit(co->getLocalIndex(varName));
                    }
                     */

                    SAVE_AS_GLOBAL_OR_LOCAL(fnName)

                } else if (op == "lambda") {
                    //
                    compileFunction(
                        /*exp*/ exp,
                        /*name*/ "lambda",
                        /*params*/ exp.list[1],
                        /*body*/ exp.list[2]);
                }
                //------------------------------
                // Function calls:
                // (square 2)
                else {
                    FUNCTION_CALL(exp)
                }
            }
            //------No symbol------------------
            // Lamda Function calls:
            // ()
            else {
                FUNCTION_CALL(exp)
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
    for (auto& co_ : codeObjects_) {
        disassembler->disassemble(co_);
    }
}

void EvaCompiler::blockEnter() {
    co->scopeLevel++;
}
void EvaCompiler::blockExit() {
    auto varsCount = getVarsCountOnScopeExit();

    if (varsCount > 0 || co->arity > 0) {
        emit(OP_SCOPE_EXIT);

        if (isFunctionBody()) {
            varsCount += co->arity + 1;
        }
        emit(varsCount);
    }
    co->scopeLevel--;
}

bool EvaCompiler::isGlobalScope() const {
    return co->name == "main" && co->scopeLevel == 1;
}

bool EvaCompiler::isFunctionBody() const {
    return co->name != "main" && co->scopeLevel == 1;
}

bool EvaCompiler::isDeclaration(const Exp& exp) const {
    return isVarDeclaration(exp);
}

bool EvaCompiler::isVarDeclaration(const Exp& exp) const {
    return isTaggedList(exp, "var");
}

bool EvaCompiler::isTaggedList(const Exp& exp,
                               const std::string& tag) const {
    return (exp.type == ExpType::LIST &&
            exp.list[0].type == ExpType::SYMBOL &&
            exp.list[0].string == tag);
}

bool EvaCompiler::isBlock(const Exp& exp) const {
    return isTaggedList(exp, "begin");
}

bool EvaCompiler::isLambdaDeclaration(const Exp& exp) const {
    return isTaggedList(exp, "lambda");
}

bool EvaCompiler::isFuncDeclaration(const Exp& exp) const {
    return isTaggedList(exp, "def");
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

EvaValue EvaCompiler::creatCodeObjectValue(const std::string& name, size_t arity) {
    auto coValue = ALLOC_CODE(name, arity);
    auto co = AS_CODE(coValue);

    codeObjects_.push_back(co);
    return coValue;
}

FunctionObject* EvaCompiler::getMainFunction() { return main; }

void EvaCompiler::compileFunction(const Exp& exp,
                                  const std::string fnName,
                                  const Exp& params,
                                  const Exp& body) {
    auto scopeInfo = scopeInfo_.at(&exp);
    scopeStack_.push(scopeInfo);

    auto arity = params.list.size();

    // save previous code object
    auto prevCo = co;

    // function code object:
    auto coValue = creatCodeObjectValue(fnName, arity);
    co = AS_CODE(coValue);

    // if (scopeInfo->scopeType != ScopeType::GLOBAL) {
    co->freeCount = scopeInfo->free.size();

    // preserver memory for potential uses
    co->cellNames.reserve(scopeInfo->free.size() +
                          scopeInfo->cells.size());

    co->cellNames.insert(co->cellNames.end(),
                         scopeInfo->free.begin(),
                         scopeInfo->free.end());

    co->cellNames.insert(co->cellNames.end(),
                         scopeInfo->cells.begin(),
                         scopeInfo->cells.end());
    // }

    // store new co as a constant:
    prevCo->addConst(coValue);

    // register the function as local
    // so the function can recursively call itself
    co->addLocal(fnName);

    // parameters are added as variables
    auto itr = params.list.begin();
    while (itr != params.list.end()) {
        auto argName = itr++->string;
        co->addLocal(argName);

        auto cellIndex = co->getCellIndex(argName);

        if (cellIndex != -1) {
            emit(OP_SET_CELL);
            emit(cellIndex);
        }
    }

    // compile body in the new code object:
    // code has been changed above
    gen(body);

    if (!isBlock(body)) {
        emit(OP_SCOPE_EXIT);
        emit(arity + 1);
    }

    emit(OP_RETURN);

    // add the function as constant
    auto fn = ALLOC_FUNCTION(co);  // encapusalte
    co = prevCo;                   // recover
    co->addConst(fn);

    emit(OP_CONST);
    emit(co->constants.size() - 1);

    scopeStack_.pop();
}
