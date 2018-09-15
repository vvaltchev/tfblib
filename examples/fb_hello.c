/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <tfblib/tfblib.h>

int main(int argc, char **argv)
{
    int rc;

    rc = tfb_acquire_fb();

    if (rc != TFB_SUCCESS) {
        fprintf(stderr, "tfb_acquire_fb failed with: %d\n", rc);
        tfb_release_fb();
        return 1;
    }

    tfb_clear_screen(tfb_make_color(0, 0, 0));

    tfb_fill_rect(tfb_screen_width()/8,
                  tfb_screen_height()/8,
                  tfb_screen_width()/4,
                  tfb_screen_height()/4,
                  tfb_make_color(255, 0, 0));
    getchar();

    tfb_release_fb();
    return 0;
}
