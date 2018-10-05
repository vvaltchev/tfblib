/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file tfblib.h
 * @brief Tfblib's main header file
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
 * Opaque font type
 */
typedef void *tfb_font_t;

/**
 * Font info structure
 *
 * tfb_iterate_over_fonts() passes a pointer to a tfb_font_info * structure for
 * each statically embedded font in the library to the callback function.
 */
typedef struct {

   const char *name;     /**< Font's file name */
   u32 width;            /**< Font's character width in pixels */
   u32 height;           /**< Font's character height in pixels */
   u32 psf_version;      /**< PSF version: either 1 or 2 */
   tfb_font_t font_id;   /**< An opaque identifier of the font */

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
int tfb_set_current_font(tfb_font_t font_id);

/**
 * Load dynamically a PSF font file
 *
 * @param[in]     file     File path
 * @param[in,out] font_id  Address of a tfb_font_t variable that will
 *                         be set by the function in case of success.
 *
 * @return                 #TFB_SUCCESS in case of success or one of the
 *                         following errors:
 *                             #TFB_ERR_READ_FONT_FILE_FAILED,
 *                             #TFB_ERR_OUT_OF_MEMORY.
 */
int tfb_dyn_load_font(const char *file, tfb_font_t *font_id);

/**
 * Unload a dynamically-loaded font
 *
 * @param[in]     font_id  Opaque pointer returned by tfb_dyn_load_font()
 *
 * @return                 #TFB_SUCCESS in case of success or
 *                         #TFB_ERR_NOT_A_DYN_LOADED_FONT if the caller passed
 *                         to it the font_id of an embedded font.
 */
int tfb_dyn_unload_font(tfb_font_t font_id);

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
 * Drawing functions
 *
 * ----------------------------------------------------------------------------
 */

/**
 * Get a representation of the RGB color (r, g, b) for the current video mode
 *
 * @param[in]  r        Red color component [0, 255]
 * @param[in]  g        Green color component [0, 255]
 * @param[in]  b        Blue color component [0, 255]
 *
 * @return              A framebuffer-specific representation of the RGB color
 *                      passed in the r, g, b parameters.
 */
inline u32 tfb_make_color(u8 r, u8 g, u8 b);

/**
 * Set the color of the pixel at (x, y) to 'color'
 *
 * @param[in]  x        Window-relative X coordinate of the pixel
 * @param[in]  y        Window-relative Y coordinate of the pixel
 * @param[in]  color    A color returned by tfb_make_color()
 *
 * \note By default, the library uses as "window" the whole screen, therefore
 *       by default the point (x, y) corresponds to the pixel at (x, y) on the
 *       screen. But, after calling tfb_set_window() the origin of the
 *       coordinate system gets shifted.
 */
inline void tfb_draw_pixel(u32 x, u32 y, u32 color);

/**
 * Draw a horizonal line on-screen
 *
 * @param[in]  x        Window-relative X coordinate of line's first point
 * @param[in]  y        Window-relative Y coordinate of line's first point
 * @param[in]  len      Length of the line, in pixels
 * @param[in]  color    Color of the line. See tfb_make_color().
 *
 * Calling tfb_draw_hline(x, y, len, color) is equivalent to calling:
 *       tfb_draw_line(x, y, x + len, y, color)
 *
 * The only difference between the two functions is in the implementation: given
 * the simpler task of tfb_draw_hline(), it can be implemented in much more
 * efficient way.
 */
void tfb_draw_hline(int x, int y, int len, u32 color);

/**
 * Draw a vertical line on-screen
 *
 * @param[in]  x        Window-relative X coordinate of line's first point
 * @param[in]  y        Window-relative Y coordinate of line's first point
 * @param[in]  len      Length of the line, in pixels
 * @param[in]  color    Color of the line. See tfb_make_color().
 *
 * Calling tfb_draw_vline(x, y, len, color) is equivalent to calling:
 *       tfb_draw_line(x, y, x, y + len, color)
 *
 * The only difference between the two functions is in the implementation: given
 * the simpler task of tfb_draw_vline(), it can be implemented in much more
 * efficient way.
 */
void tfb_draw_vline(int x, int y, int len, u32 color);

/**
 * Draw a line on-screen
 *
 * @param[in]  x0       Window-relative X coordinate of line's first point
 * @param[in]  y0       Window-relative Y coordinate of line's first point
 * @param[in]  x1       Window-relative X coordinate of line's second point
 * @param[in]  y1       Window-relative Y coordinate of line's second point
 * @param[in]  color    Color of the line. See tfb_make_color().
 */
void tfb_draw_line(int x0, int y0, int x1, int y1, u32 color);

/**
 * Draw an empty rectangle on-screen
 *
 * @param[in]  x        Window-relative X coordinate of rect's top-left corner
 * @param[in]  y        Window-relative Y coordinate of rect's top-left corner
 * @param[in]  w        Width of the rectangle
 * @param[in]  h        Height of the rectangle
 * @param[in]  color    Color of the rectangle
 */
void tfb_draw_rect(int x, int y, int w, int h, u32 color);

/**
 * Draw filled rectangle on-screen
 *
 * @param[in]  x        Window-relative X coordinate of rect's top-left corner
 * @param[in]  y        Window-relative Y coordinate of rect's top-left corner
 * @param[in]  w        Width of the rectangle
 * @param[in]  h        Height of the rectangle
 * @param[in]  color    Color of the rectangle
 */
void tfb_fill_rect(int x, int y, int w, int h, u32 color);

/**
 * Draw a single character on-screen at (x, y)
 *
 * @param[in]  x        Window-relative X coordinate of character's position
 * @param[in]  y        Window-relative Y coordinate of character's position
 * @param[in]  fg       Foreground text color
 * @param[in]  bg       Background text color
 * @param[in]  c        The character to draw on-screen
 */
void tfb_draw_char(int x, int y, u32 fg, u32 bg, u8 c);

/**
 * Draw a NUL-terminated string on-screen at (x, y)
 *
 * @param[in]  x        Window-relative X coordinate of text's position
 * @param[in]  y        Window-relative Y coordinate of text's position
 * @param[in]  fg       Foreground text color
 * @param[in]  bg       Background text color
 * @param[in]  s        A char pointer to the string
 */
void tfb_draw_string(int x, int y, u32 fg, u32 bg, const char *s);

/**
 * Draw a NUL-terminated string on-screen having its X-center at 'cx'
 *
 * @param[in]  cx       Window-relative X coordinate of text's X center
 * @param[in]  y        Window-relative Y coordinate of text's Y position
 * @param[in]  fg       Foreground text color
 * @param[in]  bg       Background text color
 * @param[in]  s        A char pointer to the string
 *
 * This function draws the given string by centering it horizonally at 'cx',
 * while 'y' coordinate is used as-it-is. In other words, 'y' is the distance
 * in pixels from the top of the current window to the top of the text.
 *
   \verbatim

       (Window -> the whole screen by default)
   #=======================================================#-------+
   #                                                       #       |
   #                                                       #       | y
   #     (x0, y)           (cx, y)                         #       |
   #        +-----------------+-----------------+ ---------#-------+
   #        | This is some text string centered |          #
   #        +-----------------+-----------------+          #
   #                          |                            #
   #                          |                            #
   #=======================================================#
   |                          |
   +--------------------------+
                cx

   x0 is calculated as:
         cx - curr_font_width * string_length / 2

   \endverbatim
 */
void tfb_draw_xcenter_string(int cx, int y, u32 fg, u32 bg, const char *s);

/**
 * Draw a single character on-screen at (x, y) scaled by (xscale, yscale)
 *
 * @param[in]  x        Window-relative X coordinate of character's position
 * @param[in]  y        Window-relative Y coordinate of character's position
 * @param[in]  fg       Foreground text color
 * @param[in]  bg       Background text color
 * @param[in]  xscale   Horizontal scale
 * @param[in]  yscale   Vertical scale
 * @param[in]  c        The character to draw on-screen

 * Like tfb_draw_char() but the current font is graphically scaled xscale times
 * horizontally and yscale times vertically. This useful when a font much bigger
 * than any available is needed. Also, it might be useful to create a simple
 * special effect by stretching a font only in one dimention (e.g. xscale=2,
 * yscale=1).
 *
 * \note    Because of the scaling, tfb_draw_char_scaled() as well as all the
 *          other tfb_draw_*_scaled() functions, is slower than their non-scaled
 *          versions. Avoid using xscale=1 and yscale=1 (for performance
 *          reasons).
 */
void tfb_draw_char_scaled(int x, int y, u32 fg, u32 bg,
                          int xscale, int yscale, u8 c);

/**
 * Draw a NUL-terminated string on-screen at (x, y) scaled by (xscale, yscale)
 *
 * @param[in]  x        Window-relative X coordinate of text's position
 * @param[in]  y        Window-relative Y coordinate of text's position
 * @param[in]  fg       Foreground text color
 * @param[in]  bg       Background text color
 * @param[in]  xscale   Horizontal scale
 * @param[in]  yscale   Vertical scale
 * @param[in]  s        A char pointer to the string
 *
 * Like tfb_draw_string(), but scaled. @see tfb_draw_char_scaled().
 */
void tfb_draw_string_scaled(int x, int y, u32 fg, u32 bg,
                            int xscale, int yscale, const char *s);
/**
 * Draw a NUL-terminated string on-screen having its X-center at 'cx' (scaled)
 *
 * @param[in]  cx       Window-relative X coordinate of text's X center
 * @param[in]  y        Window-relative Y coordinate of text's Y position
 * @param[in]  fg       Foreground text color
 * @param[in]  bg       Background text color
 * @param[in]  xscale   Horizontal scale
 * @param[in]  yscale   Vertical scale
 * @param[in]  s        A char pointer to the string
 *
 * Like tfb_draw_xcenter_string(), but scaled. @see tfb_draw_char_scaled().
 */
void tfb_draw_xcenter_string_scaled(int cx, int y, u32 fg, u32 bg,
                                    int xscale, int yscale, const char *s);

/**
 * Set all the pixels of the screen to the supplied color
 *
 * @param[in]  color    The color. See tfb_make_color().
 */
void tfb_clear_screen(u32 color);

/**
 * Set all the pixels of the current window to the supplied color
 *
 * @param[in]  color    The color. See tfb_make_color().
 *
 * \note Unless tfb_set_window() has been called, the current window is by
 *       default large as the whole screen.
 */
void tfb_clear_win(u32 color);

/**
 * Get screen's width
 *
 * @return  the width of the screen in pixels
 */
inline u32 tfb_screen_width(void);

/**
 * Get screen's height
 *
 * @return  the height of the screen in pixels
 */
inline u32 tfb_screen_height(void);

/**
 * Get current window's width
 *
 * @return  the width of the current window
 * @see tfb_set_window()
 */
inline u32 tfb_win_width(void);

/**
 * Get current window's height
 *
 * @return  the height of the current window
 * @see tfb_set_window()
 */
inline u32 tfb_win_height(void);

/**
 * Flush a given region to the actual framebuffer
 *
 * @param[in]  x        Window-relative X coordinate of region's position
 * @param[in]  y        Window-relative Y coordinate of region's position
 * @param[in]  w        Width of the region (in pixels)
 * @param[in]  h        Height of the region (in pixels)
 *
 * In case tfb_acquire_fb() has been called with #TFB_FL_USE_DOUBLE_BUFFER,
 * this function copies the pixels in the specified region to actual
 * framebuffer. By default double buffering is not used and this function has no
 * effect.
 */
void tfb_flush_rect(u32 x, u32 y, u32 w, u32 h);

/**
 * Flush the current window to the actual framebuffer
 *
 * A shortcut for tfb_flush_rect(0, 0, tfb_win_width(), tfb_win_height()).
 *
 * @see tfb_flush_rect
 * @see tfb_set_window
 */
void tfb_flush_window(void);

#include "tfb_inline_funcs.h" // internal header

/* undef the the convenience types defined above */
#undef u8
#undef u32
