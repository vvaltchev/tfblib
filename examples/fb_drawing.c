/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_colors.h>

void draw_something(void)
{
   uint32_t w = tfb_screen_width();
   uint32_t h = tfb_screen_height();

   tfb_clear_screen(tfb_black);

   // screen border
   tfb_draw_rect(0, 0, w, h, tfb_white);

   uint32_t l = (w > h ? w : h) / 4;
   uint32_t cx = w/2;
   uint32_t cy = h/2;

   const double full_circle = 2.0 * M_PI;
   const double delta_ang = full_circle / 64.0;

   for (double ang = 0.0; (full_circle - ang) > delta_ang; ang += delta_ang) {

      uint32_t px = cx + cos(ang) * l;
      uint32_t py = cy + sin(ang) * l;

      tfb_draw_line(cx, cy, px, py, tfb_red);
      tfb_draw_rect(px - 10, py - 10, 20, 20, tfb_white);
   }
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
