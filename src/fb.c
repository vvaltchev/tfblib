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
#include <termios.h>
#include <errno.h>

#include <tfblib/tfblib.h>
#include "utils.h"
#include "font.h"

#define DEFAULT_FB_DEVICE "/dev/fb0"
#define DEFAULT_TTY_DEVICE "/dev/tty"

struct fb_var_screeninfo __fbi;
int __tfb_ttyfd = -1;

static int fbfd = -1;

static void tfb_init_colors(void);

int tfb_set_window(u32 x, u32 y, u32 w, u32 h)
{
   if (x + w > (u32)__fb_screen_w)
      return TFB_ERR_INVALID_WINDOW;

   if (y + h > (u32)__fb_screen_h)
      return TFB_ERR_INVALID_WINDOW;

   __fb_off_x = __fbi.xoffset + x;
   __fb_off_y = __fbi.yoffset + y;
   __fb_win_w = w;
   __fb_win_h = h;
   __fb_win_end_x = __fb_off_x + __fb_win_w;
   __fb_win_end_y = __fb_off_y + __fb_win_h;

   return TFB_SUCCESS;
}

int tfb_acquire_fb(u32 flags, const char *fb_device, const char *tty_device)
{
   static struct fb_fix_screeninfo fb_fixinfo;

   int ret = TFB_SUCCESS;

   if (!fb_device)
      fb_device = DEFAULT_FB_DEVICE;

   if (!tty_device)
      tty_device = DEFAULT_TTY_DEVICE;

   fbfd = open(fb_device, O_RDWR);

   if (fbfd < 0) {
      ret = TFB_ERR_OPEN_FB;
      goto out;
   }

   if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_fixinfo) != 0) {
      ret = TFB_ERR_IOCTL_FB;
      goto out;
   }

   if (ioctl(fbfd, FBIOGET_VSCREENINFO, &__fbi) != 0) {
      ret = TFB_ERR_IOCTL_FB;
      goto out;
   }

   __fb_pitch = fb_fixinfo.line_length;
   __fb_size = __fb_pitch * __fbi.yres;
   __fb_pitch_div4 = __fb_pitch >> 2;

   if (__fbi.bits_per_pixel != 32) {
      ret = TFB_ERR_UNSUPPORTED_VIDEO_MODE;
      goto out;
   }

   if (__fbi.red.msb_right || __fbi.green.msb_right || __fbi.blue.msb_right) {
      ret = TFB_ERR_UNSUPPORTED_VIDEO_MODE;
      goto out;
   }

   __tfb_ttyfd = open(tty_device, O_RDWR);

   if (__tfb_ttyfd < 0) {
      ret = TFB_ERR_OPEN_TTY;
      goto out;
   }

   if (!(flags & TFB_FL_NO_TTY_KD_GRAPHICS)) {

      if (ioctl(__tfb_ttyfd, KDSETMODE, KD_GRAPHICS) != 0) {
         ret = TFB_ERR_TTY_GRAPHIC_MODE;
         goto out;
      }
   }

   __fb_real_buffer = mmap(NULL, __fb_size,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED, fbfd, 0);

   if (__fb_real_buffer == MAP_FAILED) {
      ret = TFB_ERR_MMAP_FB;
      goto out;
   }

   if (flags & TFB_FL_USE_DOUBLE_BUFFER) {

      __fb_buffer = malloc(__fb_size);

      if (!__fb_buffer) {
         ret = TFB_ERR_OUT_OF_MEMORY;
         goto out;
      }

   } else {

      __fb_buffer = __fb_real_buffer;
   }

   __fb_screen_w = __fbi.xres;
   __fb_screen_h = __fbi.yres;

   __fb_r_pos = __fbi.red.offset;
   __fb_r_mask_size = __fbi.red.length;
   __fb_r_mask = ((1 << __fb_r_mask_size) - 1) << __fb_r_pos;

   __fb_g_pos = __fbi.green.offset;
   __fb_g_mask_size = __fbi.green.length;
   __fb_g_mask = ((1 << __fb_g_mask_size) - 1) << __fb_g_pos;

   __fb_b_pos = __fbi.blue.offset;
   __fb_b_mask_size = __fbi.blue.length;
   __fb_b_mask = ((1 << __fb_b_mask_size) - 1) << __fb_b_pos;

   tfb_set_window(0, 0, __fb_screen_w, __fb_screen_h);
   tfb_init_colors();

   /* Just use as default font the first one (if any) */
   if (*tfb_font_file_list)
      tfb_set_default_font((void *)*tfb_font_file_list);

out:
   if (ret != TFB_SUCCESS)
      tfb_release_fb();

   return ret;
}

