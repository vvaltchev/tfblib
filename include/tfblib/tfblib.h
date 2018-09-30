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

/**
 * \addtogroup ErrorCodes Error Codes
 * @{
 */

/// The call completed successfully without any errors.
#define TFB_SUCCESS                  0

/// open() failed on the framebuffer device
#define TFB_ERROR_OPEN_FB            1

/// ioctl() failed on the framebuffer device file descriptor
#define TFB_ERROR_IOCTL_FB           2

/// open() failed on the tty device
#define TFB_ERROR_OPEN_TTY           3

/// ioctl() on the tty failed while trying to set tty in graphic mode
#define TFB_ERROR_TTY_GRAPHIC_MODE   4

/// mmap() on the framebuffer file description failed
#define TFB_MMAP_FB_ERROR            6

/// Invalid window position/size
#define TFB_INVALID_WINDOW           7

/// Unsupported video mode
#define TFB_UNSUPPORTED_VIDEO_MODE   8

/// The supplied font_id is invalid
#define TFB_INVALID_FONT_ID          9

/// Unable to open/read/load the supplied font file
#define TFB_READ_FONT_FILE_FAILED   10

/// Out of memory (malloc() returned 0)
#define TFB_OUT_OF_MEMORY           11

/// The supplied font_id is does not belog to a dynamically loaded font
#define TFB_NOT_A_DYN_LOADED_FONT   12

/// The keyboard input is not in the expected mode (e.g. already in raw mode)
#define TFB_KB_WRONG_MODE           13

/// Unable to get a keyboard input paramater with ioctl()
#define TFB_KB_MODE_GET_FAILED      14

/// Unable to set a keyboard input paramater with ioctl()
#define TFB_KB_MODE_SET_FAILED      15

/// Unable to find a font matching the criteria
#define TFB_FONT_NOT_FOUND          16
/** @} */


/// Convenience macro used to make the signatures shorter. Undefined at the end.
#define u8 uint8_t

/// Convenience macro used to make the signatures shorter. Undefined at the end.
#define u32 uint32_t


/* Initialization/setup functions and definitions */

#define TFB_FL_NO_TTY_KD_GRAPHICS   (1 << 0)
#define TFB_FL_USE_SHADOW_BUFFER    (1 << 1)

/**
 * The main initialization function of Tfblib
 *
 * A successful call to tfb_acquire_fb() is mandatory before calling any drawing
 * functions, including the tfb_clear_* and tfb_flush_* functions.
 *
 * @param fb_device     The framebuffer device file (optional).
 *                      Defaults to /dev/fb0.
 *
 * @param tty_device    The tty device file to use for setting tty in graphics
 *                      mode. Defaults to /dev/tty. Ignored when flags bit mask
 *                      contains TFB_FL_NO_TTY_KD_GRAPHICS.
 *
 * @return              TFB_SUCCESS in case of success or one of the errors
 *                      defined as TFB_ERROR_*.
 *
 * \note This function does not affect the kb mode. tfb_set_kb_raw_mode() can
 *       be called before or after tfb_acquire_fb().
 */
int tfb_acquire_fb(u32 flags, const char *fb_device, const char *tty_device);


/**
 * Release the framebuffer.
 *
 * It must be called before exiting, otherwise the tty will remain in graphics
 * mode and be unusable.
 */
void tfb_release_fb(void);
int tfb_set_window(u32 x, u32 y, u32 w, u32 h);
int tfb_set_center_window_size(u32 w, u32 h);

/* Text-related functions and definitions */

#define TFB_FONT_ANY_WIDTH   0
#define TFB_FONT_ANY_HEIGHT  0

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


/* KB input functions and definitions */

typedef uint64_t tfb_key_t;

#define TFB_FL_KB_NONBLOCK (1 << 2)

int tfb_set_kb_raw_mode(u32 flags);
int tfb_restore_kb_mode(void);
tfb_key_t tfb_read_keypress(void);

/* Drawing functions */

/**
 * The Fastest low-level draw pixel function
 *
 * \warning
 *      Using this function is UNSAFE:
 *        - the caller have to offset (x,y) by (__fbi.xoffset, __fbi.yoffset)
 *          in case one of the offsets is != 0.
 *        - the caller takes the full responsibility to avoid using coordinates
 *          outside of the screen boundaries. Doing that would cause an
 *          undefined behavior.
 */
inline void tfb_draw_pixel_raw(u32 x, u32 y, u32 color);
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
