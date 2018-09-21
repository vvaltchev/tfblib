/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include <tfblib/tfblib.h>
#include "utils.h"

extern inline u32 tfb_make_color(u8 red, u8 green, u8 blue);
extern inline void tfb_draw_pixel(u32 x, u32 y, u32 color);
extern inline void tfb_draw_pixel_raw(u32 x, u32 y, u32 color);
extern inline u32 tfb_screen_width(void);
extern inline u32 tfb_screen_height(void);
extern inline u32 tfb_win_width(void);
extern inline u32 tfb_win_height(void);

void *__fb_buffer;
size_t __fb_size;
size_t __fb_pitch;
size_t __fb_pitch_div4; /*
                         * Used in tfb_draw_pixel* to save a (x << 2) operation.
                         * If we had to use __fb_pitch, we'd had to write:
                         *    *(u32 *)(__fb_buffer + (x << 2) + y * __fb_pitch)
                         * which clearly requires an additional shift operation
                         * that we can skip by using __fb_pitch_div4.
                         */

u32 __fb_screen_w;
u32 __fb_screen_h;
u32 __fb_win_w;
u32 __fb_win_h;
u32 __fb_off_x;
u32 __fb_off_y;
u32 __fb_win_end_x;
u32 __fb_win_end_y;

u32 __fb_r_mask;
u32 __fb_g_mask;
u32 __fb_b_mask;
u8 __fb_r_mask_size;
u8 __fb_g_mask_size;
u8 __fb_b_mask_size;
u8 __fb_r_pos;
u8 __fb_g_pos;
u8 __fb_b_pos;

int tfb_set_center_window_size(u32 w, u32 h)
{
   return tfb_set_window(__fb_screen_w / 2 - w / 2,
                         __fb_screen_h / 2 - h / 2,
                         w, h);
}

void tfb_clear_screen(u32 color)
{
   if (__fb_pitch == 4 * __fb_screen_w) {
      memset32(__fb_buffer, color, __fb_size >> 2);
      return;
   }

   for (u32 y = 0; y < __fb_screen_h; y++)
      tfb_draw_hline(0, y, __fb_screen_w, color);
}

void tfb_clear_win(u32 color)
{
   for (u32 y = 0; y < __fb_win_h; y++)
      tfb_draw_hline(0, y, __fb_win_w, color);
}

void tfb_draw_hline(u32 x, u32 y, u32 len, u32 color)
{
   x += __fb_off_x;
   y += __fb_off_y;

   if (y >= __fb_win_end_y)
      return;

   len = INT_MIN((int)len, INT_MAX(0, (int)__fb_win_end_x - (int)x));
   memset32(__fb_buffer + y * __fb_pitch + (x << 2), color, len);
}

void tfb_draw_vline(u32 x, u32 y, u32 len, u32 color)
{
   u32 yend;

   x += __fb_off_x;
   y += __fb_off_y;
   yend = INT_MIN(y + len, __fb_win_end_y);

   volatile u32 *buf =
      ((volatile u32 *) __fb_buffer) + y * __fb_pitch_div4 + x;

   for (; y < yend; y++, buf += __fb_pitch_div4)
      *buf = color;
}

void tfb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
   u32 yend;

   x += __fb_off_x;
   y += __fb_off_y;

   w = INT_MIN((int)w, INT_MAX(0, (int)__fb_win_end_x - (int)x));
   yend = INT_MIN(y + h, __fb_win_end_y);

   for (u32 cy = y; cy < yend; cy++)
      memset32(__fb_buffer + cy * __fb_pitch + (x << 2), color, w);
}

void tfb_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
   tfb_draw_hline(x, y, w, color);
   tfb_draw_vline(x, y, h, color);
   tfb_draw_vline(x + w - 1, y, h, color);
   tfb_draw_hline(x, y + h - 1, w, color);
}

static void
midpoint_line(int x, int y, int x1, int y1, u32 color, bool swap_xy)
{
   const int dx = INT_ABS(x1 - x);
   const int dy = INT_ABS(y1 - y);
   const int sx = x1 > x ? 1 : -1;
   const int sy = y1 > y ? 1 : -1;
   const int incE = dy << 1;
   const int incNE = (dy - dx) << 1;
   const int inc_d[2] = {incNE, incE};
   const int inc_y[2] = {sy, 0};

   int d = (dy << 1) - dx;

   if (swap_xy) {

      tfb_draw_pixel_raw(y, x, color);

      while (x != x1) {
         x += sx;
         y += inc_y[d <= 0];
         d += inc_d[d <= 0];
         tfb_draw_pixel_raw(y, x, color);
      }

   } else {

      tfb_draw_pixel_raw(x, y, color);

      while (x != x1) {
         x += sx;
         y += inc_y[d <= 0];
         d += inc_d[d <= 0];
         tfb_draw_pixel_raw(x, y, color);
      }
   }
}

void tfb_draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color)
{
   x0 = INT_MIN(x0 + __fb_off_x, __fb_win_end_x);
   y0 = INT_MIN(y0 + __fb_off_y, __fb_win_end_y);
   x1 = INT_MIN(x1 + __fb_off_x, __fb_win_end_x);
   y1 = INT_MIN(y1 + __fb_off_y, __fb_win_end_y);

   const int dx = INT_ABS((int)x1 - (int)x0);
   const int dy = INT_ABS((int)y1 - (int)y0);

   if (dy <= dx)
      midpoint_line(x0, y0, x1, y1, color, false);
   else
      midpoint_line(y0, x0, y1, x1, color, true);
}
