# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)
project (tfb C)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")
add_library(tfb STATIC ${LIB_SOURCES})

add_executable(hello "${PROJECT_SOURCE_DIR}/examples/hello.c")
target_link_libraries(hello tfb)