/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <tfblib/tfblib.h>

uint32_t red, green, blue, white, black, yellow, gray;

void init_colors(void)
{
   white = tfb_make_color(255, 255, 255);
   black = tfb_make_color(0, 0, 0);
   red = tfb_make_color(255, 0, 0);
   green = tfb_make_color(0, 255, 0);
   blue = tfb_make_color(0, 0, 255);
   yellow = tfb_make_color(255, 255, 0);
   gray = tfb_make_color(50, 50, 50);
}


bool font_iter_callback(tfb_font_info *fi, void *user_arg)
{
   printf("    font '%s', psf%u, %u x %u\n",
          fi->name, fi->psf_version, fi->width, fi->height);
   return true;
}

bool font_iter_cb_select_font32x16(tfb_font_info *fi, void *user_arg)
{
   if (fi->width == 16 && fi->height == 32) {

      int rc = tfb_set_current_font(fi->font_id);

      if (rc != TFB_SUCCESS) {
         fprintf(stderr, "tfb_set_current_font() failed with error: %d\n", rc);
         abort();
      }

      printf("Selected font '%s'\n", fi->name);
      return false; /* stop iterating over fonts */
   }

   return true;
}

int main(int argc, char **argv)
{
   int rc;

   printf("Available fonts:\n");
   tfb_iterate_over_fonts(font_iter_callback, NULL);
   tfb_iterate_over_fonts(font_iter_cb_select_font32x16, NULL);

   if (argc == 2) {

      void *font_id;
      rc = tfb_dyn_load_font(argv[1], &font_id);

      if (rc != TFB_SUCCESS) {
         fprintf(stderr, "tfb_dyn_load_font(%s) failed with error: %d\n",
                 argv[1], rc);

         return 1;
      }

      printf("Setting the dynamically-loaded font '%s'\n", argv[1]);
      rc = tfb_set_current_font(font_id);

      if (rc != TFB_SUCCESS) {
         fprintf(stderr, "tfb_set_current_font() failed with error: %d\n", rc);
         return 1;
      }
   }

   rc = tfb_acquire_fb(NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   init_colors();
   tfb_clear_screen(black);
   tfb_draw_string(20, 20,
                   yellow, gray, "Hello from the Tiny Framebuffer Lib!");
   getchar();

   tfb_release_fb();
   return 0;
}
