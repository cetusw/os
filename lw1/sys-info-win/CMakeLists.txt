cmake_minimum_required(VERSION 3.31)
project(sys_info_win)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_WIN32_WINNT=0x0A00)

add_executable(sys_info_win main.cpp)
