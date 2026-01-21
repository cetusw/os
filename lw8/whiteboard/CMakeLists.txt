cmake_minimum_required(VERSION 3.20)
project(whiteboard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(Threads REQUIRED)

add_executable(whiteboard src/main.cpp
        src/Server.cpp
        src/Client.cpp
        src/Whiteboard.cpp
)

target_include_directories(whiteboard PRIVATE include)

target_link_libraries(whiteboard PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        Threads::Threads
)
