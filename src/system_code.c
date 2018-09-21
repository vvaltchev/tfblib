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

#include <tfblib/tfblib.h>
#include "utils.h"
#include "font.h"

#define DEFAULT_FB_DEVICE "/dev/fb0"
#define DEFAULT_TTY_DEVICE "/dev/tty"

#define FB_ASSUMPTION(x)                                        \
   if (!(x)) {                                                  \
      fprintf(stderr, "fb mode assumption '%s' failed\n", #x);  \
      return false;                                             \
   }

struct fb_var_screeninfo __fbi;

static struct termios orig_termios;
static bool tfb_kb_raw_mode;

static int fbfd = -1;
static int ttyfd = -1;

int tfb_set_window(u32 x, u32 y, u32 w, u32 h)
{
   if (x + w > __fb_screen_w)
      return TFB_INVALID_WINDOW;

   if (y + h > __fb_screen_h)
      return TFB_INVALID_WINDOW;

   __fb_off_x = __fbi.xoffset + x;
   __fb_off_y = __fbi.yoffset + y;
   __fb_win_w = w;
   __fb_win_h = h;
   __fb_win_end_x = __fb_off_x + __fb_win_w;
   __fb_win_end_y = __fb_off_y + __fb_win_h;

   return TFB_SUCCESS;
}

static bool check_fb_assumptions(void)
{
   FB_ASSUMPTION(__fbi.red.msb_right == 0);
   FB_ASSUMPTION(__fbi.green.msb_right == 0);
   FB_ASSUMPTION(__fbi.blue.msb_right == 0);
   return true;
}

int tfb_acquire_fb(u32 flags, const char *fb_device, const char *tty_device)
{
   static struct fb_fix_screeninfo fb_fixinfo;

   if (!fb_device)
      fb_device = DEFAULT_FB_DEVICE;

   if (!tty_device)
      tty_device = DEFAULT_TTY_DEVICE;

   fbfd = open(fb_device, O_RDWR);

   if (fbfd < 0)
      return TFB_ERROR_OPEN_FB;

   if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_fixinfo) != 0)
      return TFB_ERROR_IOCTL_FB;

   if (ioctl (fbfd, FBIOGET_VSCREENINFO, &__fbi) != 0)
      return TFB_ERROR_IOCTL_FB;

   __fb_pitch = fb_fixinfo.line_length;
   __fb_size = __fb_pitch * __fbi.yres;
   __fb_pitch_div4 = __fb_pitch >> 2;

   if (__fbi.bits_per_pixel != 32)
      return TFB_UNSUPPORTED_VIDEO_MODE;

   if (!check_fb_assumptions())
      return TFB_ASSUMPTION_FAILED;

   if (!(flags & TFB_FL_NO_TTY_KD_GRAPHICS)) {

      ttyfd = open(tty_device, O_RDWR);

      if (ttyfd < 0)
         return TFB_ERROR_OPEN_TTY;

      if (ioctl(ttyfd, KDSETMODE, KD_GRAPHICS) != 0)
         return TFB_ERROR_TTY_GRAPHIC_MODE;
   }

   __fb_buffer = mmap(NULL, __fb_size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED, fbfd, 0);

   if (__fb_buffer == MAP_FAILED)
      return TFB_MMAP_FB_ERROR;

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


   if (tfb_set_window(0, 0, __fb_screen_w, __fb_screen_h) != TFB_SUCCESS) {
      fprintf(stderr, "[tfblib] Internal error: tfb_set_window() failed\n");
      abort(); /* internal error */
   }

   /* Just use as default font the first one (if any) */

   if (*tfb_font_file_list)
      tfb_set_default_font((void *)*tfb_font_file_list);

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

int tfb_set_kb_raw_mode(void)
{
   struct termios t;

   if (tfb_kb_raw_mode)
      return TFB_KB_WRONG_MODE;

   if (tcgetattr(0, &orig_termios) != 0)
      return TFB_KB_MODE_GET_FAILED;

   t = orig_termios;
   t.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);
   t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

   if (tcsetattr(0, TCSAFLUSH, &t) != 0)
      return TFB_KB_MODE_SET_FAILED;

   tfb_kb_raw_mode = true;
   return TFB_SUCCESS;
}

int tfb_restore_kb_mode(void)
{
   if (!tfb_kb_raw_mode)
      return TFB_KB_WRONG_MODE;

   if (tcsetattr(0, TCSAFLUSH, &orig_termios) != 0)
      return TFB_KB_MODE_SET_FAILED;

   tfb_kb_raw_mode = false;
   return TFB_SUCCESS;
}

static uint64_t read_esc_seq(void)
{
   char c;
   int len;
   uint64_t ret = 0;

   ret |= '\033';

   if (read(0, &c, 1) <= 0)
      return 0;

   if (c != '[')
      return 0; /* unknown escape sequence */

   ret |= (c << 8);
   len = 2;

   while (1) {

      if (read(0, &c, 1) <= 0)
         return 0;

      ret |= ((uint64_t)c << (8 * len));

      if (0x40 <= c && c <= 0x7E && c != '[')
         break;

      if (len == 8)
         return 0; /* no more space in our 64-bit int (seq too long) */

      len++;
   }

   return ret;
}

uint64_t tfb_read_keypress(void)
{
   int rc;
   char c;

   rc = read(0, &c, 1);

   if (rc <= 0)
      return 0;

   if (c != '\033')
      return c;

   return read_esc_seq();
}
