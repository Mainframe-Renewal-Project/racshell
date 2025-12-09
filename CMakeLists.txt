
cmake_minimum_required(VERSION 3.24)

project(racshell C CXX)

set(CXX_STANDARD 17)

add_library(
    racshell

    racshell/listuser.cpp
)

target_include_directories(
    racshell PUBLIC
    rachsell
    externals/argparse/
    externals/nlohmann/
)

add_executable(listuser "racshell/listuser.cpp")
target_link_libraries(listuser racshell -static)