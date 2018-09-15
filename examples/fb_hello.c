/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <tfblib/tfblib.h>

void draw_something()
{
   u32 w = tfb_screen_width();
   u32 h = tfb_screen_height();

   u32 red = tfb_make_color(255, 0, 0);
   u32 green = tfb_make_color(0, 255, 0);
   u32 blue = tfb_make_color(0, 0, 255);
   u32 white = tfb_make_color(255, 255, 255);
   u32 yellow = tfb_make_color(255, 255, 0);

   tfb_clear_screen(tfb_make_color(0, 0, 0));

   // screen border
   tfb_draw_rect(0, 0, w, h, white);

   // full rect
   tfb_fill_rect(w/8, h/8 - h/16, w/4, h/4, red);

   // empty rect
   tfb_draw_rect(w/2, h/8 - h/16, w/4, h/4, red);

   // Lines

   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 100, red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 75, red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 50, red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 25, red);

   tfb_draw_line(w/2, h/2, w/2 + 100, h/2, red);

   tfb_draw_line(w/2, h/2, w/2 + 75, h/2 + 100, white);
   tfb_draw_line(w/2, h/2, w/2 + 50, h/2 + 100, white);
   tfb_draw_line(w/2, h/2, w/2 + 25, h/2 + 100, white);
   tfb_draw_line(w/2, h/2, w/2 + 0, h/2 + 100, white);


   tfb_draw_line(w/2, h/2, w/2 + 75, h/2 - 100, green);
   tfb_draw_line(w/2, h/2, w/2 + 50, h/2 - 100, green);
   tfb_draw_line(w/2, h/2, w/2 + 25, h/2 - 100, green);
   tfb_draw_line(w/2, h/2, w/2 + 0, h/2 - 100, green);

   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 25, red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 50, red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 75, red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 100, red);

   tfb_draw_line(w/2, h/2, w/2 - 25, h/2 + 100, blue);
   tfb_draw_line(w/2, h/2, w/2 - 50, h/2 + 100, blue);
   tfb_draw_line(w/2, h/2, w/2 - 75, h/2 + 100, blue);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 100, blue);

   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 75, red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 50, red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 25, red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 0, red);

   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 25, red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 50, red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 75, red);

   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 100, yellow);
   tfb_draw_line(w/2, h/2, w/2 - 75, h/2 - 100, yellow);
   tfb_draw_line(w/2, h/2, w/2 - 50, h/2 - 100, yellow);
   tfb_draw_line(w/2, h/2, w/2 - 25, h/2 - 100, yellow);
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
