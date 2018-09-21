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
#define TFB_KEY_F1      (*(uint64_t*)("\033[[A\0\0\0\0"))
#define TFB_KEY_F2      (*(uint64_t*)("\033[[B\0\0\0\0"))
#define TFB_KEY_F3      (*(uint64_t*)("\033[[C\0\0\0\0"))
#define TFB_KEY_F4      (*(uint64_t*)("\033[[D\0\0\0\0"))
#define TFB_KEY_F5      (*(uint64_t*)("\033[[E\0\0\0\0"))
#define TFB_KEY_F6      (*(uint64_t*)("\033[17~\0\0\0"))
#define TFB_KEY_F7      (*(uint64_t*)("\033[18~\0\0\0"))
#define TFB_KEY_F8      (*(uint64_t*)("\033[19~\0\0\0"))
#define TFB_KEY_F9      (*(uint64_t*)("\033[20~\0\0\0"))
#define TFB_KEY_F10     (*(uint64_t*)("\033[21~\0\0\0"))
#define TFB_KEY_F11     (*(uint64_t*)("\033[23~\0\0\0"))
#define TFB_KEY_F12     (*(uint64_t*)("\033[24~\0\0\0"))



