#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    EvaVM vm;
    auto result = vm.exec(R"(

    3

   )");

    std::cout << "where?" << std::endl;
    log(AS_NUMBER(result));

    std::cout << "all passed!" << std::endl;
    return 0;
}
