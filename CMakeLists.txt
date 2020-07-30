cmake_minimum_required(VERSION 3.10)
project(complyer-sat)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(complyer-sat include/complyer-sat.h src/expression.cpp src/cnf.cpp src/model.cpp src/solver.cpp)
add_subdirectory(examples)


if(WITH_TEST)
    enable_testing()
    add_subdirectory(tests)
endif(WITH_TEST)