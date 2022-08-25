add_library(vm src/EvaVM.cpp src/Global.cpp)

target_include_directories(vm PUBLIC include PRIVATE byteCode logger parser compiler)