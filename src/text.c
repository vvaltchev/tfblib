/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tfblib/tfblib.h>
#include "utils.h"
#include "font.h"

static void *curr_font;
static u32 curr_font_w;
static u32 curr_font_h;
static u32 curr_font_w_bytes;
static u32 curr_font_bytes_per_glyph;
static u8 *curr_font_data;

/* Internal function */
void tfb_set_default_font(void *font_id)
{
   if (!curr_font)
      tfb_set_current_font(font_id);
}

int tfb_dyn_load_font(const char *file, void **font_id /* out */)
{
   struct stat statbuf;
   size_t tot_read = 0;
   font_file *ff;
   size_t ret;
   FILE *fh;
   int rc;

   *font_id = NULL;
   rc = stat(file, &statbuf);

   if (rc != 0)
      return TFB_READ_FONT_FILE_FAILED;

   ff = malloc(sizeof(font_file) + statbuf.st_size);

   if (!ff)
      return TFB_OUT_OF_MEMORY;

   fh = fopen(file, "rb");

   if (!fh)
      return TFB_READ_FONT_FILE_FAILED;

   ff->filename = malloc(strlen(file) + 1);

   if (!ff->filename) {
      fclose(fh);
      free(ff);
      return TFB_OUT_OF_MEMORY;
   }

   strcpy((char *)ff->filename, file);
   ff->data_size = statbuf.st_size;

   do {

      ret = fread(ff->data + tot_read, 1, statbuf.st_size, fh);
      tot_read += ret;

   } while (ret);

   fclose(fh);

   *font_id = ff;
   return TFB_SUCCESS;
}

int tfb_dyn_unload_font(void *font_id)
{
   font_file *ff = font_id;
   const font_file **it;

   for (it = tfb_font_file_list; *it; it++)
      if (*it == font_id)
         return TFB_NOT_A_DYN_LOADED_FONT;

   free((void *)ff->filename);
   free(ff);
   return TFB_SUCCESS;
}

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

void tfb_draw_char(u32 x, u32 y, u32 fg_color, u32 bg_color, u8 c)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   u8 *d = curr_font_data + curr_font_bytes_per_glyph * c;

   /*
    * NOTE: the following algorithm is certainly not the fastest way to draw
    * a character on-screen, but its performance is pretty good, in particular
    * for text that does not have to change continuosly (like a console).
    * Actually, this algorithm is used by Tilck[1]'s framebuffer console in a
    * fail-safe case for drawing characters on-screen: on low resolutions,
    * its performance is pretty acceptable on modern machines, even when used
    * by a console to full-redraw a screen with text. Therefore, for the
    * purposes of this library the following implementation is absolutely
    * good-enough.
    *
    * -------------------------------------------------
    * [1] Tilck [A Tiny Linux-Compatible Kernel]
    *     https://github.com/vvaltchev/tilck
    */

   for (u32 row = 0; row < curr_font_h; row++, d += curr_font_w_bytes)
      for (u32 b = 0; b < curr_font_w_bytes; b++)
         for (u32 bit = 0; bit < 8; bit++)
            tfb_draw_pixel(x + (b << 3) + 8 - bit - 1,
                           y + row,
                           (d[b] & (1 << bit)) ? fg_color : bg_color);
}

void tfb_draw_string(u32 x, u32 y, u32 fg_color, u32 bg_color, const char *s)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   while (*s) {
      tfb_draw_char(x, y, fg_color, bg_color, *s);
      x += curr_font_w;
      s++;
   }
}
