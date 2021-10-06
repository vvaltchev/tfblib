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
void tfb_set_default_font(tfb_font_t font_id)
{
   if (!curr_font)
      tfb_set_current_font(font_id);
}

int tfb_dyn_load_font(const char *file, tfb_font_t *font_id)
{
   struct stat statbuf;
   size_t tot_read = 0;
   struct font_file *ff;
   size_t ret;
   FILE *fh;
   int rc;

   *font_id = NULL;
   rc = stat(file, &statbuf);

   if (rc != 0)
      return TFB_ERR_READ_FONT_FILE_FAILED;

   ff = malloc(sizeof(struct font_file) + statbuf.st_size);

   if (!ff)
      return TFB_ERR_OUT_OF_MEMORY;

   fh = fopen(file, "rb");

   if (!fh) {
      free(ff);
      return TFB_ERR_READ_FONT_FILE_FAILED;
   }

   ff->filename = malloc(strlen(file) + 1);

   if (!ff->filename) {
      fclose(fh);
      free(ff);
      return TFB_ERR_OUT_OF_MEMORY;
   }

   strcpy((char *)ff->filename, file);
   ff->data_size = statbuf.st_size;

   do {

      ret = fread(ff->data + tot_read, 1, statbuf.st_size, fh);
      tot_read += ret;

   } while (ret);

   fclose(fh);

   *font_id = (tfb_font_t)ff;
   return TFB_SUCCESS;
}

int tfb_dyn_unload_font(tfb_font_t font_id)
{
   struct font_file *ff = font_id;
   const struct font_file **it;

   for (it = tfb_font_file_list; *it; it++)
      if (*it == font_id)
         return TFB_ERR_NOT_A_DYN_LOADED_FONT;

   free((void *)ff->filename);
   free(ff);
   return TFB_SUCCESS;
}

void tfb_iterate_over_fonts(tfb_font_iter_func f, void *user_arg)
{
   struct tfb_font_info fi;
   struct psf1_header *h1;
   struct psf2_header *h2;
   const struct font_file **it;

   for (it = tfb_font_file_list; *it; it++) {

      h1 = (void *)(*it)->data;
      h2 = (void *)(*it)->data;

      if (h2->magic == PSF2_MAGIC) {

         fi = (struct tfb_font_info) {
            .name = (*it)->filename,
            .width = h2->width,
            .height = h2->height,
            .psf_version = 2,
            .font_id = (tfb_font_t)*it
         };

      } else {

         fi = (struct tfb_font_info) {
            .name = (*it)->filename,
            .width = 8,
            .height = h1->bytes_per_glyph,
            .psf_version = 1,
            .font_id = (tfb_font_t)*it
         };

      }

      if (!f(&fi, user_arg))
         break;
   }
}

struct desired_font_size {

   int w;
   int h;
   bool found;
};

static bool tfb_sel_font_cb(struct tfb_font_info *fi, void *arg)
{
   struct desired_font_size *dfs = arg;
   bool good = true;

   if (dfs->w > 0)
      good = good && (fi->width == (u32)dfs->w);

   if (dfs->h > 0)
      good = good && (fi->height == (u32)dfs->h);

   if (good) {

      if (tfb_set_current_font(fi->font_id) == TFB_SUCCESS)
         dfs->found = true;

      return false; /* stop iteration */
   }

   return true; /* continue iteration */
}

int tfb_set_font_by_size(int w, int h)
{
   struct desired_font_size dfs = { w, h, false };
   tfb_iterate_over_fonts(tfb_sel_font_cb, &dfs);

   if (!dfs.found)
      return TFB_ERR_FONT_NOT_FOUND;

   return TFB_SUCCESS;
}

int tfb_set_current_font(tfb_font_t font_id)
{
   const struct font_file *ff = font_id;
   struct psf1_header *h1 = (void *)ff->data;
   struct psf2_header *h2 = (void *)ff->data;

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
      curr_font_data = curr_font + sizeof(struct psf1_header);
      curr_font_bytes_per_glyph = h1->bytes_per_glyph;
   } else {
      return TFB_ERR_INVALID_FONT_ID;
   }

   return TFB_SUCCESS;
}

#define draw_char_partial(b)                                                \
   do {                                                                     \
      tfb_draw_pixel(x + (b << 3) + 7, row, arr[!(data[b] & (1 << 0))]);    \
      tfb_draw_pixel(x + (b << 3) + 6, row, arr[!(data[b] & (1 << 1))]);    \
      tfb_draw_pixel(x + (b << 3) + 5, row, arr[!(data[b] & (1 << 2))]);    \
      tfb_draw_pixel(x + (b << 3) + 4, row, arr[!(data[b] & (1 << 3))]);    \
      tfb_draw_pixel(x + (b << 3) + 3, row, arr[!(data[b] & (1 << 4))]);    \
      tfb_draw_pixel(x + (b << 3) + 2, row, arr[!(data[b] & (1 << 5))]);    \
      tfb_draw_pixel(x + (b << 3) + 1, row, arr[!(data[b] & (1 << 6))]);    \
      tfb_draw_pixel(x + (b << 3) + 0, row, arr[!(data[b] & (1 << 7))]);    \
   } while (0)

