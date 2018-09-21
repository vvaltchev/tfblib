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
#define TFB_KEY_F1      (*(uint64_t*)("\033OP\0\0\0\0\0"))
#define TFB_KEY_F2      (*(uint64_t*)("\033OQ\0\0\0\0\0"))
#define TFB_KEY_F3      (*(uint64_t*)("\033OR\0\0\0\0\0"))
#define TFB_KEY_F4      (*(uint64_t*)("\033OS\0\0\0\0\0"))
