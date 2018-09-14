/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <linux/fb.h>

extern struct fb_var_screeninfo __fbi;

extern char *__fb_buffer;
extern size_t __fb_pitch_div4;

uint32_t tfb_make_color(uint8_t red, uint8_t green, uint8_t blue);
void tfb_draw_rect(uint32_t x, uint32_t y,
                   uint32_t w, uint32_t h, uint32_t color);
void tfb_clear_screen(uint32_t color);
bool tfb_acquire_fb(void);
void tfb_release_fb(void);

static inline void set_pixel(uint32_t x, uint32_t y, uint32_t color)
{
   ((volatile uint32_t *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

static uint32_t inline tfb_screen_width(void)
{
    return __fbi.xres;
}

static uint32_t inline tfb_screen_height(void)
{
    return __fbi.yres;
}
