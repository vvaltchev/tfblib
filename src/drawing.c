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
extern inline u32 tfb_screen_width(void);
extern inline u32 tfb_screen_height(void);
extern inline u32 tfb_win_width(void);
extern inline u32 tfb_win_height(void);

void *__fb_buffer;
void *__fb_real_buffer;
size_t __fb_size;
size_t __fb_pitch;
size_t __fb_pitch_div4; /*
                         * Used in tfb_draw_pixel* to save a (x << 2) operation.
                         * If we had to use __fb_pitch, we'd had to write:
                         *    *(u32 *)(__fb_buffer + (x << 2) + y * __fb_pitch)
                         * which clearly requires an additional shift operation
                         * that we can skip by using __fb_pitch_div4 + an early
                         * cast to u32.
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

static inline void tfb_draw_pixel_raw(u32 x, u32 y, u32 color)
{
   ((volatile u32 *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

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
   tfb_fill_rect(0, 0, __fb_win_w, __fb_win_h, color);
}

void tfb_draw_hline(int x, int y, int len, u32 color)
{
   if (x < 0) {
      len += x;
      x = 0;
   }

   x += __fb_off_x;
   y += __fb_off_y;

   if (len < 0 || y < __fb_off_y || y >= __fb_win_end_y)
      return;

   len = MIN(len, MAX(0, (int)__fb_win_end_x - x));
   memset32(__fb_buffer + y * __fb_pitch + (x << 2), color, len);
}

void tfb_draw_vline(int x, int y, int len, u32 color)
{
   u32 yend;

   if (y < 0) {
      len += y;
      y = 0;
   }

   x += __fb_off_x;
   y += __fb_off_y;

   if (len < 0 || x < __fb_off_x || x >= __fb_win_end_x)
      return;

   yend = MIN(y + len, __fb_win_end_y);

   volatile u32 *buf =
      ((volatile u32 *) __fb_buffer) + y * __fb_pitch_div4 + x;

   for (; y < yend; y++, buf += __fb_pitch_div4)
      *buf = color;
}

void tfb_fill_rect(int x, int y, int w, int h, u32 color)
{
   u32 yend;
   void *dest;

   x += __fb_off_x;
   y += __fb_off_y;

   if (x < 0) {
      w += x;
      x = 0;
   }

   if (y < 0) {
      h += y;
      y = 0;
   }

   if (w < 0 || h < 0)
      return;

   w = MIN(w, MAX(0, (int)__fb_win_end_x - x));
   yend = MIN(y + h, __fb_win_end_y);

   dest = __fb_buffer + y * __fb_pitch + (x << 2);

   for (u32 cy = y; cy < yend; cy++, dest += __fb_pitch)
      memset32(dest, color, w);
}

void tfb_draw_rect(int x, int y, int w, int h, u32 color)
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

      tfb_draw_pixel(y, x, color);

      while (x != x1) {
         x += sx;
         y += inc_y[d <= 0];
         d += inc_d[d <= 0];
         tfb_draw_pixel(y, x, color);
      }

   } else {

      tfb_draw_pixel(x, y, color);

      while (x != x1) {
         x += sx;
         y += inc_y[d <= 0];
         d += inc_d[d <= 0];
         tfb_draw_pixel(x, y, color);
      }
   }
}

void tfb_draw_line(int x0, int y0, int x1, int y1, u32 color)
{
   if (INT_ABS(y1 - y0) <= INT_ABS(x1 - x0))
      midpoint_line(x0, y0, x1, y1, color, false);
   else
      midpoint_line(y0, x0, y1, x1, color, true);
}
