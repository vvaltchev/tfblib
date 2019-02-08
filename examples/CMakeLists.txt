# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)

add_executable(fb_hello_world fb_hello_world.c)
target_link_libraries(fb_hello_world tfb)

add_executable(fb_drawing fb_drawing.c)
target_link_libraries(fb_drawing tfb)

add_executable(fb_keyinput fb_keyinput.c)
target_link_libraries(fb_keyinput tfb)

add_executable(fb_text fb_text.c)
target_link_libraries(fb_text tfb)

file(GLOB TETRIS_SRC tetris/*.c)
add_executable(fb_tetris ${TETRIS_SRC})
target_link_libraries(fb_tetris tfb)

# Tilck compiles all the applications with a libmusl GCC toolchain which has
# the math library embedded. Therefore, we need to link to only in the Linux
# case.
if (NOT "${CMAKE_PROJECT_NAME}" STREQUAL "tilck")
   target_link_libraries(fb_drawing m)
endif()
