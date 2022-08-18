cmake_minimum_required(VERSION 3.10)

project(Vision LANGUAGES CXX CUDA)


set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

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

add_executable(unit_tests 
    unit_tests.cpp
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
    
add_executable(tuner
    tuner.cu
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


target_compile_features(tuner PUBLIC cxx_std_11)
set_target_properties(tuner 
    PROPERTIES CUDA_SEPARABLE_COMPILATION ON
    )

