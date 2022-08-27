add_library(compiler src/EvaCompiler.cpp src/Scope.cpp)

target_include_directories(compiler PUBLIC include)
