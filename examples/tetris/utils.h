/* SPDX-License-Identifier: BSD-2-Clause */

#define MIN(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x <= _y ? _x : _y; })

#define MAX(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x > _y ? _x : _y; })

extern uint32_t red, green, blue, white, black, yellow, gray;

void init_colors(void);
