# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.2)

if (NOT "${CMAKE_PROJECT_NAME}" STREQUAL "tilck")

   project(tfblib C)

else()

   # We're not using the project() command here because CMake does not handle
   # sub-projects (with add_subdirectory) in the case the parent project changed
   # the CMAKE_C_COMPILER variable. This use case is relevant for Tilck[1].
   # In particular, the problem is that Tilck by default cannot build all of its
   # targets using a single compiler: unit tests, which are completely portable,
   # build by default for the host machine, no matter which ARCH is used for the
   # kernel itself. That's why for the arch-specific code, Tilck's build system
   # set manually the CMAKE_C_COMPILER variable. Apparently, with that variable
   # manually overriden, it's not possible (currently) to build a sub-project
   # in a sub-directory, by just adding it with add_subdirectory().
   #
   # ---------------------------------------------------
   # [1] https://github.com/vvaltchev/tilck

   message(STATUS "Building tfblib as part of Tilck")

endif()

add_subdirectory(tools)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file (GLOB font_files "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*")

set(font_decls "")
set(fonts_list_str "")

foreach(file ${font_files})

   get_filename_component(fn ${file} NAME)

   string(REGEX MATCH "^(.*)\\.[^.]*$" ignored_output ${fn})
   set(basename ${CMAKE_MATCH_1})

   add_custom_command(

      OUTPUT
         fonts/${basename}.c

      COMMAND
         mkdir -p fonts

      COMMAND
         binary2c ${file} fonts/${basename}.c font_${basename}

      DEPENDS
         binary2c
   )

   list(APPEND c_font_files fonts/${basename}.c)
   string(CONCAT font_decls "${font_decls}\nextern font_file ${basename};")
   string(CONCAT fonts_list_str "&${basename}, ${fonts_list_str}")

endforeach()

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts_decls_template
   fonts/fonts_decls.c
   @ONLY
)

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(tfb STATIC ${LIB_SOURCES} ${c_font_files} fonts/fonts_decls.c)

add_subdirectory(examples)
