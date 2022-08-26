#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    EvaVM vm;
    auto result = vm.exec(R"(

        // (def square (x) (* x x))

        // (square 2) //4

        (def sum (a b)
            (begin
                (var x 10)
                (+ x (+ a b))))

        // (def factorial (x)
        //     (if (== x 1)
        //         1
        //         (* x (factorial (- x 1)))))
        
        // (factorial 5)
   )");

    log(result);

    vm.DebugDumpStack(0);
    std::cout << std::endl;
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