cmake_minimum_required(VERSION 3.21)
project(matMultp C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)

add_executable(matMultp main.c)
