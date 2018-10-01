/* SPDX-License-Identifier: BSD-2-Clause */

#include <tfblib/tfblib.h>
#include "utils.h"

#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

extern int __tfb_ttyfd;

static struct termios orig_termios;
static bool tfb_kb_raw_mode;
static bool tfb_kb_nonblock;
static int tfb_saved_kdmode;
static u32 tfb_kb_saved_fcntl_flags;

int tfb_set_kb_raw_mode(u32 flags)
{
   struct termios t;
   int rc;

   if (tfb_kb_raw_mode)
      return TFB_ERR_KB_WRONG_MODE;

   if (ioctl(__tfb_ttyfd, KDGKBMODE, &tfb_saved_kdmode) != 0)
      return TFB_ERR_KB_MODE_GET_FAILED;

   if (tfb_saved_kdmode != K_XLATE) {
      if (ioctl(__tfb_ttyfd, KDSKBMODE, K_XLATE) != 0)
         return TFB_ERR_KB_MODE_SET_FAILED;
   }

   if (tcgetattr(__tfb_ttyfd, &orig_termios) != 0)
      return TFB_ERR_KB_MODE_GET_FAILED;

   t = orig_termios;
   t.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);
   t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

   if (tcsetattr(__tfb_ttyfd, TCSAFLUSH, &t) != 0)
      return TFB_ERR_KB_MODE_SET_FAILED;

   tfb_kb_raw_mode = true;

   if (flags & TFB_FL_KB_NONBLOCK) {

      rc = fcntl(__tfb_ttyfd, F_GETFL, 0);

      if (rc < 0) {
         tfb_restore_kb_mode();
         return TFB_ERR_KB_MODE_GET_FAILED;
      }

      tfb_kb_saved_fcntl_flags  = rc;

      rc = fcntl(__tfb_ttyfd, F_SETFL, tfb_kb_saved_fcntl_flags | O_NONBLOCK);

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

      fcntl(__tfb_ttyfd, F_SETFL, tfb_kb_saved_fcntl_flags);
      tfb_kb_nonblock = false;
   }

  if (!tfb_kb_raw_mode)
      return TFB_ERR_KB_WRONG_MODE;

   /* Restore the original kb mode. Note: ignoring any failure */
  ioctl(__tfb_ttyfd, KDSKBMODE, &tfb_saved_kdmode);

   if (tcsetattr(__tfb_ttyfd, TCSAFLUSH, &orig_termios) != 0)
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

         rc = read(__tfb_ttyfd, nb_ctx.readbuf, sizeof(nb_ctx.readbuf));

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

tfb_key_t *tfb_fn_key_sequences = (tfb_key_t *)tfb_fn_key_seq_char;

int tfb_get_fn_key_num(tfb_key_t k)
{
   for (u32 i = 0; i < 12; i++)
      if (tfb_fn_key_sequences[i] == k)
         return i + 1;

   return 0;
}
