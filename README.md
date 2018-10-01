# Tfblib (Tiny Framebuffer Library)

[![Build Status](https://travis-ci.org/vvaltchev/tfblib.svg?branch=master)](https://travis-ci.org/vvaltchev/tfblib)

`Tfblib` is a simple and low-level graphics library for drawing to the Linux
framebuffer. Currently, it is capable of drawing lines, rectangles and text.
It has support both for embedded (compiled-in) fonts in the library and
dynamically loaded `PSF` fonts at runtime. In addition to drawing functions,
`Tfblib` has a minimal support for keyboard input that allows simple
applications to put the TTY input in *raw mode* and read keystrokes. Both
blocking and non-blocking modes are supported.

Building
---------

Building `Tfblib` as a static library is simple as executing (in project's root
directory):

    - mkdir build
    - cd build
    - cmake ..
    - make

The `make` command will build the library along with the programs in the
examples/ directory. In case a release build (with optimizations) is desired,
the `cmake` command has to be run this way (assuming the current working
directory is the build directory):

    cmake -DCMAKE_BUILD_TYPE=Release ..

Or:

    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

In case a release build with debug info is desired.


Documentation
---------------

Libray's API is documented using `Doxygen`. In order to generate the
documentation, make sure you have `doxygen` installed on your system and just
run (in project's root directory):

    doxygen

The output html files will be placed in `<PROJECT_ROOT_DIR>/doxydocs/html`.
Just open `index.html` with your browser.
