#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    EvaVM vm;
    auto result = vm.exec(R"(

        (var x 5)
        (set x (+ x 10))

        x

        (begin
            (var x 100)
            (begin 
                (var x 200)
            x)
        x)

        x
   )");

    log(result);

    std::cout << "all passed!" << std::endl;
    return 0;
}

/**
 * @temp
 *
 * (begin
            (var x 100)
            x)
 */