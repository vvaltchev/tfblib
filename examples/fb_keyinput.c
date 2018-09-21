/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
   uint32_t w = tfb_screen_width();
   uint32_t h = tfb_screen_height();
   uint64_t k = 0;

   if (tfb_set_center_window_size(w/2, h/2) != TFB_SUCCESS) {
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

      // win border
      tfb_draw_rect(0, 0, w, h, white);

      // erase the rect
      tfb_fill_rect(x, y, w/4, h/4, black);

      tfb_draw_string(5, 5, green, black, "                                 ");

      if (isprint(k & 0xff)) {

         char buf[64];
         sprintf(buf, "Pressed key: %c", (char)k);
         tfb_draw_string(5, 5, green, black, buf);

      } else if (k == TFB_KEY_F1) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F1");
      } else if (k == TFB_KEY_F2) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F2");
      } else if (k == TFB_KEY_F3) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F3");
      } else if (k == TFB_KEY_F4) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F4");
      } else if (k == TFB_KEY_F5) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F5");
      } else if (k == TFB_KEY_F6) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F6");
      } else if (k == TFB_KEY_F7) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F7");
      } else if (k == TFB_KEY_F8) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F8");
      } else if (k == TFB_KEY_F9) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F9");
      } else if (k == TFB_KEY_F10) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F10");
      } else if (k == TFB_KEY_F11) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F11");
      } else if (k == TFB_KEY_F12) {
         tfb_draw_string(5, 5, green, black, "Pressed key: F12");
      }

      if (k == TFB_KEY_RIGHT) {
         x += 10;
      } else if (k == TFB_KEY_DOWN) {
         y += 10;
      } else if (k == TFB_KEY_UP) {
         if (y > 10) y -= 10;
      } else if (k == TFB_KEY_LEFT) {
         if (x > 10) x -= 10;
      }

      // redraw the rect
      tfb_fill_rect(x, y, w/4, h/4, red);

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
   rc = tfb_acquire_fb(TFB_FL_NO_TTY_KD_GRAPHICS, NULL, NULL);

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
