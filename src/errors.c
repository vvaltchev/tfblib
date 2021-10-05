/* SPDX-License-Identifier: BSD-2-Clause */

#include <tfblib/tfb_errors.h>
#include "utils.h"

static const char *error_msgs[] =
{
   /*  0 */    "Success",
   /*  1 */    "Unable to open the framebuffer device",
   /*  2 */    "Unable to get screen info for the framebuffer device",
   /*  3 */    "Unable to open the TTY device",
   /*  4 */    "Unable to set TTY in graphic mode",
   /*  5 */    "Unable to mmap the framebuffer",
   /*  6 */    "Invalid window position/size",
   /*  7 */    "Unsupported video mode",
   /*  8 */    "Invalid font_id",
   /*  9 */    "Unable to open/read/load the font file",
   /* 10 */    "Out of memory",
   /* 11 */    "The font_id is does not belog to a dynamically loaded font",
   /* 12 */    "The keyboard input is not in the expected mode",
   /* 13 */    "Unable to get a keyboard input paramater with ioctl()",
   /* 14 */    "Unable to set a keyboard input paramater with ioctl()",
   /* 15 */    "Unable to find a font matching the criteria",
   /* 16 */    "Unable to flush the framebuffer with ioctl()",
};

const char *tfb_strerror(int error_code)
{
   if (error_code < 0 || error_code >= ARRAY_SIZE(error_msgs))
      return "(unknown error)";

   return error_msgs[error_code];
}
