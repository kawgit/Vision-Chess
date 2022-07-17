cmake_minimum_required(VERSION 3.10)

project(Vision)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(Vision 
    main.cpp
    bits.cpp
    eval.cpp 
    hash.cpp
    movegen.cpp
    order.cpp
    pos.cpp
    search.cpp
    timer.cpp
    tt.cpp
    types.cpp
    uci.cpp
    )