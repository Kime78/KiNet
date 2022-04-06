cmake_minimum_required(VERSION 3.5)

project(KiNet CXX)

IF (WIN32)
    
ELSE()
    add_library(libminisat STATIC
    KiNet.hpp
    )
ENDIF()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

link_libraries(ws2_32 wsock32)

add_executable(Client client.cpp)

add_executable(Server server.cpp)
target_link_libraries(Server Threads::Threads)

#-lws2_32