void tfb_release_fb(void)
{
   if (__fb_real_buffer)
      munmap(__fb_real_buffer, __fb_size);

   if (__fb_buffer != __fb_real_buffer)
      free(__fb_buffer);

   if (__tfb_ttyfd != -1) {
      ioctl(__tfb_ttyfd, KDSETMODE, KD_TEXT);
      close(__tfb_ttyfd);
   }

   if (fbfd != -1)
      close(fbfd);
}

void tfb_flush_rect(int x, int y, int w, int h)
{
   int yend;

   if (__fb_buffer == __fb_real_buffer)
      return;

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

   w = MIN(w, MAX(0, __fb_win_end_x - x));
   yend = MIN(y + h, __fb_win_end_y);

   size_t offset = y * __fb_pitch + (__fb_off_x << 2);
   void *dest = __fb_real_buffer + offset;
   void *src = __fb_buffer + offset;
   u32 rect_pitch = w << 2;

   for (int cy = y; cy < yend; cy++, src += __fb_pitch, dest += __fb_pitch)
      memcpy(dest, src, rect_pitch);
}

void tfb_flush_window(void)
{
   tfb_flush_rect(0, 0, __fb_win_w, __fb_win_h);
}

int tfb_flush_fb(void)
{ 
   __fbi.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
   if(ioctl(fbfd, FBIOPUT_VSCREENINFO, &__fbi) < 0) {
      return TFB_ERR_FB_FLUSH_IOCTL_FAILED;
   }

   return TFB_SUCCESS;
}

/*
 * ----------------------------------------------------------------------------
 *
 * Colors
 *
 * ----------------------------------------------------------------------------
 */

uint32_t tfb_red;
uint32_t tfb_darkred;
uint32_t tfb_pink;
uint32_t tfb_deeppink;
uint32_t tfb_orange;
uint32_t tfb_darkorange;
uint32_t tfb_gold;
uint32_t tfb_yellow;
uint32_t tfb_violet;
uint32_t tfb_magenta;
uint32_t tfb_darkviolet;
uint32_t tfb_indigo;
uint32_t tfb_lightgreen;
uint32_t tfb_green;
uint32_t tfb_darkgreen;
uint32_t tfb_olive;
uint32_t tfb_cyan;
uint32_t tfb_lightblue;
uint32_t tfb_blue;
uint32_t tfb_darkblue;
uint32_t tfb_brown;
uint32_t tfb_maroon;
uint32_t tfb_white;
uint32_t tfb_lightgray;
uint32_t tfb_gray;
uint32_t tfb_darkgray;
uint32_t tfb_silver;
uint32_t tfb_black;
uint32_t tfb_purple;

static void tfb_init_colors(void)
{
   tfb_red = tfb_make_color(255, 0, 0);
   tfb_darkred = tfb_make_color(139, 0, 0);
   tfb_pink = tfb_make_color(255, 192, 203);
   tfb_deeppink = tfb_make_color(255, 20, 147);
   tfb_orange = tfb_make_color(255, 165, 0);
   tfb_darkorange = tfb_make_color(255, 140, 0);
   tfb_gold = tfb_make_color(255, 215, 0);
   tfb_yellow = tfb_make_color(255, 255, 0);
   tfb_violet = tfb_make_color(238, 130, 238);
   tfb_magenta = tfb_make_color(255, 0, 255);
   tfb_darkviolet = tfb_make_color(148, 0, 211);
   tfb_indigo = tfb_make_color(75, 0, 130);
   tfb_lightgreen = tfb_make_color(144, 238, 144);
   tfb_green = tfb_make_color(0, 255, 0);
   tfb_darkgreen = tfb_make_color(0, 100, 0);
   tfb_olive = tfb_make_color(128, 128, 0);
   tfb_cyan = tfb_make_color(0, 255, 255);
   tfb_lightblue = tfb_make_color(173, 216, 230);
   tfb_blue = tfb_make_color(0, 0, 255);
   tfb_darkblue = tfb_make_color(0, 0, 139);
   tfb_brown = tfb_make_color(165, 42, 42);
   tfb_maroon = tfb_make_color(128, 0, 0);
   tfb_white = tfb_make_color(255, 255, 255);
   tfb_lightgray = tfb_make_color(211, 211, 211);
   tfb_gray = tfb_make_color(128, 128, 128);
   tfb_darkgray = tfb_make_color(169, 169, 169);
   tfb_silver = tfb_make_color(192, 192, 192);
   tfb_black = tfb_make_color(0, 0, 0);
   tfb_purple = tfb_make_color(128, 0, 128);
}
