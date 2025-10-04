cmake_minimum_required(VERSION 4.0)
project(exe_packer)

set(CMAKE_CXX_STANDARD 20)

add_executable(exe_packer main.cpp
        ExePacker.cpp
        ExePacker.h
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZLIB REQUIRED zlib)

target_link_libraries(exe_packer ${ZLIB_LIBRARIES})
target_include_directories(exe_packer PRIVATE ${ZLIB_INCLUDE_DIRS})
target_compile_options(exe_packer PRIVATE ${ZLIB_CFLAGS_OTHER})