/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#define MIN(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x <= _y ? _x : _y; })

#define MAX(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x > _y ? _x : _y; })

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

   tfb_clear_win(black);
   tfb_draw_rect(0, 0, w, h, white);
   tfb_fill_rect(x, y, w/4, h/4, red);

   do {

      k = tfb_read_keypress();
      tfb_clear_win(black);

      if (isprint(k & 0xff)) {

         sprintf(buf, "Pressed key: %c", (char)k);
         tfb_draw_string(5, 5, green, black, buf);

      } else if ((n = tfb_get_fn_key_num(k))) {

         sprintf(buf, "Pressed key: F%d", n);
         tfb_draw_string(5, 5, green, black, buf);

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
      tfb_fill_rect(x, y, w/4, h/4, red);

      // win border
      tfb_draw_rect(0, 0, w, h, white);

   } while (k != TFB_KEY_ENTER);
}

bool font_iter_cb_select_font32x16(tfb_font_info *fi, void *user_arg)
{
   if (fi->width == 16 && fi->height == 32) {

      int rc = tfb_set_current_font(fi->font_id);

      if (rc != TFB_SUCCESS) {
         fprintf(stderr, "tfb_set_current_font() failed with error: %d\n", rc);
         abort();
      }

      printf("Selected font '%s'\n", fi->name);
      return false; /* stop iterating over fonts */
   }

   return true;
}

int main(int argc, char **argv)
{
   int rc;

   tfb_iterate_over_fonts(font_iter_cb_select_font32x16, NULL);
   rc = tfb_acquire_fb(0, NULL, NULL);

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
