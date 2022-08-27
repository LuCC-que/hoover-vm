#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    EvaVM vm;
    auto result = vm.exec(R"(
        (def createCounter()
            (begin
                (var value 0)
                (def inc () (set value (+ value 1)))
                inc
            )
        )
        
        (var fn1 (createCounter))
        (var fn2 (createCounter))

        (fn1)
        (fn2)
        
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