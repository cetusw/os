cmake_minimum_required(VERSION 3.14)
project(MemoryManager)

set(CMAKE_CXX_STANDARD 20)

add_library(memory-manager
        src/MemoryManager.cpp
        tests/MemoryManagerTest.cpp
)

target_include_directories(memory-manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(MemoryManager src/main.cpp)
target_link_libraries(MemoryManager PRIVATE memory-manager)

enable_testing()
add_subdirectory(tests)
