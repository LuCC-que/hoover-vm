#include <iostream>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

int main(int argc, char const *argv[]) {
    {
        EvaVM vm;
        Traceable::printStats();
        auto result = vm.exec(R"(

        (+ "hello" "string")

   )");

        //     vm.exec(R"(

        //     (+ "hello" "string")

        // )");

        vm.DebugDumpStack(0);
        std::cout << std::endl;
        log(result);

        Traceable::printStats();
    }

    Traceable::printStats();

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