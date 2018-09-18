/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

#include <tfblib/tfblib.h>
#include "utils.h"

#define FB_DEVICE "/dev/fb0"
#define TTY_DEVICE "/dev/tty"

#define FB_ASSUMPTION(x)                                        \
   if (!(x)) {                                                  \
      fprintf(stderr, "fb mode assumption '%s' failed\n", #x);  \
      return false;                                             \
   }

extern inline u32 tfb_make_color(u8 red, u8 green, u8 blue);
extern inline void tfb_draw_pixel(u32 x, u32 y, u32 color);
extern inline void tfb_draw_pixel_raw(u32 x, u32 y, u32 color);
extern inline u32 tfb_screen_width(void);
extern inline u32 tfb_screen_height(void);
extern inline u32 tfb_win_width(void);
extern inline u32 tfb_win_height(void);

struct fb_var_screeninfo __fbi;

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

static int fbfd = -1;
static int ttyfd = -1;

int tfb_set_window(u32 x, u32 y, u32 w, u32 h)
{
   if (x + w > __fbi.xres)
      return TFB_INVALID_WINDOW;

   if (y + h > __fbi.yres)
      return TFB_INVALID_WINDOW;

   __fb_off_x = __fbi.xoffset + x;
   __fb_off_y = __fbi.yoffset + y;
   __fb_win_w = w;
   __fb_win_h = h;
   __fb_win_end_x = __fb_off_x + __fb_win_w;
   __fb_win_end_y = __fb_off_y + __fb_win_h;

   printf("[tfblib debug] win: (%u, %u), (%u, %u)\n",
          __fb_off_x, __fb_off_y, __fb_win_end_x, __fb_win_end_y);

   return TFB_SUCCESS;
}

int tfb_set_center_window_size(u32 w, u32 h)
{
   return tfb_set_window(__fbi.xres / 2 - w / 2,
                         __fbi.yres / 2 - h / 2,
                         w, h);
}

void tfb_clear_screen(u32 color)
{
   if (__fb_pitch == 4 * __fbi.xres) {
      memset32(__fb_buffer, color, __fb_size >> 2);
      return;
   }

   for (u32 y = 0; y < __fbi.yres; y++)
      tfb_draw_hline(0, y, __fbi.xres, color);
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

   len = INT_MIN((int)len, (int)__fb_win_end_x - (int)x);
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

   w = INT_MIN((int)w, (int)__fb_win_end_x - (int)x);
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

static bool check_fb_assumptions(void)
{
   FB_ASSUMPTION(__fbi.bits_per_pixel == 32);
   FB_ASSUMPTION(__fbi.red.msb_right == 0);
   FB_ASSUMPTION(__fbi.green.msb_right == 0);
   FB_ASSUMPTION(__fbi.blue.msb_right == 0);
   return true;
}

int tfb_acquire_fb(void)
{
   static struct fb_fix_screeninfo fb_fixinfo;

   fbfd = open(FB_DEVICE, O_RDWR);

   if (fbfd < 0)
      return TFB_ERROR_OPEN_FB;

   if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_fixinfo) != 0)
      return TFB_ERROR_IOCTL_FB;

   if (ioctl (fbfd, FBIOGET_VSCREENINFO, &__fbi) != 0)
      return TFB_ERROR_IOCTL_FB;

   __fb_pitch = fb_fixinfo.line_length;
   __fb_size = __fb_pitch * __fbi.yres;
   __fb_pitch_div4 = __fb_pitch >> 2;

   if (!check_fb_assumptions())
      return TFB_ASSUMPTION_FAILED;

   ttyfd = open(TTY_DEVICE, O_RDWR);

   if (ttyfd < 0)
      return TFB_ERROR_OPEN_TTY;

   if (ioctl(ttyfd, KDSETMODE, KD_GRAPHICS) != 0)
      return TFB_ERROR_TTY_GRAPHIC_MODE;

   __fb_buffer = mmap(0, __fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

   if (__fb_buffer == MAP_FAILED)
      return TFB_MMAP_FB_ERROR;

   if (tfb_set_window(0, 0, __fbi.xres, __fbi.yres) != TFB_SUCCESS)
      abort(); /* internal error */

   __fb_r_pos = __fbi.red.offset;
   __fb_r_mask_size = __fbi.red.length;
   __fb_r_mask = ((1 << __fb_r_mask_size) - 1) << __fb_r_pos;

   __fb_g_pos = __fbi.green.offset;
   __fb_g_mask_size = __fbi.green.length;
   __fb_g_mask = ((1 << __fb_g_mask_size) - 1) << __fb_g_pos;

   __fb_b_pos = __fbi.blue.offset;
   __fb_b_mask_size = __fbi.blue.length;
   __fb_b_mask = ((1 << __fb_b_mask_size) - 1) << __fb_b_pos;

   return TFB_SUCCESS;
}

void tfb_release_fb(void)
{
   if (__fb_buffer)
      munmap(__fb_buffer, __fb_size);

   if (ttyfd != -1) {
      ioctl(ttyfd, KDSETMODE, KD_TEXT);
      close(ttyfd);
   }

   if (fbfd != -1)
      close(fbfd);
}
