cmake_minimum_required(VERSION 3.25)
project(ARMTinyVM)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(ARMTinyVM
        src/ARMTinyVM.h src/ARMTinyVM.c src/main.c src/instruction_set.h src/win_elf.h)
