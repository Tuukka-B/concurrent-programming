cmake_minimum_required(VERSION 3.20)
project(concurrent_programming)

set(CMAKE_CXX_STANDARD 14)

add_executable(concurrent_programming main.cpp ass1.h ass2.h ass3.h ass4.h ass5.h ass6.h ass7.h ass8.h)

set(CMAKE_CXX_FLAGS "-pthread")
