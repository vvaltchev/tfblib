/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file tfb_kb.h
 * @brief Tfblib's keyboard input related functions and definitions
 */

#pragma once
#include <stdint.h>

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
int tfb_set_kb_raw_mode(uint32_t flags);

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


#define TFB_KEY_ENTER   ((tfb_key_t)10)
#define TFB_KEY_UP      (*(tfb_key_t*)("\033[A\0\0\0\0\0"))
#define TFB_KEY_DOWN    (*(tfb_key_t*)("\033[B\0\0\0\0\0"))
#define TFB_KEY_RIGHT   (*(tfb_key_t*)("\033[C\0\0\0\0\0"))
#define TFB_KEY_LEFT    (*(tfb_key_t*)("\033[D\0\0\0\0\0"))
#define TFB_KEY_DELETE  (*(tfb_key_t*)("\033[\x7f\0\0\0\0\0"))
#define TFB_KEY_HOME    (*(tfb_key_t*)("\033[H\0\0\0\0\0"))
#define TFB_KEY_END     (*(tfb_key_t*)("\033[F\0\0\0\0\0"))

#define TFB_KEY_F1      (tfb_int_fn_key_sequences[0])
#define TFB_KEY_F2      (tfb_int_fn_key_sequences[1])
#define TFB_KEY_F3      (tfb_int_fn_key_sequences[2])
#define TFB_KEY_F4      (tfb_int_fn_key_sequences[3])
#define TFB_KEY_F5      (tfb_int_fn_key_sequences[4])
#define TFB_KEY_F6      (tfb_int_fn_key_sequences[5])
#define TFB_KEY_F7      (tfb_int_fn_key_sequences[6])
#define TFB_KEY_F8      (tfb_int_fn_key_sequences[7])
#define TFB_KEY_F9      (tfb_int_fn_key_sequences[8])
#define TFB_KEY_F10     (tfb_int_fn_key_sequences[9])
#define TFB_KEY_F11     (tfb_int_fn_key_sequences[10])
#define TFB_KEY_F12     (tfb_int_fn_key_sequences[11])

extern tfb_key_t *tfb_int_fn_key_sequences;