/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <linux/fb.h>

typedef uint8_t u8;
typedef uint32_t u32;

extern struct fb_var_screeninfo __fbi;

extern void *__fb_buffer;
extern size_t __fb_pitch_div4;

u32 tfb_make_color(u8 red, u8 green, u8 blue);

void tfb_draw_hline(u32 x, u32 y, u32 len, u32 color);
void tfb_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void tfb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void tfb_clear_screen(u32 color);

int tfb_acquire_fb(void);
void tfb_release_fb(void);

static inline void set_pixel(u32 x, u32 y, u32 color)
{
   ((volatile u32 *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

static u32 inline tfb_screen_width(void)
{
    return __fbi.xres;
}

static u32 inline tfb_screen_height(void)
{
    return __fbi.yres;
}

#define TFB_SUCCESS                  0
#define TFB_ERROR_OPEN_FB            1
#define TFB_ERROR_IOCTL_FB           2
#define TFB_ERROR_OPEN_TTY           3
#define TFB_ERROR_TTY_GRAPHIC_MODE   4
#define TFB_ASSUMPTION_FAILED        5
#define TFB_MMAP_FB_ERROR            6
