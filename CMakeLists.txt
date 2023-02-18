cmake_minimum_required(VERSION 3.10)

project(Vision LANGUAGES CXX)


set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

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

add_executable(tests 
    tests.cpp
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
