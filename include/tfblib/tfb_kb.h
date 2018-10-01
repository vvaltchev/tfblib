/* SPDX-License-Identifier: BSD-2-Clause */

#ifndef _TFBLIB_H_
#  error Never include this header directly. Include <tfblib/tfblib.h>.
#endif

#define TFB_KEY_ENTER   ((tfb_key_t)10)
#define TFB_KEY_UP      (*(tfb_key_t*)("\033[A\0\0\0\0\0"))
#define TFB_KEY_DOWN    (*(tfb_key_t*)("\033[B\0\0\0\0\0"))
#define TFB_KEY_RIGHT   (*(tfb_key_t*)("\033[C\0\0\0\0\0"))
#define TFB_KEY_LEFT    (*(tfb_key_t*)("\033[D\0\0\0\0\0"))
#define TFB_KEY_DELETE  (*(tfb_key_t*)("\033[\x7f\0\0\0\0\0"))
#define TFB_KEY_HOME    (*(tfb_key_t*)("\033[H\0\0\0\0\0"))
#define TFB_KEY_END     (*(tfb_key_t*)("\033[F\0\0\0\0\0"))

#define TFB_KEY_F1      (tfb_fn_key_sequences[0])
#define TFB_KEY_F2      (tfb_fn_key_sequences[1])
#define TFB_KEY_F3      (tfb_fn_key_sequences[2])
#define TFB_KEY_F4      (tfb_fn_key_sequences[3])
#define TFB_KEY_F5      (tfb_fn_key_sequences[4])
#define TFB_KEY_F6      (tfb_fn_key_sequences[5])
#define TFB_KEY_F7      (tfb_fn_key_sequences[6])
#define TFB_KEY_F8      (tfb_fn_key_sequences[7])
#define TFB_KEY_F9      (tfb_fn_key_sequences[8])
#define TFB_KEY_F10     (tfb_fn_key_sequences[9])
#define TFB_KEY_F11     (tfb_fn_key_sequences[10])
#define TFB_KEY_F12     (tfb_fn_key_sequences[11])

extern tfb_key_t *tfb_fn_key_sequences;
