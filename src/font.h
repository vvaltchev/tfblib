/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include "utils.h"

typedef struct {
   const char *filename;
   unsigned int data_size;
   unsigned char data[];
} font_file;

extern const font_file **tfb_font_file_list;

#define PSF2_FONT_MAGIC 0x864ab572

typedef struct {
    u32 magic;
    u32 version;          /* zero */
    u32 header_size;
    u32 flags;            /* 0 if there's no unicode table */
    u32 glyphs_count;
    u32 bytes_per_glyph;
    u32 height;          /* height in pixels */
    u32 width;           /* width in pixels */
} psf2_header;
