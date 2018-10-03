/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_colors.h>

int main(int argc, char **argv)
{
   int rc;

   if ((rc = tfb_acquire_fb(0, NULL, NULL)) != TFB_SUCCESS) {
      fprintf(stderr, "tfb_acquire_fb() failed with error code: %d\n", rc);
      return 1;
   }

   uint32_t w = tfb_screen_width();
   uint32_t h = tfb_screen_height();
   uint32_t rect_w = 200;
   uint32_t rect_h = 200;

   /* Paint the whole screen in black */
   tfb_clear_screen(tfb_black);

   /* Draw a red rectangle at the center of the screen */

   tfb_draw_rect(w / 2 - rect_w / 2,  /* x coordinate */
                 h / 2 - rect_h / 2,  /* y coordinate */
                 rect_w,              /* width */
                 rect_h,              /* height */
                 tfb_red              /* color */);

   getchar();
   tfb_release_fb();
   return 0;
}
