cmake_minimum_required(VERSION 3.18)
project(L1b)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fopenmp -lcrypto")

add_executable(L1b main.cpp Data.h json.hpp DataBuffer.cpp DataBuffer.h ResultsBuffer.cpp ResultsBuffer.h)

