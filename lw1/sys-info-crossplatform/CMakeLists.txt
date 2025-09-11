cmake_minimum_required(VERSION 3.31)
project(sys_info_crossplatform)

set(CMAKE_CXX_STANDARD 20)

add_executable(sys_info_crossplatform main.cpp
        SysInfo.cpp
        SysInfo.h)
