cmake_minimum_required(VERSION 3.14)
project(MemorySimulator)

set(CMAKE_CXX_STANDARD 20)

add_library(memory-simulator
        src/MyOS.cpp
        src/PhysicalMemory.cpp
        src/VirtualMemory.cpp
)

target_include_directories(memory-simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(MemorySimulator src/main.cpp)
target_link_libraries(MemorySimulator PRIVATE memory-simulator)

enable_testing()
add_subdirectory(tests)
