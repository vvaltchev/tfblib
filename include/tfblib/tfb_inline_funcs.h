
#ifndef _TFBLIB_H_
#error Never include tfb_inline_funcs.h directly. Include <tfblib/tfblib.h>.
#endif

/* Essential variables */
extern void *__fb_buffer;
extern void *__fb_real_buffer;
extern u32 __fb_screen_w;
extern u32 __fb_screen_h;
extern size_t __fb_size;
extern size_t __fb_pitch;
extern size_t __fb_pitch_div4; /* see the comment in drawing.c */

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

inline u32 tfb_make_color(u8 r, u8 g, u8 b)
{
   return ((r << __fb_r_pos) & __fb_r_mask) |
          ((g << __fb_g_pos) & __fb_g_mask) |
          ((b << __fb_b_pos) & __fb_b_mask);
}

inline void tfb_draw_pixel_raw(u32 x, u32 y, u32 color)
{
   ((volatile u32 *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

inline void tfb_draw_pixel(u32 x, u32 y, u32 color)
{
   x += __fb_off_x;
   y += __fb_off_y;

   if (x < __fb_win_end_x && y < __fb_win_end_y)
      ((volatile u32 *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

inline u32 tfb_screen_width(void) { return __fb_screen_w; }
inline u32 tfb_screen_height(void) { return __fb_screen_h; }
inline u32 tfb_win_width(void) { return __fb_win_w; }
inline u32 tfb_win_height(void) { return __fb_win_h; }
