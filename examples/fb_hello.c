/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <tfblib/tfblib.h>

void draw_something()
{
   u32 w = tfb_screen_width();
   u32 h = tfb_screen_height();

   u32 red = tfb_make_color(255, 0, 0);
   u32 white = tfb_make_color(255, 255, 255);

   tfb_clear_screen(tfb_make_color(0, 0, 0));

   tfb_draw_rect(0, 0, w, h, white);
   tfb_fill_rect(w/8, h/8, w/4, h/4, red);
   tfb_draw_rect(w/2, h/8, w/4, h/4, red);
}

int main(int argc, char **argv)
{
   int rc;

   rc = tfb_acquire_fb();

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   draw_something();
   getchar();

   tfb_release_fb();
   return 0;
}
