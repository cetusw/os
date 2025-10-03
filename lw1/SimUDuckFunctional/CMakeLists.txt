cmake_minimum_required(VERSION 4.0)
project(SimUDuckFunctional)

set(CMAKE_CXX_STANDARD 20)

add_executable(SimUDuckFunctional main.cpp
        lib/Duck/FunctionalBehaviors/FunctionalBehaviors.h
)
