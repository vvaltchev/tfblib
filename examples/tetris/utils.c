/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

uint32_t red, green, blue, white, black, yellow, gray, magenta, cyan;

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
}

static bool font_iter_cb_select_font(tfb_font_info *fi, void *user_arg)
{
   if (fi->height == (uint32_t)user_arg) {

      int rc = tfb_set_current_font(fi->font_id);

      if (rc != TFB_SUCCESS) {
         fprintf(stderr, "tfb_set_current_font() failed with error: %d\n", rc);
      }

      return false; /* stop iterating over fonts */
   }

   return true;
}

void set_fb_font(void)
{
   tfb_iterate_over_fonts(font_iter_cb_select_font, (void *)16);
}
