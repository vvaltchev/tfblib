/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include "utils.h"
#include "font.h"

static psf2_header *current_font;

void tfb_draw_char(u32 x, u32 y, u32 color, u8 c)
{
   psf2_header *h = current_font;

   if (!h) {
      fprintf(stderr, "[tfblib] Error: no font currently selected\n");
      abort();
   }

   // ASSUMPTION: width is divisible by 8
   const u32 width_bytes = h->width >> 3;

   u8 *data = (u8 *)h + h->header_size + h->bytes_per_glyph * c;

   for (u32 row = 0; row < h->height; row++)
      for (u32 b = 0; b < width_bytes; b++)
         for (u32 bit = 0; bit < 8; bit++)
            if ((data[b + width_bytes * row] & (1 << bit)))
               tfb_draw_pixel(x + (b << 3) + 8 - bit - 1, y + row, color);
}
