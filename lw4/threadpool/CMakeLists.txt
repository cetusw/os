cmake_minimum_required(VERSION 3.10)
project(threadpool)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
include(GoogleTest)

add_library(ThreadPool
        ThreadPool.cpp)

target_include_directories(ThreadPool
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(threadpool main.cpp
        ThreadPool.cpp)

target_link_libraries(threadpool
        ThreadPool
)

add_executable(thread_pool_test
        ../tests/ThreadPoolTest.cpp
)

target_link_libraries(thread_pool_test
        ThreadPool
        GTest::gtest
        GTest::gtest_main
)

add_executable(thread_pool_benchmarks
        benchmarks/ThreadPoolBenchmark.cpp)

target_link_libraries(thread_pool_benchmarks
        ThreadPool
        benchmark::benchmark)
