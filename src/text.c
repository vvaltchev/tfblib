/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include "utils.h"
#include "font.h"

static void *curr_font;
static u32 curr_font_w;
static u32 curr_font_h;
static u32 curr_font_w_bytes;
static u32 curr_font_bytes_per_glyph;
static u8 *curr_font_data;

void tfb_iterate_over_fonts(tfb_font_iter_func f, void *user_arg)
{
   tfb_font_info fi;
   psf1_header *h1;
   psf2_header *h2;
   const font_file **it;

   for (it = tfb_font_file_list; *it; it++) {

      h1 = (void *)(*it)->data;
      h2 = (void *)(*it)->data;

      if (h2->magic == PSF2_MAGIC) {

         fi = (tfb_font_info) {
            .name = (*it)->filename,
            .width = h2->width,
            .height = h2->height,
            .psf_version = 2,
            .font_id = (void *)*it
         };

      } else {

         fi = (tfb_font_info) {
            .name = (*it)->filename,
            .width = 8,
            .height = h1->bytes_per_glyph,
            .psf_version = 1,
            .font_id = (void *)*it
         };

      }

      if (!f(&fi, user_arg))
         break;
   }
}

int tfb_set_current_font(void *font_id)
{
   const font_file *ff = font_id;
   psf1_header *h1 = (void *)ff->data;
   psf2_header *h2 = (void *)ff->data;

   if (h2->magic == PSF2_MAGIC) {
      curr_font = h2;
      curr_font_w = h2->width;
      curr_font_h = h2->height;
      curr_font_w_bytes = h2->bytes_per_glyph / h2->height;
      curr_font_data = curr_font + h2->header_size;
      curr_font_bytes_per_glyph = h2->bytes_per_glyph;
   } else if (h1->magic == PSF1_MAGIC) {
      curr_font = h1;
      curr_font_w = 8;
      curr_font_h = h1->bytes_per_glyph;
      curr_font_w_bytes = 1;
      curr_font_data = curr_font + sizeof(psf1_header);
      curr_font_bytes_per_glyph = h1->bytes_per_glyph;
   } else {
      return TFB_INVALID_FONT_ID;
   }

   return TFB_SUCCESS;
}

void tfb_draw_char(u32 x, u32 y, u32 color, u8 c)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   u8 *data = curr_font_data + curr_font_bytes_per_glyph * c;

   for (u32 row = 0; row < curr_font_h; row++)
      for (u32 b = 0; b < curr_font_w_bytes; b++)
         for (u32 bit = 0; bit < 8; bit++)
            if ((data[b + curr_font_w_bytes * row] & (1 << bit)))
               tfb_draw_pixel(x + (b << 3) + 8 - bit - 1, y + row, color);
}

void tfb_draw_string(u32 x, u32 y, u32 color, const char *s)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   while (*s) {
      tfb_draw_char(x, y, color, *s);
      x += curr_font_w;
      s++;
   }
}