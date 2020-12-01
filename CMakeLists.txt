cmake_minimum_required(VERSION 3.5)

project(KiNet CXX)

add_library(libminisat STATIC
    KiNet.hpp
)


add_executable(Client client.cpp)

add_executable(Server server.cpp)