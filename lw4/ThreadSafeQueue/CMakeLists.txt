cmake_minimum_required(VERSION 4.0)
project(ThreadSafeQueue)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
FetchContent_MakeAvailable(benchmark)

add_executable(
        run_benchmarks
        benchmarks/ThreadSafeQueueBenchmark.cpp
)

target_link_libraries(
        run_benchmarks
        PRIVATE
        benchmark::benchmark
)

target_include_directories(
        run_benchmarks
        PRIVATE
        ${Boost_INCLUDE_DIRS}
)

add_executable(ThreadSafeQueue main.cpp
        ThreadSafeQueue.h
)

add_executable(run_tests
        tests/ThreadSafeQueueTest.cpp
)

target_link_libraries(
        run_tests
        PRIVATE gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
