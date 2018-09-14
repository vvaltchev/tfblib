# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)
project (tfb C)

file(GLOB SOURCES "src/*.c")
add_library(tfb STATIC ${SOURCES})