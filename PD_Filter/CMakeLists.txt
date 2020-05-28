cmake_minimum_required(VERSION 3.16)
project(PD_Filter)

set(CMAKE_CXX_STANDARD 14)

add_executable(PD_Filter
        #main.cpp
        dict.hpp
        hash_table.hpp hash_table.cpp
        PD.h PD.cpp
        macros.h
        tests.hpp tests.cpp
        hashutil.hpp hashutil.cpp
        )
