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
 * @param[in] flags        One or more among: #TFB_FL_NO_TTY_KD_GRAPHICS,
 *                         #TFB_FL_USE_DOUBLE_BUFFER.
 *
 * @param[in] fb_device    The framebuffer device file. Can be NULL.
 *                         Defaults to /dev/fb0.
 *
 * @param[in] tty_device   The tty device file to use for setting tty in
 *                         graphics mode. Can be NULL. Defaults to /dev/tty.
 *
 * @return                 #TFB_SUCCESS in case of success or one of the
 *                         following errors:
 *                             #TFB_ERR_OPEN_FB,
 *                             #TFB_ERR_IOCTL_FB,
 *                             #TFB_ERR_UNSUPPORTED_VIDEO_MODE,
 *                             #TFB_ERR_TTY_GRAPHIC_MODE,
 *                             #TFB_ERR_MMAP_FB,
 *                             #TFB_ERR_OUT_OF_MEMORY.
 *
 * \note This function does not affect the kb mode. tfb_set_kb_raw_mode() can
 *       be called before or after tfb_acquire_fb().
 */
int tfb_acquire_fb(u32 flags, const char *fb_device, const char *tty_device);


/**
 * Release the framebuffer device
 *
 * \note    The function **must** be called before exiting, otherwise the TTY
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
 * @return           #TFB_SUCCESS in case of success or #TFB_ERR_INVALID_WINDOW.
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
 * @return           #TFB_SUCCESS in case of success or #TFB_ERR_INVALID_WINDOW.
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

/**
 * When passed to the 'w' param of tfb_set_font_by_size(), means that any font
 * width is acceptable.
 */
#define TFB_FONT_ANY_WIDTH   0

/**
 * When passed to the 'h' param of tfb_set_font_by_size(), means that any font
 * height is acceptable.
 */
#define TFB_FONT_ANY_HEIGHT  0

/** @} */


/**
 * Font info structure
 *
 * tfb_iterate_over_fonts() passes a pointer to a tfb_font_info * structure for
 * each statically embedded font in the library to the callback function.
 */
typedef struct {

   const char *name; /**< Font's file name */
   u32 width;        /**< Font's character width in pixels */
   u32 height;       /**< Font's character height in pixels */
   u32 psf_version;  /**< PSF version: either 1 or 2 */
   void *font_id;    /**< An opaque identifier of the font */

} tfb_font_info;

/**
 * Callback type accepted by tfb_iterate_over_fonts().
 */
typedef bool (*tfb_font_iter_func)(tfb_font_info *cb, void *user_arg);

/**
 * Iterate over the fonts embedded in the library.
 *
 * tfb_iterate_over_fonts() calls 'cb' once for each embedded font passing to
 * it a pointer a tfb_font_info structure and the user_arg until either the
 * font list is over or the callback returned false.
 *
 * @param[in] cb           An user callback function
 * @param[in] user_arg     An arbitrary user pointer that will be passed to the
 *                         callback function.
 */
void tfb_iterate_over_fonts(tfb_font_iter_func cb, void *user_arg);

/**
 * Set the font used by the functions for drawing text
 *
 * @param[in] font_id      An opaque identifier provided by the library either
 *                         as a member of tfb_font_info, or returned as an out
 *                         parameter by tfb_dyn_load_font().
 *
 * @return                 #TFB_SUCCESS in case of success or
 *                         #TFB_ERR_INVALID_FONT_ID otherwise.
 */
int tfb_set_current_font(void *font_id);

/**
 * Load dynamically a PSF font file
 *
 * @param[in]     file     File path
 * @param[in,out] font_id  Address of a void *font_id opaque pointer that will
 *                         be set by the function in case of success.
 *
 * @return                 #TFB_SUCCESS in case of success or one of the
 *                         following errors:
 *                             #TFB_ERR_READ_FONT_FILE_FAILED,
 *                             #TFB_ERR_OUT_OF_MEMORY.
 */
int tfb_dyn_load_font(const char *file, void **font_id);

/**
 * Unload a dynamically-loaded font
 *
 * @param[in]     font_id  Opaque pointer returned by tfb_dyn_load_font()
 *
 * @return                 #TFB_SUCCESS in case of success or
 *                         #TFB_ERR_NOT_A_DYN_LOADED_FONT if the caller passed
 *                         to it the font_id of an embedded font.
 */
int tfb_dyn_unload_font(void *font_id);

/**
 * Select the first font matching the given (w, h) criteria
 *
 * The tfb_set_font_by_size() function iterates over the fonts embedded in the
 * library and sets the first font having width = w and height = h.
 *
 * @param[in]     w        Desired width of the font.
 *                         The caller may pass #TFB_FONT_ANY_WIDTH to tell the
 *                         function that any font width is acceptable.
 *
 * @param[in]     h        Desired height of the font.
 *                         The caller may pass #TFB_FONT_ANY_HEIGHT to tell the
 *                         function that any font width is acceptable.
 *
 * @return        #TFB_SUCCESS in case a font matching the given criteria has
 *                been found or #TFB_ERR_FONT_NOT_FOUND otherwise.
 */
int tfb_set_font_by_size(int w, int h);

/**
 * Get current font's width
 *
 * @return        the width (in pixels) of the current font or 0 in case there
 *                is no currently selected font.
 */
int tfb_get_curr_font_width(void);

/**
 * Get current font's height
 *
 * @return        the height (in pixels) of the current font or 0 in case there
 *                is no currently selected font.
 */
int tfb_get_curr_font_height(void);



/*
 * ----------------------------------------------------------------------------
 *
 * KB input functions and definitions
 *
 * ----------------------------------------------------------------------------
 */


/**
 * Library's type used to represent keystrokes
 */
typedef uint64_t tfb_key_t;

/**
 * \addtogroup flags Flags
 * @{
 */


/**
 * Non-blocking input mode
 *
 * When passed to tfb_set_kb_raw_mode(), this flag makes the TTY input to be
 * non-blocking.
 */
#define TFB_FL_KB_NONBLOCK (1 << 2)

/* @} */


/**
 * Set the TTY keyboard input to raw mode
 *
 * @param[in]  flags    Default value: 0. The only currently supported flag is
 *                      #TFB_FL_KB_NONBLOCK.
 *
 * @return              #TFB_SUCCESS in case of success, otherwise one of the
 *                      following errors:
 *                            #TFB_ERR_KB_WRONG_MODE,
 *                            #TFB_ERR_KB_MODE_GET_FAILED,
 *                            #TFB_ERR_KB_MODE_SET_FAILED.
 */
int tfb_set_kb_raw_mode(u32 flags);

/**
 * Restore the TTY keyboard input to its previous state
 *
 * @return              #TFB_SUCCESS in case of success, otherwise one of the
 *                      following errors:
 *                            #TFB_ERR_KB_WRONG_MODE,
 *                            #TFB_ERR_KB_MODE_SET_FAILED.
 */
int tfb_restore_kb_mode(void);

/**
 * Read a keystroke
 *
 * @return              An ASCII character or one of the values TFB_KEY_*.
 *                      In case the input is non-blocking (#TFB_FL_KB_NONBLOCK),
 *                      returns 0 in case there was no keystroke to return.
 */
tfb_key_t tfb_read_keypress(void);

/**
 * Get the number of the F key corresponding to 'k'
 *
 * @return              A number in the range [1, 12] in case 'k' is equal to
 *                      one of the TFB_KEY_F* values. Otherwise, return 0.
 */
int tfb_get_fn_key_num(tfb_key_t k);


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
#include "tfb_kb.h"

/* undef the the convenience types defined above */
#undef u8
#undef u32
