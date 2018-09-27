/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

u32 red, green, blue, white, black, yellow, gray, magenta, cyan, orange, purple;

void init_colors(void)
{
   white = tfb_make_color(255, 255, 255);
   black = tfb_make_color(0, 0, 0);
   red = tfb_make_color(255, 0, 0);
   green = tfb_make_color(0, 255, 0);
   blue = tfb_make_color(0, 0, 255);
   yellow = tfb_make_color(255, 255, 0);
   gray = tfb_make_color(50, 50, 50);
   magenta = tfb_make_color(255, 0, 255);
   cyan = tfb_make_color(0, 255, 255);
   orange = tfb_make_color(0xff, 0xa7, 0x00);
   purple = tfb_make_color(0x8c, 0x00, 0x81);
}

