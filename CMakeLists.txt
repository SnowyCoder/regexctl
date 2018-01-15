cmake_minimum_required(VERSION 3.8)
project(regexctl)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)

set(CMAKE_CXX_FLAGS "-Os")

find_package(Curses REQUIRED)
include_directories(${CURSER_INCLUDE_DIR})


add_executable(regexctl ${SOURCE_FILES})

target_link_libraries(regexctl ${CURSES_LIBRARIES})
