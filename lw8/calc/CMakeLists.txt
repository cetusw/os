cmake_minimum_required(VERSION 3.20)
project(calc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(calc
        src/main.cpp
        src/TCPSocket.cpp
        src/Calculator.cpp
        src/Server.cpp
        src/Client.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(calc PRIVATE Threads::Threads)
