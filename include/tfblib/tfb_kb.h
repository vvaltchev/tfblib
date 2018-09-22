/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include <stdint.h>

#define TFB_KEY_ENTER   ((uint64_t)10)
#define TFB_KEY_UP      (*(uint64_t*)("\033[A\0\0\0\0\0"))
#define TFB_KEY_DOWN    (*(uint64_t*)("\033[B\0\0\0\0\0"))
#define TFB_KEY_RIGHT   (*(uint64_t*)("\033[C\0\0\0\0\0"))
#define TFB_KEY_LEFT    (*(uint64_t*)("\033[D\0\0\0\0\0"))
#define TFB_KEY_DELETE  (*(uint64_t*)("\033[\x7f\0\0\0\0\0"))
#define TFB_KEY_HOME    (*(uint64_t*)("\033[H\0\0\0\0\0"))
#define TFB_KEY_END     (*(uint64_t*)("\033[F\0\0\0\0\0"))

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

extern uint64_t *tfb_fn_key_sequences;
int tfb_get_fn_key_num(uint64_t k);