void tfb_draw_char(int x, int y, u32 fg_color, u32 bg_color, u8 c)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   u8 *data = curr_font_data + curr_font_bytes_per_glyph * c;
   const u32 arr[] = { fg_color, bg_color };

   /*
    * NOTE: the following algorithm is certainly not the fastest way to draw
    * a character on-screen, but its performance is pretty good, in particular
    * for text that does not have to change continuosly (like a console).
    * Actually, this algorithm is used by Tilck[1]'s framebuffer console in a
    * fail-safe case for drawing characters on-screen: on low resolutions,
    * its performance is pretty acceptable on modern machines, even when used
    * by a console to full-redraw a screen with text. Therefore, for the
    * purposes of this library (mostly to show static text on-screen), the
    * following implementation is absolutely good enough.
    *
    * -------------------------------------------------
    * [1] Tilck [A Tiny Linux-Compatible Kernel]
    *     https://github.com/vvaltchev/tilck
    */

   if (curr_font_w_bytes == 1)

      for (u32 row = y; row < (y + curr_font_h); row++) {
         draw_char_partial(0);
         data += curr_font_w_bytes;
      }

   else if (curr_font_w_bytes == 2)

      for (u32 row = y; row < (y + curr_font_h); row++) {
         draw_char_partial(0);
         draw_char_partial(1);
         data += curr_font_w_bytes;
      }

   else

      for (u32 row = y; row < (y + curr_font_h); row++) {

         for (u32 b = 0; b < curr_font_w_bytes; b++) {
            draw_char_partial(b);
         }

         data += curr_font_w_bytes;
      }
}

void tfb_draw_char_scaled(int x, int y,
                          u32 fg, u32 bg, int xscale, int yscale, u8 c)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   if (xscale < 0)
      x += -xscale * curr_font_w;

   if (yscale < 0)
      y += -yscale * curr_font_h;

   u8 *d = curr_font_data + curr_font_bytes_per_glyph * c;

   /*
    * NOTE: this algorithm is clearly much slower than the simpler variant
    * used in tfb_draw_char(), but it is still pretty good for static text.
    * In case better performance is needed, the proper solution would be to use
    * a scaled font instead of the *_scaled draw text functions.
    */

   for (u32 row = 0; row < curr_font_h; row++, d += curr_font_w_bytes)
      for (u32 b = 0; b < curr_font_w_bytes; b++)
         for (u32 bit = 0; bit < 8; bit++) {

            const int xoff = xscale * ((b << 3) + 8 - bit - 1);
            const int yoff = yscale * row;
            const u32 color = (d[b] & (1 << bit)) ? fg : bg;

            tfb_fill_rect(x + xoff, y + yoff, xscale, yscale, color);
         }
}

void tfb_draw_string(int x, int y, u32 fg_color, u32 bg_color, const char *s)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   for (; *s; s++, x += curr_font_w) {
      tfb_draw_char(x, y, fg_color, bg_color, *s);
   }
}

void tfb_draw_string_scaled_wrapped(int x, int y,
                                    u32 fg, u32 bg,
                                    int xscale, int yscale, u32 wrap_col,
                                    const char *s)
{
   int base_x = x;
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   const int xs = xscale > 0 ? xscale : -xscale;

   for (int i = 0; *s; s++, x += xs * curr_font_w, i++) {
      if ((*s == '\n' && *++s) || (wrap_col > 0 && i % wrap_col == 0))
      {
         y += curr_font_h * yscale + 2;
         x = base_x;
      }
      tfb_draw_char_scaled(x, y, fg, bg, xscale, yscale, *s);
   }
}

void tfb_draw_string_scaled(int x, int y,
                            u32 fg, u32 bg,
                            int xscale, int yscale, const char *s)
{
   if (!curr_font) {
      fprintf(stderr, "[tfblib] ERROR: no font currently selected\n");
      return;
   }

   const int xs = xscale > 0 ? xscale : -xscale;

   for (; *s; s++, x += xs * curr_font_w) {
      tfb_draw_char_scaled(x, y, fg, bg, xscale, yscale, *s);
   }
}

void tfb_draw_xcenter_string(int cx, int y, u32 fg, u32 bg, const char *s)
{
   tfb_draw_string(cx - curr_font_w * strlen(s) / 2, y, fg, bg, s);
}

void tfb_draw_xcenter_string_scaled(int cx, int y,
                                    u32 fg, u32 bg,
                                    int xscale, int yscale, const char *s)
{
   tfb_draw_string_scaled(cx - xscale * curr_font_w * strlen(s) / 2,
                          y, fg, bg, xscale, yscale, s);
}

int tfb_get_curr_font_width(void)
{
   return curr_font_w;
}

int tfb_get_curr_font_height(void)
{
   return curr_font_h;
}
