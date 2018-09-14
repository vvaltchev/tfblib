/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <tfblib/tfblib.h>

int main(int argc, char **argv)
{
    if (!tfb_acquire_fb()) {
        tfb_release_fb();
        return 1;
    }

    tfb_clear_screen(tfb_make_color(0, 0, 0));

    tfb_draw_rect(tfb_screen_width()/8,
                  tfb_screen_height()/8,
                  tfb_screen_width()/4,
                  tfb_screen_height()/4,
                  tfb_make_color(255, 0, 0));
    getchar();

    tfb_release_fb();
    return 0;
}