# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)

# NOTE: not using the "project()" command because CMake does not handle
# sub-projects (with add_subdirectory) in the case the parent project changed
# the CMAKE_C_COMPILER variable. This use case is relevant for Tilck.
# See https://github.com/vvaltchev/tilck
# For uses outside of Tilck, CMake works just fine even without project().

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(tfb STATIC ${LIB_SOURCES})

add_executable(fb_hello "${CMAKE_CURRENT_SOURCE_DIR}/examples/fb_hello.c")
target_link_libraries(fb_hello tfb)

set(EXTRA_APPS_LIST_copy ${extra_apps_list})
list(APPEND EXTRA_APPS_LIST_copy "${CMAKE_CURRENT_BINARY_DIR}/fb_hello")
set(EXTRA_APPS_LIST ${EXTRA_APPS_LIST_copy} PARENT_SCOPE)
