/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include <stdint.h>

extern char *__fb_buffer;
extern size_t __fb_pitch_div4;

static inline void set_pixel(uint32_t x, uint32_t y, uint32_t color)
{
   ((volatile uint32_t *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void clear_screen(uint32_t color);
bool fb_acquire(void);
void fb_release(void);


