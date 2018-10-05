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
   char buf1[64], buf2[64];
   uint32_t w = MAX(tfb_screen_width()/2, 640);
   uint32_t h = MAX(tfb_screen_height()/2, 480);
   uint32_t step = 10;
   int n, rc;
   tfb_key_t k = (tfb_key_t)-1;

   /*
    * Set the current window to be smaller than the screen. This will allow
    * to see and test how graphic elements (such as rectangles and text) get
    * cut when they are moved outside of the current window.
    */
   rc = tfb_set_center_window_size(w, h);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "Unable to set window to %ux%u\n", w, h);
      return;
   }

   w = tfb_win_width();
   h = tfb_win_height();

   int rw = w / 4;
   int rh = h / 4;
   int x = w / 2 - rw / 2;
   int y = h / 2 - rh / 2;

   do {

      k = (k != (tfb_key_t)-1) ? tfb_read_keypress() : 0;

      tfb_clear_win(tfb_black);

      strcpy(buf1, "Use ARROW keys to move or ENTER to exit");

      if (isprint(k & 0xff)) {
         sprintf(buf1, "Pressed key: %c", (char)k);
      } else if ((n = tfb_get_fn_key_num(k))) {
         sprintf(buf1, "Pressed key: F%d", n);
      } else if (k == TFB_KEY_RIGHT) {
         x += step;
      } else if (k == TFB_KEY_DOWN) {
         y += step;
      } else if (k == TFB_KEY_UP) {
         y -= step;
      } else if (k == TFB_KEY_LEFT) {
         x -= step;
      }

      // draw the text at the top
      tfb_draw_string(5, 5, tfb_green, tfb_black, buf1);

      // draw the rectangle
      tfb_draw_rect(x, y, rw, rh, tfb_red);
      tfb_fill_rect(x + 5, y + 5, rw - 10, rh - 10, tfb_red);

      sprintf(buf2, "%d, %d", x, y);

      // draw the text at the center of rectangle
      tfb_draw_xcenter_string(x + rw / 2,
                              y + rh / 2 - tfb_get_curr_font_height()/2,
                              tfb_white, tfb_red, buf2);


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
