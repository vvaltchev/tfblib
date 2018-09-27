/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

#define MAX_ROWS 40
#define MAX_COLS 40

u32 tw = 2 * 20; /* single tile width */
u32 th = 2 * 20; /* single tile height */

u32 rows;
u32 cols;

int tetris_row = -1;
u32 off_y = 0; /* temporary offset used for the tetris effect */

unsigned char tiles[MAX_ROWS][MAX_COLS];

int curr_piece;
int cp_row;
int cp_col;
int cp_rot;
double fp_cp_row;
double row_dec_speed;

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
   &cyan,
   &blue,
   &orange,
   &yellow,
   &green,
   &purple,
   &red
};

bool is_tile_set(u32 p, int r, int c, u32 rotation)
{
   switch (rotation % 4) {

      case 0:
         return (*pieces[p])[r][c];

      case 1:
         return (*pieces[p])[4-c-1][r];

      case 2:
         return (*pieces[p])[4-r-1][4-c-1];

      case 3:
         return (*pieces[p])[c][4-r-1];
   }

   __builtin_unreachable();
}

void draw_tile_xy(int x, int y, u32 color)
{
   if (x >= 0 && y >= 0)
      tfb_fill_rect(x + 1, y + 1, tw - 2, th - 2, color);
}

void draw_tile(int r, int c, u32 color)
{
   int off = (tetris_row >= 0 && r > tetris_row) ? off_y : 0;
   draw_tile_xy(c * tw, (rows - r - 1) * th + off, color);
}

void draw_piece(int piece, int row, int col, u32 color, u32 rotation)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         if (is_tile_set(piece, r, c, rotation))
            draw_tile(row + 4 - r - 1, col + c, color);
}


void redraw_scene(void)
{
   u32 w = tfb_win_width();
   u32 h = tfb_win_height();

   if (tetris_row == -1) {
      draw_piece(curr_piece,
                 cp_row,
                 cp_col,
                 *piece_colors[curr_piece],
                 cp_rot);
   }

   for (u32 row = 0; row < rows; row++)
      for (u32 col = 0; col < cols; col++)
         if (tiles[row][col] > 0)
            draw_tile(row, col, *piece_colors[tiles[row][col] - 1]);

   tfb_draw_center_string(tw * cols + (w - tw * cols) / 2,
                          20, yellow, black,
                          "A Tiny Framebuffer Tetris");

   // window border
   tfb_draw_rect(0, 0, w, h, white);

   // tetris area / info area separation line
   tfb_draw_vline(tw * cols, 0, h, white);

   tfb_flush_window();
}

bool will_cp_collide(int new_row, int new_col, int rot)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         if (is_tile_set(curr_piece, r, c, rot)) {

            if (new_row + 4 - r - 1 < 0)
               return true;

            if (new_col + c < 0 || new_col + c >= cols)
               return true;

            if (tiles[new_row + 4 - r - 1][new_col + c] > 0)
               return true;
         }

   return false;
}

bool is_row_full(int row)
{
   for (int c = 0; c < cols; c++)
      if (!tiles[row][c])
         return false;

   return true;
}

void do_tetris(int full_row)
{
   for (int c = 0; c < cols; c++)
      tiles[full_row][c] = 0;

   tetris_row = full_row;
   for (off_y = 0; off_y < th; off_y += 2) {
      tfb_clear_win(black);
      redraw_scene();
      usleep(10 * 1000);
   }

   off_y = 0;
   tetris_row = -1;

   for (int r = full_row + 1; r < rows; r++)
      for (int c = 0; c < cols; c++)
         tiles[r - 1][c] = tiles[r][c];
}

void consolidate_curr_piece(void)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         if (is_tile_set(curr_piece, r, c, cp_rot))
            tiles[cp_row + 4 - r - 1][cp_col + c] = curr_piece + 1;

   for (int r = 0; r < rows; r++) {
      if (is_row_full(r)) {
         do_tetris(r);
         r--; /* stay on the same row! */
      }
   }
}

int get_random_piece(void)
{
   return rand() % 7;
}

void setup_new_piece(void)
{
   curr_piece = get_random_piece();
   cp_row = rows;
   cp_col = cols/2;
   cp_rot = 0;

   fp_cp_row = cp_row;
}

void move_curr_piece_down(double dec)
{
   int p_row = fp_cp_row - dec;

   if (will_cp_collide(p_row, cp_col, cp_rot)) {

      consolidate_curr_piece();
      setup_new_piece();

   } else {
      fp_cp_row -= dec;
   }
}

int game_loop(void)
{
   uint32_t w = 2 * 480;
   uint32_t h = 2 * 480;
   uint64_t k = 0;
   row_dec_speed = 0.05;

   if (tfb_set_center_window_size(w, h) != TFB_SUCCESS) {

      w /= 2;
      h /= 2;
      tw /= 2;
      th /= 2;

      if (tfb_set_center_window_size(w, h) != TFB_SUCCESS)
         return 1;

      tfb_set_font_by_size(8, TFB_FONT_ANY_HEIGHT);
   }

   w = tfb_win_width();
   h = tfb_win_height();

   rows = h / th;
   cols = (w * 5 / 10) / tw;

   setup_new_piece();

   while (true) {

      if (k == 'q') {

         break;

      } else if (k == TFB_KEY_UP) {

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

      } else if (k == ' ') {

         move_curr_piece_down(1.0);

      } else if (k == 0) {

         move_curr_piece_down(row_dec_speed);
      }

      if (k || ((int)fp_cp_row != cp_row)) {
         cp_row = fp_cp_row;
         tfb_clear_win(black);
         redraw_scene();
      }

      k = tfb_read_keypress();

      if (!k)
         usleep(25*1000);
   }

   return 0;
}


int main(int argc, char **argv)
{
   int rc;
   srand(time(NULL));

   if (tfb_set_font_by_size(16, TFB_FONT_ANY_HEIGHT) != TFB_SUCCESS) {
      fprintf(stderr, "Unable to select a font with width = 16\n");
      return 1;
   }

   rc = tfb_acquire_fb(TFB_FL_USE_SHADOW_BUFFER, NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   init_colors();

   rc = tfb_set_kb_raw_mode(TFB_FL_KB_NONBLOCK);

   if (rc != TFB_SUCCESS)
      fprintf(stderr, "tfb_set_kb_raw_mode() failed with err: %d", rc);

   rc = game_loop();

   tfb_restore_kb_mode();
   tfb_release_fb();

   if (rc) {
      fprintf(stderr, "game_loop() failed with error code: %d\n", rc);
   }

   return 0;
}
