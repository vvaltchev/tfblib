# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.10)
project(tfblib_tools)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(binary2c binary2c.c)
