/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <tfblib/tfblib.h>
#include <tfblib/tfb_colors.h>


bool font_iter_callback(tfb_font_info *fi, void *user_arg)
{
   printf("    font '%s', psf%u, %u x %u\n",
          fi->name, fi->psf_version, fi->width, fi->height);
   return true;
}

bool font_iter_cb_select_font16(tfb_font_info *fi, void *user_arg)
{
   if (fi->height == 16) {

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

   /*
    * Set a font after searching for the best fit with a custom callback.
    * If the criteria is just the size as in this case, the more convenient
    * utility function tfb_set_font_by_size() can be used.
    */
   tfb_iterate_over_fonts(font_iter_cb_select_font16, NULL);

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

   rc = tfb_acquire_fb(0, NULL, NULL);

   if (rc != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      tfb_release_fb();
      return 1;
   }

   tfb_clear_screen(tfb_black);

   tfb_draw_string(20, 20,
                   tfb_yellow, tfb_gray,
                   "Using font 8x16!! [Press ENTER to exit]");

   tfb_set_font_by_size(16, 32);

   tfb_draw_string(20, tfb_get_curr_font_height() * 2,
                   tfb_blue, tfb_gray,
                   "Using font 16x32");

   tfb_draw_string_scaled(20, tfb_get_curr_font_height() * 4,
                          tfb_blue, tfb_gray, 2, 1, "16x32 scaled 2 x 1");

   tfb_draw_string_scaled(20, tfb_get_curr_font_height() * 6,
                          tfb_blue, tfb_gray, 1, 2, "16x32 scaled 1 x 2");

   tfb_draw_string_scaled(20, tfb_get_curr_font_height() * 9,
                          tfb_blue, tfb_gray, 2, 2, "16x32 scaled 2 x 2");

   tfb_draw_string_scaled(20, tfb_get_curr_font_height() * 12,
                          tfb_red, tfb_gray, -2, 2, "16x32 scaled -2 x 2");

   tfb_draw_string_scaled(20, tfb_get_curr_font_height() * 15,
                          tfb_green, tfb_gray, 2, -2, "16x32 scaled 2 x -2");

   getchar();

   tfb_release_fb();
   return 0;
}
