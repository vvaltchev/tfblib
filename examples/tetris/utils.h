/* SPDX-License-Identifier: BSD-2-Clause */

#define MIN(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x <= _y ? _x : _y; })

#define MAX(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x > _y ? _x : _y; })

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

double int_pow(double b, int p);