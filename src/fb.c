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

int tfb_set_window(u32 x, u32 y, u32 w, u32 h)
{
   if (x + w > __fb_screen_w)
      return TFB_ERR_INVALID_WINDOW;

   if (y + h > __fb_screen_h)
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

void tfb_flush_rect(u32 x, u32 y, u32 w, u32 h)
{
   if (__fb_buffer == __fb_real_buffer)
      return;

   u32 yend;
   x += __fb_off_x;
   y += __fb_off_y;

   w = MIN((int)w, MAX(0, (int)__fb_win_end_x - (int)x));
   yend = MIN(y + h, __fb_win_end_y);

   size_t offset = y * __fb_pitch + (__fb_off_x << 2);
   void *dest = __fb_real_buffer + offset;
   void *src = __fb_buffer + offset;
   u32 rect_pitch = w << 2;

   for (u32 cy = y; cy < yend; cy++, src += __fb_pitch, dest += __fb_pitch)
      memcpy(dest, src, rect_pitch);
}

void tfb_flush_window(void)
{
   tfb_flush_rect(0, 0, __fb_win_w, __fb_win_h);
}
