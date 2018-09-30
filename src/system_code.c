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

#define FB_ASSUMPTION(x)                                        \
   if (!(x)) {                                                  \
      fprintf(stderr, "fb mode assumption '%s' failed\n", #x);  \
      return false;                                             \
   }

struct fb_var_screeninfo __fbi;

static struct termios orig_termios;
static bool tfb_kb_raw_mode;
static bool tfb_kb_nonblock;
static int tfb_saved_kdmode;
static u32 tfb_kb_saved_fcntl_flags;

static int fbfd = -1;
static int ttyfd = -1;

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

   ttyfd = open(tty_device, O_RDWR);

   if (ttyfd < 0) {
      ret = TFB_ERR_OPEN_TTY;
      goto out;
   }

   if (!(flags & TFB_FL_NO_TTY_KD_GRAPHICS)) {

      if (ioctl(ttyfd, KDSETMODE, KD_GRAPHICS) != 0) {
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

   if (ttyfd != -1) {
      ioctl(ttyfd, KDSETMODE, KD_TEXT);
      close(ttyfd);
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

int tfb_set_kb_raw_mode(u32 flags)
{
   struct termios t;
   int rc;

   if (tfb_kb_raw_mode)
      return TFB_ERR_KB_WRONG_MODE;

   if (ioctl(ttyfd, KDGKBMODE, &tfb_saved_kdmode) != 0)
      return TFB_ERR_KB_MODE_GET_FAILED;

   if (tfb_saved_kdmode != K_XLATE) {
      if (ioctl(ttyfd, KDSKBMODE, K_XLATE) != 0)
         return TFB_ERR_KB_MODE_SET_FAILED;
   }

   if (tcgetattr(ttyfd, &orig_termios) != 0)
      return TFB_ERR_KB_MODE_GET_FAILED;

   t = orig_termios;
   t.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);
   t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

   if (tcsetattr(ttyfd, TCSAFLUSH, &t) != 0)
      return TFB_ERR_KB_MODE_SET_FAILED;

   tfb_kb_raw_mode = true;

   if (flags & TFB_FL_KB_NONBLOCK) {

      rc = fcntl(ttyfd, F_GETFL, 0);

      if (rc < 0) {
         tfb_restore_kb_mode();
         return TFB_ERR_KB_MODE_GET_FAILED;
      }

      tfb_kb_saved_fcntl_flags  = rc;

      rc = fcntl(ttyfd, F_SETFL, tfb_kb_saved_fcntl_flags | O_NONBLOCK);

      if (rc < 0) {
         tfb_restore_kb_mode();
         return TFB_ERR_KB_MODE_SET_FAILED;
      }

      tfb_kb_nonblock = true;
   }

   return TFB_SUCCESS;
}

int tfb_restore_kb_mode(void)
{
   if (tfb_kb_nonblock) {

      /*
       * Restore the original flags.
       * NOTE: ignoring any eventual error from fcntl() since we have to try
       * anyway to restore the tty in canonical mode (with tcsetattr() below).
       */

      fcntl(ttyfd, F_SETFL, tfb_kb_saved_fcntl_flags);
      tfb_kb_nonblock = false;
   }

  if (!tfb_kb_raw_mode)
      return TFB_ERR_KB_WRONG_MODE;

   /* Restore the original kb mode. Note: ignoring any failure */
  ioctl(ttyfd, KDSKBMODE, &tfb_saved_kdmode);

   if (tcsetattr(ttyfd, TCSAFLUSH, &orig_termios) != 0)
      return TFB_ERR_KB_MODE_SET_FAILED;

   tfb_kb_raw_mode = false;
   return TFB_SUCCESS;
}

static struct {

   enum {
      NB_INITIAL_STATE,
      NB_AFTER_ESC_READ,
      NB_AFTER_OPEN_BRACKET_READ
   } state;

   int len;
   int readbuf_len;

   union {
      tfb_key_t key;
      char buf[sizeof(tfb_key_t)];
   };

   char readbuf[sizeof(tfb_key_t)];

} nb_ctx;

static inline void nb_ctx_append(char c)
{
   nb_ctx.buf[nb_ctx.len++] = c;
}

static tfb_key_t nb_handle_initial_state(int rc, char c)
{
   nb_ctx.len = 0;
   nb_ctx.key = 0;

   if (rc <= 0)
      return 0;

   if (c != '\033')
      return c; /* remain in the initial state */

   nb_ctx_append(c);
   nb_ctx.state = NB_AFTER_ESC_READ;
   return 0;
}

static tfb_key_t nb_handle_after_esc_state(int rc, char c)
{
   if (rc <= 0) {

      if (errno != EAGAIN)
         nb_ctx.state = NB_INITIAL_STATE;

      return 0;
   }

   if (c != '[') {

      /* unknown escape sequence */
      nb_ctx.state = NB_INITIAL_STATE;
      return 0;
   }

   /* c is '[' */
   nb_ctx_append(c);
   nb_ctx.state = NB_AFTER_OPEN_BRACKET_READ;
   return 0;
}

static tfb_key_t nb_handle_after_open_bracket_state(int rc, char c)
{
   if (rc < 0) {

      if (errno != EAGAIN)
         nb_ctx.state = NB_INITIAL_STATE;

      return 0;
   }

   if (rc == 0) {

      nb_ctx.state = NB_INITIAL_STATE;

      if (nb_ctx.len > 2)
         return nb_ctx.key;

      return 0;
   }

   nb_ctx_append(c);

   if (0x40 <= c && c <= 0x7E && c != '[') {
      nb_ctx.state = NB_INITIAL_STATE;
      return nb_ctx.key;
   }

   if (nb_ctx.len == sizeof(tfb_key_t)) {
      /* no more space in our 64-bit int (seq too long) */
      nb_ctx.state = NB_INITIAL_STATE;
   }

   return 0;
}

static tfb_key_t tfb_switch_state_read(int rc, char c)
{
   switch (nb_ctx.state) {

      case NB_INITIAL_STATE:
         return nb_handle_initial_state(rc, c);

      case NB_AFTER_ESC_READ:
         return nb_handle_after_esc_state(rc, c);

      case NB_AFTER_OPEN_BRACKET_READ:
         return nb_handle_after_open_bracket_state(rc, c);
   }

   return 0;
}

tfb_key_t tfb_read_keypress(void)
{
   tfb_key_t ret = 0;
   int rc;
   char c;

   if (!tfb_kb_raw_mode) {
      /*
       * tfb_read_keypress() is supposed to be used only after a successful
       * call to tfb_set_kb_raw_mode().
       */
      return 0;
   }

   for (u32 i = 0; i < sizeof(tfb_key_t); i++) {

      if (!nb_ctx.readbuf_len) {

         rc = read(ttyfd, nb_ctx.readbuf, sizeof(nb_ctx.readbuf));

         if (rc <= 0)
            break;

         nb_ctx.readbuf_len = rc;
      }

      c = nb_ctx.readbuf[0];
      memmove(nb_ctx.readbuf, nb_ctx.readbuf + 1, sizeof(nb_ctx.readbuf) - 1);
      nb_ctx.readbuf_len--;

      ret = tfb_switch_state_read(rc, c);

      if (ret != 0)
         break;
   }

   return ret;
}

static char tfb_fn_key_seq_char[12][8] =
{
   { '\033', '[', '[', 'A', 0, 0, 0, 0 },
   { '\033', '[', '[', 'B', 0, 0, 0, 0 },
   { '\033', '[', '[', 'C', 0, 0, 0, 0 },
   { '\033', '[', '[', 'D', 0, 0, 0, 0 },
   { '\033', '[', '[', 'E', 0, 0, 0, 0 },
   { '\033', '[', '1', '7', '~', 0, 0, 0 },
   { '\033', '[', '1', '8', '~', 0, 0, 0 },
   { '\033', '[', '1', '9', '~', 0, 0, 0 },
   { '\033', '[', '2', '0', '~', 0, 0, 0 },
   { '\033', '[', '2', '1', '~', 0, 0, 0 },
   { '\033', '[', '2', '3', '~', 0, 0, 0 },
   { '\033', '[', '2', '4', '~', 0, 0, 0 },
};

uint64_t *tfb_fn_key_sequences = (uint64_t *)tfb_fn_key_seq_char;

int tfb_get_fn_key_num(uint64_t k)
{
   for (u32 i = 0; i < 12; i++)
      if (tfb_fn_key_sequences[i] == k)
         return i + 1;

   return 0;
}
