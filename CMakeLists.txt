cmake_minimum_required(VERSION 3.5)

project(KiNet)

add_library(libminisat STATIC
    KiNet.hpp
)

#add_executable(KiNet client.cpp)

#add_executable(KiNet server.cpp)