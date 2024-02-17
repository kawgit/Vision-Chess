cmake_minimum_required(VERSION 3.10)

project(Vision LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BUILTINS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

add_executable(Vision
        attacks.cpp 
        bits.cpp 
        hash.cpp 
        history.cpp 
        main.cpp
        move.cpp 
        movegen.cpp 
        movepicker.cpp 
        nnue.cpp
        pos.cpp 
        search.cpp 
        thread.cpp
        timer.cpp 
        tt.cpp 
        types.cpp 
        util.cpp 
    )

# add_executable(tests 
#     tests.cpp
#     bits.cpp
#     eval.cpp 
#     hash.cpp
#     movegen.cpp
#     # nnue.cpp
#     order.cpp
#     pos.cpp
#     search.cpp
#     timer.cpp
#     tt.cpp
#     types.cpp
#     uci.cpp
#     )
