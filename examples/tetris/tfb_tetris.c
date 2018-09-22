/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

uint32_t tw = 20; /* single tile width */
uint32_t th = 20; /* single tile height */

unsigned char piece_i[4][4] =
{
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0}
};

unsigned char piece_j[4][4] =
{
   {0, 0, 1, 0},
   {0, 0, 1, 0},
   {0, 1, 1, 0},
   {0, 0, 0, 0}
};

unsigned char piece_l[4][4] =
{
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 1, 0},
   {0, 0, 0, 0}
};

unsigned char piece_o[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 1, 0},
   {0, 1, 1, 0},
   {0, 0, 0, 0}
};

unsigned char piece_s[4][4] =
{
   {0, 0, 0, 0},
   {0, 0, 1, 1},
   {0, 1, 1, 0},
   {0, 0, 0, 0}
};

unsigned char piece_t[4][4] =
{
   {0, 0, 0, 0},
   {0, 0, 1, 0},
   {0, 1, 1, 1},
   {0, 0, 0, 0},
};

unsigned char piece_z[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 1, 0},
   {0, 0, 1, 1},
   {0, 0, 0, 0}
};

unsigned char (*pieces[])[4][4] =
{
   &piece_i,
   &piece_j,
   &piece_l,
   &piece_o,
   &piece_s,
   &piece_t,
   &piece_z,
};

u32 *piece_colors[] =
{
   &red,
   &yellow,
   &green,
   &magenta,
   &blue,
   &cyan,
   &white
};

bool is_tile_set(u32 p, u32 i, u32 j, u32 rotation)
{
   switch (rotation % 4) {

      case 0:
         return (*pieces[p])[j][i];

      case 1:
         return (*pieces[p])[i][4-j-1];

      case 2:
         return (*pieces[p])[4-j-1][4-i-1];

      case 3:
         return (*pieces[p])[4-i-1][j];

   }

   __builtin_unreachable();
}

void draw_piece(u32 piece, u32 x, u32 y, u32 color, u32 rotation)
{
   for (u32 i = 0; i < 4; i++) {
      for (u32 j = 0; j < 4; j++) {

         if (is_tile_set(piece, i, j, rotation))
            tfb_fill_rect(x + i * tw + 1,
                          y + j * th + 1,
                          tw - 2,
                          th - 2,
                          color);
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

   for (u32 rot = 0; rot < 4; rot++) {

      for (u32 p = 0; p < 7; p++)
         draw_piece(p,
                    10 + p * tw * 4 + (p>0?tw:0),
                    10 + rot * th * 5 + th,
                    *piece_colors[p],
                    rot);

   }

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
