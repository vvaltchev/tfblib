/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_colors.h>

static inline double rad_to_deg(double rad)
{
   return (rad / (2 * M_PI)) * 360.0;
}

void draw_something(void)
{
   uint32_t w = tfb_screen_width();
   uint32_t h = tfb_screen_height();

   tfb_set_font_by_size(8, 16);
   tfb_clear_screen(tfb_black);
   tfb_draw_string(10, 10, tfb_white, tfb_black, "Press ENTER for the next");

   // screen border
   tfb_draw_rect(0, 0, w, h, tfb_white);

   uint32_t l = (w < h ? w : h) * 45 / 100;
   uint32_t cx = w/2;
   uint32_t cy = h/2;

   const double full_circle = 2.0 * M_PI;
   const double delta_ang = full_circle / 120.0;

   for (double ang = 0.0; ang < full_circle; ang += delta_ang) {

      uint32_t px = cx + cos(ang) * l;
      uint32_t py = cy + sin(ang) * l;
      uint32_t color =
         tfb_make_color_hsv(TFB_HUE_DEGREE * rad_to_deg(ang), 255, 255);

      tfb_draw_line(cx, cy, px, py, color);
      tfb_fill_rect(px - 10, py - 10, 20, 20, color);
   }
}

void draw_circles(void)
{
   uint32_t w = tfb_screen_width();

   tfb_clear_screen(tfb_black);
   tfb_set_font_by_size(8, 16);

   int n = 20;
   int r = w / n;

   for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
         if ((i+j) % 2)
            tfb_draw_circle(i * r * 2, j * r * 2, r, tfb_red);
         else
            tfb_fill_circle(i * r * 2, j * r * 2, r, tfb_red);
      }
   }

   tfb_draw_string(10, 10, tfb_white, tfb_black, "Press ENTER to quit");
}

int main(int argc, char **argv)
{
   int rc;
   rc = tfb_acquire_fb(0, NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed: %s\n", tfb_strerror(rc));
      return 1;
   }

   draw_something();
   getchar();
   draw_circles();
   getchar();

   tfb_release_fb();
   return 0;
}
