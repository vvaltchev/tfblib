/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file tfb_errors.h
 * @brief All the error codes returned by Tfblib
 */

#pragma once

/**
 * \addtogroup ErrorCodes Error Codes
 * @{
 */

/// The call completed successfully without any errors.
#define TFB_SUCCESS                       0

/// open() failed on the framebuffer device
#define TFB_ERR_OPEN_FB                   1

/// ioctl() failed on the framebuffer device file descriptor
#define TFB_ERR_IOCTL_FB                  2

/// open() failed on the TTY device
#define TFB_ERR_OPEN_TTY                  3

/// ioctl() on the TTY failed while trying to set TTY in graphic mode
#define TFB_ERR_TTY_GRAPHIC_MODE          4

/// mmap() on the framebuffer file description failed
#define TFB_ERR_MMAP_FB                   5

/// Invalid window position/size
#define TFB_ERR_INVALID_WINDOW            6

/**
 * Unsupported video mode
 *
 * \note Currently the library supports only 32-bit color modes.
 */
#define TFB_ERR_UNSUPPORTED_VIDEO_MODE    7

/// The supplied font_id is invalid
#define TFB_ERR_INVALID_FONT_ID           8

/// Unable to open/read/load the supplied font file
#define TFB_ERR_READ_FONT_FILE_FAILED     9

/// Out of memory (malloc() returned 0)
#define TFB_ERR_OUT_OF_MEMORY            10

/// The supplied font_id is does not belog to a dynamically loaded font
#define TFB_ERR_NOT_A_DYN_LOADED_FONT    11

/// The keyboard input is not in the expected mode (e.g. already in raw mode)
#define TFB_ERR_KB_WRONG_MODE            12

/// Unable to get a keyboard input paramater with ioctl()
#define TFB_ERR_KB_MODE_GET_FAILED       13

/// Unable to set a keyboard input paramater with ioctl()
#define TFB_ERR_KB_MODE_SET_FAILED       14

/// Unable to find a font matching the criteria
#define TFB_ERR_FONT_NOT_FOUND           15

/// Unable to flush the framebuffer with ioctl()
#define TFB_ERR_FB_FLUSH_IOCTL_FAILED 16

/**
 * Returns a human-readable error message.
 *
 * @param[in] error_code   The error code returned by one of the library funcs
 *
 * @return  A string representation of the error code
 */
const char *tfb_strerror(int error_code);

/** @} */
