add_library(vm src/EvaVM.cpp)

target_include_directories(vm PUBLIC include PRIVATE byteCode logger parser compiler)