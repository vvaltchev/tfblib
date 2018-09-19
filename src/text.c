/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include "utils.h"
#include "font.h"

static psf2_header *current_font;

void tfb_iterate_over_fonts(tfb_font_iter_func f, void *user_arg)
{
   tfb_font_info fi;
   psf2_header *h;
   const font_file **it;

   for (it = tfb_font_file_list; *it; it++) {

      h = (void *)(*it)->data;

      if (h->magic != PSF2_FONT_MAGIC) {
         fprintf(stderr,
                 "[tfblib] Skipping non-psf2 font file '%s'\n",
                 (*it)->filename);
         continue;
      }

      fi = (tfb_font_info) {
         .name = (*it)->filename,
         .width = h->width,
         .height = h->height,
         .font_id = (void *)*it
      };

      if (!f(&fi, user_arg))
         break;
   }
}

int tfb_set_current_font(void *font_id)
{
   psf2_header *h;
   const font_file **it = tfb_font_file_list;

   for (it = tfb_font_file_list; *it; it++) {

      if (*it == font_id) {

         h = (void *)(*it)->data;

         if (h->magic == PSF2_FONT_MAGIC) {
            current_font = h;
            return TFB_SUCCESS;
         }
      }
   }

   return TFB_INVALID_FONT_ID;
}

void tfb_draw_char(u32 x, u32 y, u32 color, u8 c)
{
   psf2_header *h = current_font;

   if (!h) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   const u32 width_bytes = h->bytes_per_glyph / h->height;

   u8 *data = (u8 *)h + h->header_size + h->bytes_per_glyph * c;

   for (u32 row = 0; row < h->height; row++)
      for (u32 b = 0; b < width_bytes; b++)
         for (u32 bit = 0; bit < 8; bit++)
            if ((data[b + width_bytes * row] & (1 << bit)))
               tfb_draw_pixel(x + (b << 3) + 8 - bit - 1, y + row, color);
}

void tfb_draw_string(u32 x, u32 y, u32 color, const char *s)
{
   psf2_header *h = current_font;

   if (!h) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   while (*s) {
      tfb_draw_char(x, y, color, *s);
      x += h->width;
      s++;
   }
}