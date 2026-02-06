cmake_minimum_required(VERSION 4.0)
project(FileSystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 REQUIRED fuse3)

set(COMMON_SOURCES
        src/FileSystemManager.cpp
        include/FileSystemManager.h
        include/Structure.h
)

add_executable(myfs-mkfs
        src/mkfs.cpp
        ${COMMON_SOURCES}
)
target_include_directories(myfs-mkfs PRIVATE include)

add_executable(myfs-mount
        src/mount.cpp
        src/MountManager.cpp
        include/MountManager.h
        ${COMMON_SOURCES}
)

target_include_directories(myfs-mount PRIVATE include ${FUSE3_INCLUDE_DIRS})
target_link_libraries(myfs-mount PRIVATE ${FUSE3_LIBRARIES})
target_compile_options(myfs-mount PRIVATE ${FUSE3_CFLAGS})
