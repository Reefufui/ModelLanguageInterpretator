cmake_minimum_required (VERSION 3.8)
project (mli)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wpedantic")

add_executable(mli
    src/main.cpp
    )

