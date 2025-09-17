cmake_minimum_required(VERSION 3.31)
project(sys_info_crossplatform)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    add_executable(sys_info_crossplatform main.cpp
            SysInfoWindows.cpp)
elseif(UNIX)
    add_executable(sys_info_crossplatform main.cpp
            SysInfoLinux.cpp)
endif()