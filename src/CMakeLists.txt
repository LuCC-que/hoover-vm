cmake_minimum_required(VERSION 3.0.0)

project(HOOVE_VM VERSION 1.0.0)

set (CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# set(CMAKE_CXX_FLAGS_RELEASE "-O")

add_subdirectory(vm)
add_subdirectory(logger)
add_subdirectory(parser)
add_subdirectory(compiler)

add_executable(VM main.cpp)



target_link_libraries(VM PRIVATE vm logger parser compiler)