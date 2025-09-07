# SPDX-License-Identifier: BSD-2-Clause
cmake_minimum_required(VERSION 3.10)
include(ExternalProject)

if (NOT "${CMAKE_PROJECT_NAME}" STREQUAL "tilck")

   project(tfblib
      LANGUAGES C
      HOMEPAGE_URL https://github.com/vvaltchev/tfblib
      DESCRIPTION "A Tiny Linux Framebuffer Library"
      VERSION "0.1.1"
      )

   set(CMAKE_C_FLAGS_DEBUG "-g")
   set(CMAKE_C_FLAGS_MINSIZEREL "-Os")
   set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g")
   set(CMAKE_C_FLAGS_RELEASE "-O3")

   if (NOT CMAKE_BUILD_TYPE)
     set(CMAKE_BUILD_TYPE Debug)
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG}")
   endif()

   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-unused-parameter")

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

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

ExternalProject_Add(

   tools

   PREFIX "${CMAKE_CURRENT_BINARY_DIR}/tools"
   SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools"
   INSTALL_COMMAND ""
)

set(B2C "${CMAKE_CURRENT_BINARY_DIR}/tools/src/tools-build/binary2c")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file (GLOB font_files "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*")

set(font_decls "")
set(fonts_list_str "0")

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
         ${B2C} ${file} fonts/${basename}.c font_${basename}

      DEPENDS
         tools
   )

   set(cvar "font_${basename}")
   list(APPEND c_font_files fonts/${basename}.c)
   set(new_decl "extern const struct font_file ${cvar};")
   set(font_decls "${font_decls}\n${new_decl}")
   set(fonts_list_str "&${cvar}, ${fonts_list_str}")

endforeach()

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts_decls_template
   fonts/fonts_decls.c
   @ONLY
)

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(tfb ${LIB_SOURCES} ${c_font_files} fonts/fonts_decls.c)

add_subdirectory(examples)

# Extra stuff in order to allow a full integration with Tilck's build system
if (${CMAKE_PROJECT_NAME} STREQUAL "tilck")
   tilck_add_extra_app(${CMAKE_CURRENT_BINARY_DIR}/examples/fb_hello_world)
   tilck_add_extra_app(${CMAKE_CURRENT_BINARY_DIR}/examples/fb_drawing)
   tilck_add_extra_app(${CMAKE_CURRENT_BINARY_DIR}/examples/fb_keyinput)
   tilck_add_extra_app(${CMAKE_CURRENT_BINARY_DIR}/examples/fb_text)
   tilck_add_extra_app(${CMAKE_CURRENT_BINARY_DIR}/examples/fb_tetris)
else()
   # For populating pkg-config template
   set(target1 tfb)
   configure_file(tfblib.pc.in tfblib.pc @ONLY)

   install(TARGETS tfb DESTINATION lib)
   install (
     DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
     DESTINATION include
     FILES_MATCHING PATTERN "*.h")
   install(FILES ${CMAKE_BINARY_DIR}/tfblib.pc DESTINATION lib/pkgconfig)
endif()
