cmake_minimum_required(VERSION 4.0)
project(MemorySimulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(MemorySimulator main.cpp
        PhysicalMemory.cpp
        PhysicalMemory.h
        OSHandler.cpp
        OSHandler.h
        VirtualMemory.cpp
        VirtualMemory.h)
