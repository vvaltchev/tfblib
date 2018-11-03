/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * This file contains a heavily modified version of the hsv2rgb() function
 * written by Martin Mitas in 2013.
 *
 * Original source code:
 *    https://gist.github.com/mity/6034000
 *
 * The original license follows:
 *
 * RGB <--> HSV conversion in integer arithmetics, to be used on Windows.
 * Copyright (c) 2013 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <tfblib/tfblib.h>
#include "utils.h"

#define DEG_60 (60 * TFB_HUE_DEGREE)

u32 tfb_make_color_hsv(u32 h, u8 s, u8 v)
{
   u32 p, x;
   u32 r = 0, g = 0, b = 0;
   int sv = -s * v;

   u32 region = h / DEG_60;
   u32 r1 = region;

   p = (256 * v - s * v) / 256;

   if (!(region & 1)) {
      r1++;
      sv = -sv;
   }

   x = (256 * DEG_60 * v + h * sv - DEG_60 * sv * r1) / (256 * DEG_60);

   switch(region) {
      case 0: r = v; g = x; b = p; break;
      case 1: r = x; g = v; b = p; break;
      case 2: r = p; g = v; b = x; break;
      case 3: r = p; g = x; b = v; break;
      case 4: r = x; g = p; b = v; break;
      case 5: r = v; g = p; b = x; break;
   }

   return tfb_make_color(r, g, b);
}

