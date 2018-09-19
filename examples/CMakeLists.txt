# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)


add_executable(fb_hello fb_hello.c)
target_link_libraries(fb_hello tfb)


# Extra stuff in order to allow a full integration with Tilck's build system
if (${CMAKE_PROJECT_NAME} STREQUAL "tilck")
   tilck_add_extra_app(${CMAKE_CURRENT_BINARY_DIR}/fb_hello)
endif()
