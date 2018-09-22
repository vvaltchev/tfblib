/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

uint32_t tw = 40; /* single tile width */
uint32_t th = 40; /* single tile height */

unsigned char tiles[12][12];

int curr_piece;
int cp_row;
int cp_col;
int cp_rot;

unsigned char piece_i[4][4] =
{
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0}
};

unsigned char piece_j[4][4] =
{
   {0, 0, 0, 0},
   {0, 0, 1, 0},
   {0, 0, 1, 0},
   {0, 1, 1, 0}
};

unsigned char piece_l[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 1, 0}
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

bool is_tile_set(u32 p, int r, int c, u32 rotation)
{
   switch (rotation % 4) {

      case 0:
         return (*pieces[p])[4-r-1][c];

      case 1:
         return (*pieces[p])[c][r];

      case 2:
         return (*pieces[p])[r][4-c-1];

      case 3:
         return (*pieces[p])[4-c-1][4-r-1];
   }

   __builtin_unreachable();
}

void draw_tile_xy(int x, int y, u32 color)
{
   if (x >= 0 && y >= 0)
      tfb_fill_rect(x + 1, y + 1, tw - 2, th - 2, color);
}

void draw_tile(int row, int col, u32 color)
{
   draw_tile_xy(col * tw, (12 - row - 1) * th, color);
}

void draw_piece(int piece, int row, int col, u32 color, u32 rotation)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         if (is_tile_set(piece, r, c, rotation))
            draw_tile(row + r, col + c, color);
}


void redraw_scene(void)
{
   u32 w = tfb_win_width();
   u32 h = tfb_win_height();

   draw_piece(curr_piece,
              cp_row,
              cp_col,
              *piece_colors[curr_piece],
              cp_rot);

   for (u32 row = 0; row < 12; row++) {
      for (u32 col = 0; col < 12; col++) {

         int p = tiles[row][col] - 1;

         if (p < 0)
            continue;

         draw_tile(row, col, *piece_colors[p]);
      }
   }

   // window border
   tfb_draw_rect(0, 0, w, h, white);

   // tetris area / info area separation line
   tfb_draw_vline(480, 0, h, white);
}

bool will_cp_collide(int new_row, int new_col, int rot)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         if (is_tile_set(curr_piece, r, c, rot)) {

            if (new_row + r < 0)
               return true;

            if (new_col + c < 0 || new_col + c > 11)
               return true;

            if (tiles[new_row + r][new_col + c] > 0)
               return true;
         }

   return false;
}

void consolidate_curr_piece(void)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         if (is_tile_set(curr_piece, r, c, cp_rot))
            tiles[cp_row + r][cp_col + c] = curr_piece + 1;
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

   curr_piece = rand() % 7;
   cp_row = 12;
   cp_col = 5;
   cp_rot = 0;

   while (true) {

      if (k == TFB_KEY_UP) {

         if (!will_cp_collide(cp_row, cp_col, cp_rot + 1))
            cp_rot++;

      } else if (k == TFB_KEY_DOWN) {

         if (!will_cp_collide(cp_row, cp_col, cp_rot - 1))
            cp_rot--;

      } else if (k == TFB_KEY_LEFT) {

         if (!will_cp_collide(cp_row, cp_col - 1, cp_rot))
            cp_col--;

      } else if (k == TFB_KEY_RIGHT) {

         if (!will_cp_collide(cp_row, cp_col + 1, cp_rot))
            cp_col++;
      }

      if (k == ' ') {

         if (will_cp_collide(cp_row - 1, cp_col, cp_rot)) {

            consolidate_curr_piece();

            curr_piece = rand() % 7;
            cp_row = 12;
            cp_col = 5;
            cp_rot = 0;

         } else {
            cp_row--;
         }
      }


      tfb_clear_win(black);
      redraw_scene();

      k = tfb_read_keypress();

      if (k == 'q')
         break;
   }
}


int main(int argc, char **argv)
{
   int rc;

   srand(time(NULL));

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
