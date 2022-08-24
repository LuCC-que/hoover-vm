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
            DIE << "ExpType::SYMBOL: unimplemented";
            break;
        case ExpType::LIST:
            DIE << "ExpType::LIST: unimplemented";
            break;
    }
}

void EvaCompiler::emit(uint8_t code) {
    co->code.push_back(code);
}

size_t EvaCompiler::numericConstIdx(const double value) {
    for (auto itr = co->constants.begin(); itr != co->constants.end(); ++itr) {
        if (!IS_NUMBER(*itr)) {
            continue;
        }

        if (AS_NUMBER(*itr) == value) {
            return std::distance(co->constants.begin(), itr);
        }
    }

    co->constants.push_back(NUMBER(value));

    return co->constants.size() - 1;
}

size_t EvaCompiler::stringConstIdx(const std::string& value) {
    for (auto itr = co->constants.begin(); itr != co->constants.end(); ++itr) {
        if (!IS_STRING(*itr)) {
            continue;
        }

        if (AS_CPPSTRING(*itr) == value) {
            return std::distance(co->constants.begin(), itr);
        }
    }

    co->constants.push_back(ALLOC_STRING(value));
    return co->constants.size() - 1;
}