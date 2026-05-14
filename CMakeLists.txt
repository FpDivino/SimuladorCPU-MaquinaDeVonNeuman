cmake_minimum_required(VERSION 4.0)
project(SimuladorCPU_MaquinaDeVonNeuman C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(SimuladorCPU_MaquinaDeVonNeuman
        main.c
        mvn.h)
