/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <tfblib/tfblib.h>

void draw_something()
{
   uint32_t w = tfb_screen_width();
   uint32_t h = tfb_screen_height();

   tfb_clear_screen(tfb_black);

   // screen border
   tfb_draw_rect(0, 0, w, h, tfb_white);

   // full rect
   tfb_fill_rect(w/8, h/8 - h/16, w/4, h/4, tfb_red);

   // empty rect
   tfb_draw_rect(w - w/4 - w/8, h/8 - h/16, w/4, h/4, tfb_red);

   // Lines

   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 100, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 75, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 50, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 + 25, tfb_red);

   tfb_draw_line(w/2, h/2, w/2 + 100, h/2, tfb_red);

   tfb_draw_line(w/2, h/2, w/2 + 75, h/2 + 100, tfb_white);
   tfb_draw_line(w/2, h/2, w/2 + 50, h/2 + 100, tfb_white);
   tfb_draw_line(w/2, h/2, w/2 + 25, h/2 + 100, tfb_white);
   tfb_draw_line(w/2, h/2, w/2 + 0, h/2 + 100, tfb_white);


   tfb_draw_line(w/2, h/2, w/2 + 75, h/2 - 100, tfb_green);
   tfb_draw_line(w/2, h/2, w/2 + 50, h/2 - 100, tfb_green);
   tfb_draw_line(w/2, h/2, w/2 + 25, h/2 - 100, tfb_green);
   tfb_draw_line(w/2, h/2, w/2 + 0, h/2 - 100, tfb_green);

   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 25, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 50, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 75, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 + 100, h/2 - 100, tfb_red);

   tfb_draw_line(w/2, h/2, w/2 - 25, h/2 + 100, tfb_blue);
   tfb_draw_line(w/2, h/2, w/2 - 50, h/2 + 100, tfb_blue);
   tfb_draw_line(w/2, h/2, w/2 - 75, h/2 + 100, tfb_blue);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 100, tfb_blue);

   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 75, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 50, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 25, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 + 0, tfb_red);

   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 25, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 50, tfb_red);
   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 75, tfb_red);

   tfb_draw_line(w/2, h/2, w/2 - 100, h/2 - 100, tfb_yellow);
   tfb_draw_line(w/2, h/2, w/2 - 75, h/2 - 100, tfb_yellow);
   tfb_draw_line(w/2, h/2, w/2 - 50, h/2 - 100, tfb_yellow);
   tfb_draw_line(w/2, h/2, w/2 - 25, h/2 - 100, tfb_yellow);
}

void draw_something2(void)
{
   uint32_t w = tfb_screen_width() / 2;
   uint32_t h = tfb_screen_height() / 2;

   tfb_set_center_window_size(w, h);
   tfb_clear_win(tfb_gray);

   tfb_draw_rect(0, 0, w, h, tfb_red);
   tfb_draw_line(0, 0, w * 2, h * 2, tfb_yellow);

   tfb_fill_rect(w/2, h/2, w, h, tfb_green);
}

int main(int argc, char **argv)
{
   int rc;
   rc = tfb_acquire_fb(0, NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      return 1;
   }

   draw_something();
   getchar();
   draw_something2();
   getchar();

   tfb_release_fb();
   return 0;
}
