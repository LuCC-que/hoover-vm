#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    EvaVM vm;
    auto result = vm.exec(R"(

    (if (< 5 10) 1 2)

   )");

    std::cout << "where?" << std::endl;
    log(result);

    std::cout << "all passed!" << std::endl;
    return 0;
}
