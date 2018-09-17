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

#define INT_ABS(_x) ((_x) > 0 ? (_x) : (-(_x)))

extern inline void tfb_draw_pixel(u32 x, u32 y, u32 color);
extern inline u32 tfb_screen_width(void);
extern inline u32 tfb_screen_height(void);

struct fb_var_screeninfo __fbi;

void *__fb_buffer;
size_t __fb_pitch_div4;

static struct fb_fix_screeninfo fb_fixinfo;

static size_t fb_size;
static size_t fb_pitch;
static int fbfd = -1;
static int ttyfd = -1;


u32 tfb_make_color(u8 red, u8 green, u8 blue)
{
   return red << __fbi.red.offset |
          green << __fbi.green.offset |
          blue << __fbi.blue.offset;
}

void tfb_clear_screen(u32 color)
{
   if (fb_pitch == 4 * __fbi.xres) {
      memset32(__fb_buffer, color, fb_size >> 2);
      return;
   }

   for (u32 y = 0; y < __fbi.yres; y++)
      tfb_draw_hline(0, y, __fbi.xres, color);
}

void tfb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
   for (u32 cy = y; cy < y + h; cy++)
      memset32(__fb_buffer + cy * fb_pitch + (x << 2), color, w);
}

void tfb_draw_hline(u32 x, u32 y, u32 len, u32 color)
{
   memset32(__fb_buffer + y * fb_pitch + (x << 2), color, len);
}

void tfb_draw_vline(u32 x, u32 y, u32 len, u32 color)
{
   volatile u32 *buf =
      ((volatile u32 *) __fb_buffer) + y * __fb_pitch_div4 + x;

   for (u32 cy = y; cy < y + len; cy++, buf += __fb_pitch_div4)
      *buf = color;
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

void tfb_draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color)
{
   const int dx = INT_ABS((int)x1 - (int)x0);
   const int dy = INT_ABS((int)y1 - (int)y0);

   if (dy <= dx)
      midpoint_line(x0, y0, x1, y1, color, false);
   else
      midpoint_line(y0, x0, y1, x1, color, true);
}

void tfb_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
   tfb_draw_hline(x, y, w, color);
   tfb_draw_vline(x, y, h, color);
   tfb_draw_vline(x + w - 1, y, h, color);
   tfb_draw_hline(x, y + h - 1, w, color);
}

static bool check_fb_assumptions(void)
{
   FB_ASSUMPTION(__fbi.bits_per_pixel == 32);

   FB_ASSUMPTION((__fbi.red.offset % 8) == 0);
   FB_ASSUMPTION((__fbi.green.offset % 8) == 0);
   FB_ASSUMPTION((__fbi.blue.offset % 8) == 0);
   FB_ASSUMPTION((__fbi.transp.offset % 8) == 0);

   FB_ASSUMPTION(__fbi.red.length == 8);
   FB_ASSUMPTION(__fbi.green.length == 8);
   FB_ASSUMPTION(__fbi.blue.length == 8);
   FB_ASSUMPTION(__fbi.transp.length == 0);

   FB_ASSUMPTION(__fbi.xoffset == 0);
   FB_ASSUMPTION(__fbi.yoffset == 0);

   FB_ASSUMPTION(__fbi.red.msb_right == 0);
   FB_ASSUMPTION(__fbi.green.msb_right == 0);
   FB_ASSUMPTION(__fbi.blue.msb_right == 0);

   return true;
}

int tfb_acquire_fb(void)
{
   fbfd = open(FB_DEVICE, O_RDWR);

   if (fbfd < 0)
      return TFB_ERROR_OPEN_FB;

   if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_fixinfo) != 0)
      return TFB_ERROR_IOCTL_FB;

   if (ioctl (fbfd, FBIOGET_VSCREENINFO, &__fbi) != 0)
      return TFB_ERROR_IOCTL_FB;

   fb_pitch = fb_fixinfo.line_length;
   fb_size = fb_pitch * __fbi.yres;
   __fb_pitch_div4 = fb_pitch >> 2;

   if (!check_fb_assumptions())
      return TFB_ASSUMPTION_FAILED;

   ttyfd = open(TTY_DEVICE, O_RDWR);

   if (ttyfd < 0)
      return TFB_ERROR_OPEN_TTY;

   if (ioctl(ttyfd, KDSETMODE, KD_GRAPHICS) != 0)
      return TFB_ERROR_TTY_GRAPHIC_MODE;

   __fb_buffer = mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

   if (__fb_buffer == MAP_FAILED)
      return TFB_MMAP_FB_ERROR;

   return TFB_SUCCESS;
}

void tfb_release_fb(void)
{
   if (__fb_buffer)
      munmap(__fb_buffer, fb_size);

   if (ttyfd != -1) {
      ioctl(ttyfd, KDSETMODE, KD_TEXT);
      close(ttyfd);
   }

   if (fbfd != -1)
      close(fbfd);
}
