cmake_minimum_required(VERSION 3.15)

project(bzf)

find_package(ZLIB)

add_executable(bzf 
				main.cpp
				src/BZFArchive.cpp)

set_property(TARGET bzf PROPERTY CXX_STANDARD 17)

target_link_libraries(bzf ${ZLIB_LIBRARIES})

target_include_directories(bzf PRIVATE include ${ZLIB_INCLUDE_DIRS})

message(${ZLIB_INCLUDE_DIRS})