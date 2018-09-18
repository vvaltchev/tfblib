/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#define INT_ABS(_x) ((_x) > 0 ? (_x) : (-(_x)))
#define INT_MIN(x, y) ((x) <= (y) ? (x) : (y))
#define INT_MAX(x, y) ((x) > (y) ? (x) : (y))

/* Here in our internal header, it's save to typedef our convenience types */
typedef uint8_t u8;
typedef uint32_t u32;


/*
 * Set 'n' 32-bit elems pointed by 's' to 'val'.
 */
static inline void *memset32(void *s, u32 val, size_t n)
{

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

   unsigned unused;

   __asm__ volatile ("rep stosl"
                     : "=D" (unused), "=a" (val), "=c" (n)
                     :  "D" (s), "a" (val), "c" (n)
                     : "cc", "memory");
#else


   for (size_t i = 0; i < n; i++)
      ((volatile u32 *)s)[i] = val;

#endif

   return s;
}
