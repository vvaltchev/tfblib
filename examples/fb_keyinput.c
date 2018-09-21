/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

uint32_t red, green, blue, white, black, yellow, gray;

void init_colors(void)
{
   white = tfb_make_color(255, 255, 255);
   black = tfb_make_color(0, 0, 0);
   red = tfb_make_color(255, 0, 0);
   green = tfb_make_color(0, 255, 0);
   blue = tfb_make_color(0, 0, 255);
   yellow = tfb_make_color(255, 255, 0);
   gray = tfb_make_color(50, 50, 50);
}

void loop(void)
{
   uint64_t k;

   tfb_set_center_window_size(640, 480);

   uint32_t w = tfb_win_width();
   uint32_t h = tfb_win_height();
   uint32_t x = w/8, y = h/8 - h/16;

   do {

      tfb_clear_win(black);
      tfb_draw_rect(0, 0, w, h, white);
      tfb_fill_rect(x, y, w/4, h/4, red);

      k = tfb_read_keypress();

      if (k == TFB_KEY_RIGHT) {
         x += 10;

      } else if (k == TFB_KEY_DOWN) {

         y += 10;

      } else if (k == TFB_KEY_UP) {

         if (y > 10) y -= 10;

      } else if (k == TFB_KEY_LEFT) {

         if (x > 10) x -= 10;
      }

   } while (k != TFB_KEY_ENTER);
}

int main(int argc, char **argv)
{
   int rc;
   rc = tfb_acquire_fb(NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   init_colors();

   rc = tfb_set_kb_raw_mode();

   if (rc != TFB_SUCCESS)
      fprintf(stderr, "tfb_set_kb_raw_mode() failed with err: %d", rc);

   loop();

   tfb_restore_kb_mode();
   tfb_release_fb();
   return 0;
}
