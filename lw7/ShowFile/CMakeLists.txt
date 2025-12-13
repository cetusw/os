cmake_minimum_required(VERSION 4.0)
project(ShowFile)

set(CMAKE_CXX_STANDARD 20)

add_library(show-file
        src/ImageReader.cpp
        src/FatSystem.cpp
)

target_include_directories(show-file PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ShowFile src/main.cpp)
target_link_libraries(ShowFile PRIVATE show-file)
