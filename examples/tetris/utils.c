/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

/* Generic utils */
double int_pow(double b, int p)
{
   double ret = 1.0;

   for (int i = 0; i < p; i++)
      ret *= b;

   return ret;
}

/* Game-related stuff */
static const unsigned char piece_i[4][4] =
{
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0}
};

static const unsigned char piece_j[4][4] =
{
   {0, 0, 0, 0},
   {0, 0, 1, 0},
   {0, 0, 1, 0},
   {0, 1, 1, 0}
};

static const unsigned char piece_l[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 0, 0},
   {0, 1, 1, 0}
};

static const unsigned char piece_o[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 1, 0},
   {0, 1, 1, 0},
   {0, 0, 0, 0}
};

static const unsigned char piece_s[4][4] =
{
   {0, 0, 0, 0},
   {0, 0, 1, 1},
   {0, 1, 1, 0},
   {0, 0, 0, 0}
};

static const unsigned char piece_t[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 0, 0},
   {1, 1, 1, 0},
   {0, 0, 0, 0},
};

static const unsigned char piece_z[4][4] =
{
   {0, 0, 0, 0},
   {0, 1, 1, 0},
   {0, 0, 1, 1},
   {0, 0, 0, 0}
};

static const unsigned char (*pieces[])[4][4] =
{
   &piece_i,
   &piece_j,
   &piece_l,
   &piece_o,
   &piece_s,
   &piece_t,
   &piece_z,
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