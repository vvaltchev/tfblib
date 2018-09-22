/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

void game_loop(void)
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


   k = tfb_read_keypress();
}

bool font_iter_cb_select_font32x16(tfb_font_info *fi, void *user_arg)
{
   if (fi->width == 16 && fi->height == 32) {

      int rc = tfb_set_current_font(fi->font_id);

      if (rc != TFB_SUCCESS) {
         fprintf(stderr, "tfb_set_current_font() failed with error: %d\n", rc);
         abort();
      }

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

   game_loop();

   tfb_restore_kb_mode();
   tfb_release_fb();
   return 0;
}
