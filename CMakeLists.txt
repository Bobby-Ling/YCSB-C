cmake_minimum_required(VERSION 3.10)

project(YCSB-C)

set(CMAKE_CXX_STANDARD 17)

include(common.cmake)

add_subdirectory(deps)
add_subdirectory(log)

find_package(RocksDB REQUIRED)

set(YCSBC_SOURCES ycsbc.cc)

set(YCSBC_SOURCE_DIRS
    core
    db
)

foreach(dir ${YCSBC_SOURCE_DIRS})
    file(GLOB_RECURSE DIR_SOURCES ${dir}/*.cc)
    list(APPEND YCSBC_SOURCES ${DIR_SOURCES})
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ycsbc
    ${YCSBC_SOURCES}
)

message(STATUS "RocksDB_DIR: ${RocksDB_DIR}")

target_link_libraries(ycsbc pthread tbb leveldb rocksdb external logger)
