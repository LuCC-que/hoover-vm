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
            }
            break;
    }
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
