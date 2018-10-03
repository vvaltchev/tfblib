/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>
#include <tfblib/tfb_colors.h>

#define MIN(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x <= _y ? _x : _y; })

#define MAX(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x > _y ? _x : _y; })

void loop(void)
{
   char buf[64];
   uint32_t w = MAX(tfb_screen_width()/2, 640);
   uint32_t h = MAX(tfb_screen_height()/2, 480);
   uint32_t step = 10;
   int n;

   uint64_t k = 0;

   if (tfb_set_center_window_size(w, h) != TFB_SUCCESS) {
      fprintf(stderr, "Unable to set window to %ux%u\n", w, h);
      return;
   }

   w = tfb_win_width();
   h = tfb_win_height();
   uint32_t x = w/8, y = h/8 - h/16;

   tfb_clear_win(tfb_black);
   tfb_draw_rect(0, 0, w, h, tfb_white);
   tfb_fill_rect(x, y, w/4, h/4, tfb_red);
   tfb_flush_window();

   do {

      k = tfb_read_keypress();
      tfb_clear_win(tfb_black);

      if (isprint(k & 0xff)) {

         sprintf(buf, "Pressed key: %c", (char)k);
         tfb_draw_string(5, 5, tfb_green, tfb_black, buf);

      } else if ((n = tfb_get_fn_key_num(k))) {

         sprintf(buf, "Pressed key: F%d", n);
         tfb_draw_string(5, 5, tfb_green, tfb_black, buf);

      } else if (k == TFB_KEY_RIGHT) {
         x += step;
      } else if (k == TFB_KEY_DOWN) {
         y += step;
      } else if (k == TFB_KEY_UP) {
         y -= y >= step ? step : y;
      } else if (k == TFB_KEY_LEFT) {
         x -= x >= step ? step : x;
      }

      // redraw the rect
      tfb_fill_rect(x, y, w/4, h/4, tfb_red);

      // win border
      tfb_draw_rect(0, 0, w, h, tfb_white);

      // do the actual copy to the framebuffer
      tfb_flush_window();

   } while (k != TFB_KEY_ENTER);
}

int main(int argc, char **argv)
{
   int rc;

   tfb_set_font_by_size(16, 32);
   rc = tfb_acquire_fb(TFB_FL_USE_DOUBLE_BUFFER, NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   rc = tfb_set_kb_raw_mode(0);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_set_kb_raw_mode() failed with err: %d\n", rc);
      goto end;
   }

   loop();

end:
   tfb_restore_kb_mode();
   tfb_release_fb();

   if (rc)
      fprintf(stderr, "tfblib error: %d", rc);

   return 0;
}
