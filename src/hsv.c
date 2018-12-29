/*
 * This file contains a heavily modified version of the hsv2rgb() function
 * written by Martin Mitas in 2013.
 *
 * Original source code:
 *    https://gist.github.com/mity/6034000
 *
 * For the original license, see the NOTICE file.
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

