# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)

add_executable(fb_hello fb_hello.c)
target_link_libraries(fb_hello tfb)

add_executable(fb_keyinput fb_keyinput.c)
target_link_libraries(fb_keyinput tfb)
