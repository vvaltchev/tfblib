/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <tfblib/tfblib.h>

uint32_t red, green, blue, white, black, yellow, gray;

void init_colors(void)
{
   white = tfb_make_color(255, 255, 255);
   black = tfb_make_color(0, 0, 0);
   red = tfb_make_color(255, 0, 0);
   green = tfb_make_color(0, 255, 0);
   blue = tfb_make_color(0, 0, 255);
   yellow = tfb_make_color(255, 255, 0);
   gray = tfb_make_color(50, 50, 50);
}

void draw_something()
{
   uint32_t w = tfb_screen_width();
   uint32_t h = tfb_screen_height();

   tfb_clear_screen(black);
   tfb_fill_rect(w/8, h/8 - h/16, w/4, h/4, red);
}

int main(int argc, char **argv)
{
   int rc;
   rc = tfb_acquire_fb(NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   init_colors();

   rc = tfb_set_kb_raw_mode();

   if (rc != TFB_SUCCESS)
      fprintf(stderr, "tfb_set_kb_raw_mode() failed with err: %d", rc);

   draw_something();
   getchar();

   tfb_restore_kb_mode();
   tfb_release_fb();
   return 0;
}
