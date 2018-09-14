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

#define FB_DEVICE "/dev/fb0"
#define TTY_DEVICE "/dev/tty"

#define FB_ASSUMPTION(x)                                        \
   if (!(x)) {                                                  \
      fprintf(stderr, "fb mode assumption '%s' failed\n", #x);  \
      return false;                                             \
   }

struct fb_var_screeninfo __fbi;
struct fb_fix_screeninfo fb_fixinfo;

char *__fb_buffer;
size_t __fb_pitch_div4;

static size_t fb_size;
static size_t fb_pitch;
static int fbfd = -1;
static int ttyfd = -1;

/*
 * Set 'n' 32-bit elems pointed by 's' to 'val'.
 */
static inline void *memset32(void *s, uint32_t val, size_t n)
{
   unsigned unused;

   __asm__ volatile ("rep stosl"
                     : "=D" (unused), "=a" (val), "=c" (n)
                     :  "D" (s), "a" (val), "c" (n)
                     : "cc", "memory");

   return s;
}

uint32_t tfb_make_color(uint8_t red, uint8_t green, uint8_t blue)
{
   return red << __fbi.red.offset |
          green << __fbi.green.offset |
          blue << __fbi.blue.offset;
}

void tfb_clear_screen(uint32_t color)
{
   memset32(__fb_buffer, color, fb_size >> 2);
}

void tfb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
   for (uint32_t cy = y; cy < y + h; cy++)
      memset32(__fb_buffer + cy * fb_pitch + x, color, w);
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

bool tfb_acquire_fb(void)
{
   fbfd = open(FB_DEVICE, O_RDWR);

   if (fbfd < 0) {
      fprintf(stderr, "unable to open '%s'\n", FB_DEVICE);
      return false;
   }

   if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_fixinfo) != 0) {
      fprintf(stderr, "ioctl(FBIOGET_FSCREENINFO) failed\n");
      return false;
   }

   if (ioctl (fbfd, FBIOGET_VSCREENINFO, &__fbi) != 0) {
      fprintf(stderr, "ioctl(FBIOGET_VSCREENINFO) failed\n");
      return false;
   }

   fb_pitch = fb_fixinfo.line_length;
   fb_size = fb_pitch * __fbi.yres;
   __fb_pitch_div4 = fb_pitch >> 2;

   if (!check_fb_assumptions())
      return false;

   ttyfd = open(TTY_DEVICE, O_RDWR);

   if (ttyfd < 0) {
      fprintf(stderr, "Unable to open '%s'\n", TTY_DEVICE);
      return false;
   }

   if (ioctl(ttyfd, KDSETMODE, KD_GRAPHICS) != 0) {
      fprintf(stderr,
              "WARNING: unable set tty into graphics mode on '%s'\n",
              TTY_DEVICE);
   }

   __fb_buffer = mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

   if (__fb_buffer == MAP_FAILED) {
      fprintf(stderr, "Unable to mmap frame__fb_buffer '%s'\n", FB_DEVICE);
      return false;
   }

   return true;
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
