#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    EvaVM vm;
    auto result = vm.exec(R"(
        // (var adder (lambda (x) (+ x x)))
        // (def factorial (x)
        //     (if (== x 1)
        //         1
        //         (* x (factorial (- x 1)))))
        
        // (factorial 5)
        // //IIIE
        // (adder 2)

        // (var x 1)
        // (var y (+ x 1))

        (begin
            (var y 100)
            (begin 
                (var z 200)
                (def bar () (+ y z))
                (bar)
            )
        )
        
   )");

    vm.DebugDumpStack(0);
    std::cout << std::endl;
    log(result);
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