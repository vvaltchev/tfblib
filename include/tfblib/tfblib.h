/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <linux/fb.h>

/* Error codes */
#define TFB_SUCCESS                  0
#define TFB_ERROR_OPEN_FB            1
#define TFB_ERROR_IOCTL_FB           2
#define TFB_ERROR_OPEN_TTY           3
#define TFB_ERROR_TTY_GRAPHIC_MODE   4
#define TFB_ASSUMPTION_FAILED        5
#define TFB_MMAP_FB_ERROR            6
#define TFB_INVALID_WINDOW           7

typedef uint8_t u8;
typedef uint32_t u32;

extern struct fb_var_screeninfo __fbi;
extern void *__fb_buffer;
extern size_t __fb_pitch_div4;

/* Window-related variables */
extern u32 __fb_win_w;
extern u32 __fb_win_h;
extern u32 __fb_off_x;
extern u32 __fb_off_y;
extern u32 __fb_win_end_x;
extern u32 __fb_win_end_y;

/* Color-related variables */
extern u32 __fb_r_mask;
extern u32 __fb_g_mask;
extern u32 __fb_b_mask;
extern u8 __fb_r_mask_size;
extern u8 __fb_g_mask_size;
extern u8 __fb_b_mask_size;
extern u8 __fb_r_pos;
extern u8 __fb_g_pos;
extern u8 __fb_b_pos;

/* Initialization/setup functions */
int tfb_set_window(u32 x, u32 y, u32 w, u32 h);
int tfb_set_center_window_size(u32 w, u32 h);
int tfb_acquire_fb(void);
void tfb_release_fb(void);

/* Drawing functions */
void tfb_draw_hline(u32 x, u32 y, u32 len, u32 color);
void tfb_draw_vline(u32 x, u32 y, u32 len, u32 color);
void tfb_draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color);
void tfb_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void tfb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void tfb_clear_screen(u32 color);
void tfb_clear_win(u32 color);

inline u32 tfb_make_color(u8 r, u8 g, u8 b)
{
   return ((r << __fb_r_pos) & __fb_r_mask) |
          ((g << __fb_g_pos) & __fb_g_mask) |
          ((b << __fb_b_pos) & __fb_b_mask);
}

inline void tfb_draw_pixel(u32 x, u32 y, u32 color)
{
   ((volatile u32 *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

inline void tfb_draw_pixel_win(u32 x, u32 y, u32 color)
{
   x += __fb_off_x;
   y += __fb_off_y;

   if (x < __fb_win_end_x && y < __fb_win_end_y)
      ((volatile u32 *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

inline u32 tfb_screen_width(void)
{
    return __fbi.xres;
}

inline u32 tfb_screen_height(void)
{
    return __fbi.yres;
}

inline u32 tfb_win_width(void)
{
   return __fb_win_w;
}

inline u32 tfb_win_height(void)
{
   return __fb_win_h;
}
