
cmake_minimum_required(VERSION 3.15)
project(flight_simulator)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS -pthread)

add_executable(flight_simulator main.cpp ex1.cpp Interpreter.cpp commands.cpp commands.h Var.h SymbolTable.cpp SymbolTable.h)