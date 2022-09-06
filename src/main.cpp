#include <fstream>
#include <iostream>
#include <string>

#include "EvaVM.h"
#include "EvaValue.h"
#include "Logger.h"

void printHelp() {
    std::cout << "\nUsage: eva-vm [options]\n\n"
              << "Options:\n"
              << "   -e, --expression Expression to parse\n"
              << "   -f, --file       File to parse\n\n";
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printHelp();
        return 0;
    }
    // Expression mode.
    std::string mode = argv[1];

    std::string program;
    // program to execute.

    if (mode == "-e") {
        program = argv[2];
    } else if (mode == "-f") {
        std::ifstream programFile(argv[2]);
        std::stringstream buffer;
        buffer << programFile.rdbuf() << "\n";

        program = buffer.str();
    } else {
        printHelp();
        return 0;
    }
    EvaVM vm;

    auto result = vm.exec(program);

    // Traceable::printStats();
    // auto result = vm.exec(R"(

    //         (class Point null
    //             (def constructor (self x y)
    //                 (begin
    //                     (set (prop self x) x)
    //                     (set (prop self y) y)
    //                     self
    //                 )
    //             )

    //             (def calc (self)

    //                 (+ (prop self x) (prop self y))
    //             )
    //         )

    //         (class Point3D Point
    //             (def constructor (self x y z)
    //                 (begin
    //                     ((prop (super Point3D) constructor) self x y)
    //                     (set (prop self z) z)
    //                     self
    //                 )
    //             )

    //             (def calc (self)

    //                 (+ ((prop (super Point3D) calc) self) (prop self z))
    //             )
    //         )

    //         (var p (new Point3D 10 20 30))

    //         ((prop p calc) p) //pass p as self

    //    )");

    // vm.DebugDumpStack(0);
    // std::cout << std::endl;
    log(result);

    // Traceable::printStats();

    // Traceable::printStats();

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