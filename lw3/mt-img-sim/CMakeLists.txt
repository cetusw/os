cmake_minimum_required(VERSION 4.0)
project(mt_img_sim CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG        v3.3.1
)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)

FetchContent_MakeAvailable(cxxopts stb)

add_executable(mt_img_sim main.cpp)

target_include_directories(mt_img_sim PRIVATE
        ${stb_SOURCE_DIR}
)

target_link_libraries(mt_img_sim PRIVATE
        cxxopts::cxxopts
        Threads::Threads
)