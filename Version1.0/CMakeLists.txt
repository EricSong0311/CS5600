cmake_minimum_required(VERSION 3.26)
project(P1 C)

set(CMAKE_C_STANDARD 11)

add_executable(P1 main.c
        message.c
        message.h)
