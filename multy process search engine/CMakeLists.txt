cmake_minimum_required(VERSION 3.9)
project(hw2)

set(CMAKE_CXX_STANDARD 11)

add_executable(hw2
        datasets/dataset.txt
        datasets/Entertainment.txt
        datasets/input.txt
        docfile
        jobExecutor_main.cpp
        List.cpp
        List.h
        main.h
        Trie.cpp
        Trie.h
        worker_main.cpp)
