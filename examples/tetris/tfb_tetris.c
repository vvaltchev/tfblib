/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

uint32_t tw = 20; /* single tile width */
uint32_t th = 20; /* single tile height */

unsigned char piece_i[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {1, 1, 1, 1, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0}
};

unsigned char piece_j[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {1, 1, 1, 0, 0},
   {0, 0, 1, 0, 0},
   {0, 0, 0, 0, 0}
};

unsigned char piece_l[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 1, 0, 0},
   {1, 1, 1, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0}
};

unsigned char piece_o[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 1, 1, 0, 0},
   {0, 1, 1, 0, 0},
   {0, 0, 0, 0, 0}
};

unsigned char piece_s[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 1, 1, 0},
   {0, 1, 1, 0, 0},
   {0, 0, 0, 0, 0}
};

unsigned char piece_t[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 1, 0, 0},
   {0, 1, 1, 1, 0},
   {0, 0, 0, 0, 0}
};

unsigned char piece_z[5][5] =
{
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 1, 1, 0, 0},
   {0, 0, 1, 1, 0},
   {0, 0, 0, 0, 0}
};

unsigned char (*pieces[])[5][5] =
{
   &piece_i,
   &piece_j,
   &piece_l,
   &piece_o,
   &piece_s,
   &piece_t,
   &piece_z,
};

void draw_piece(u32 x, u32 y, int p, u32 color)
{
   for (u32 i = 0; i < 5; i++) {
      for (u32 j = 0; j < 5; j++) {

         if ((*pieces[p])[j][i]) {
            tfb_fill_rect(x + i * tw + 1,
                          y + j * th + 1,
                          tw - 2,
                          th - 2,
                          color);
         }

      }
   }
}

void game_loop(void)
{
   uint32_t w = MAX(tfb_screen_width()/2, 640);
   uint32_t h = MAX(tfb_screen_height()/2, 480);
   uint64_t k = 0;

   if (tfb_set_center_window_size(w, h) != TFB_SUCCESS) {
      fprintf(stderr, "Unable to set window to %ux%u\n", w, h);
      return;
   }

   w = tfb_win_width();
   h = tfb_win_height();

   tfb_clear_win(black);

   draw_piece(20 + 0 * tw * 4, 20, 0, red);
   draw_piece(20 + 1 * tw * 4 + tw, 20, 1, yellow);
   draw_piece(20 + 2 * tw * 4 + tw, 20, 2, green);
   draw_piece(20 + 3 * tw * 4 + tw, 20, 3, magenta);
   draw_piece(20 + 4 * tw * 4 + tw, 20, 4, blue);
   draw_piece(20 + 5 * tw * 4 + tw, 20, 5, cyan);
   draw_piece(20 + 6 * tw * 4 + tw, 20, 6, white);

   tfb_draw_rect(0, 0, w, h, white);
   k = tfb_read_keypress();
}


int main(int argc, char **argv)
{
   int rc;

   set_fb_font();
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
