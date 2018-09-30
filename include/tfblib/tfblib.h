/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file tfblib.h
 * @brief The library's main header file
 */

#pragma once
#define _TFBLIB_H_

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "tfb_errors.h"

/// Convenience macro used to shorten the signatures. Undefined at the end.
#define u8 uint8_t

/// Convenience macro used to shorten the signatures. Undefined at the end.
#define u32 uint32_t

/*
 * ----------------------------------------------------------------------------
 *
 * Initialization/setup functions and definitions
 *
 * ----------------------------------------------------------------------------
 */

/**
 * \addtogroup flags Flags
 * @{
 */

/**
 * Do NOT put TTY in graphics mode.
 *
 * Passing this flag to tfb_acquire_fb() will
 * allow to use the framebuffer and to see stdout on TTY as well. That usually
 * is undesirable because the text written to TTY will overwrite the graphics.
 */
#define TFB_FL_NO_TTY_KD_GRAPHICS   (1 << 0)

/**
 * Do NOT write directly onto the framebuffer.
 *
 * Passing this flag to tfb_acquire_fb() will make it allocate a regular memory
 * buffer where all the writes (while drawing) will be directed to. The changes
 * will appear on-screen only after manually called tfb_flush_rect() or
 * tfb_flush_rect(). This flag is useful for applications needing to clean and
 * redraw the whole screen (or part of it) very often (e.g. games) in order to
 * avoid the annoying flicker effect.
 */
#define TFB_FL_USE_DOUBLE_BUFFER    (1 << 1)

/** @} */

/**
 * Opens and maps the framebuffer device in the current address space
 *
 * A successful call to tfb_acquire_fb() is mandatory before calling any drawing
 * functions, including the tfb_clear_* and tfb_flush_* functions.
 *
 * @param[in] flags        One or more (OR-ed) among: TFB_FL_NO_TTY_KD_GRAPHICS,
 *                         TFB_FL_USE_DOUBLE_BUFFER.
 *
 * @param[in] fb_device    The framebuffer device file. Can be NULL.
 *                         Defaults to /dev/fb0.
 *
 * @param[in] tty_device   The tty device file to use for setting tty in
 *                         graphics mode. Can be NULL. Defaults to /dev/tty.
 *
 * @return                 TFB_SUCCESS in case of success or one of the errors
 *                         defined in tfb_errors.h. See \ref ErrorCodes.
 *
 * \note This function does not affect the kb mode. tfb_set_kb_raw_mode() can
 *       be called before or after tfb_acquire_fb().
 */
int tfb_acquire_fb(u32 flags, const char *fb_device, const char *tty_device);


/**
 * Release the framebuffer device
 *
 * \note    The function **must** be called before exiting, otherwise the tty
 *          will remain in graphics mode and be unusable.
 *
 * \note    This function does not affect the kb mode. If tfb_set_kb_raw_mode()
 *          has been used, tfb_restore_kb_mode() must be called to restore the
 *          kb mode to its original value.
 */
void tfb_release_fb(void);

/**
 * Limit the drawing to a window at (x, y) having size (w, h)
 *
 * In case the application does not want to use the whole screen, it can call
 * this function to get the coordinate system shifted by (+x, +y) and everything
 * outside of it just cut off. Using windows smaller than then screen could
 * improve application's performance.
 *
 * @param[in] x      X coordinate of the window, in pixels
 * @param[in] y      Y coordinate of the window, in pixels
 * @param[in] w      Width of the window, in pixels
 * @param[in] h      Height of the window, in pixels
 *
 * @return           TFB_SUCCESS in case of success or one of the errors
 *                   defined in tfb_errors.h. See \ref ErrorCodes.
 */
int tfb_set_window(u32 x, u32 y, u32 w, u32 h);

/**
 * Limit the drawing to a window having size (w, h) at the center of the screen
 *
 * tfb_set_center_window_size() is a wrapper of tfb_set_window() which just
 * calculates the (x, y) coordinates of the window in order it to be at the
 * center of the screen.
 *
 * @param[in] w      Width of the window, in pixels
 * @param[in] h      Height of the window, in pixels
 *
 * @return           TFB_SUCCESS in case of success or one of the errors
 *                   defined in tfb_errors.h. See \ref ErrorCodes.
 */
int tfb_set_center_window_size(u32 w, u32 h);



/*
 * ----------------------------------------------------------------------------
 *
 * Text-related functions and definitions
 *
 * ----------------------------------------------------------------------------
 */

/**
 * \addtogroup flags Flags
 * @{
 */

#define TFB_FONT_ANY_WIDTH   0
#define TFB_FONT_ANY_HEIGHT  0

/** @} */

typedef struct {

   const char *name;
   u32 width;
   u32 height;
   u32 psf_version; /* 1 or 2 */
   void *font_id;

} tfb_font_info;

typedef bool (*tfb_font_iter_func)(tfb_font_info *, void *);
void tfb_iterate_over_fonts(tfb_font_iter_func f, void *user_arg);
int tfb_set_current_font(void *font_id);
int tfb_dyn_load_font(const char *file, void **font_id /* out */);
int tfb_dyn_unload_font(void *font_id);
int tfb_set_font_by_size(int w, int h);
int tfb_get_curr_font_width(void);
int tfb_get_curr_font_height(void);



/*
 * ----------------------------------------------------------------------------
 *
 * KB input functions and definitions
 *
 * ----------------------------------------------------------------------------
 */

typedef uint64_t tfb_key_t;

#define TFB_FL_KB_NONBLOCK (1 << 2)

int tfb_set_kb_raw_mode(u32 flags);
int tfb_restore_kb_mode(void);
tfb_key_t tfb_read_keypress(void);



/*
 * ----------------------------------------------------------------------------
 *
 * Drawing functions
 *
 * ----------------------------------------------------------------------------
 */

inline void tfb_draw_pixel(u32 x, u32 y, u32 color);
void tfb_draw_hline(u32 x, u32 y, u32 len, u32 color);
void tfb_draw_vline(u32 x, u32 y, u32 len, u32 color);
void tfb_draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color);
void tfb_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void tfb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void tfb_draw_char(u32 x, u32 y, u32 fg_color, u32 bg_color, u8 c);
void tfb_draw_string(u32 x, u32 y, u32 fg_color, u32 bg_color, const char *s);
void tfb_draw_center_string(u32 cx, u32 y, u32 fg, u32 bg, const char *s);
void tfb_draw_char_scaled(u32 x, u32 y, u32 fg, u32 bg,
                          u32 xscale, u32 yscale, u8 c);
void tfb_draw_string_scaled(u32 x, u32 y, u32 fg, u32 bg,
                            u32 xscale, u32 yscale, const char *s);
void tfb_draw_center_string_scaled(u32 cx, u32 y, u32 fg, u32 bg,
                                   u32 xscale, u32 yscale, const char *s);
void tfb_clear_screen(u32 color);
void tfb_clear_win(u32 color);
void tfb_flush_window(void);
void tfb_flush_rect(u32 x, u32 y, u32 w, u32 h);

#include "tfb_inline_funcs.h"

/* undef the the convenience types defined above */
#undef u8
#undef u32
