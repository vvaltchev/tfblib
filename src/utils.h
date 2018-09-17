/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

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