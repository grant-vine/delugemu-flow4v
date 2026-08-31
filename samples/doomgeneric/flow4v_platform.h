/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef FLOW4V_PLATFORM_H
#define FLOW4V_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FLOW4V_PLATFORM_WIDTH 320u
#define FLOW4V_PLATFORM_HEIGHT 200u

#define FLOW4V_KEY_UP     (1u << 0)
#define FLOW4V_KEY_DOWN   (1u << 1)
#define FLOW4V_KEY_LEFT   (1u << 2)
#define FLOW4V_KEY_RIGHT  (1u << 3)
#define FLOW4V_KEY_FIRE   (1u << 4)
#define FLOW4V_KEY_USE    (1u << 5)
#define FLOW4V_KEY_STRAFE (1u << 6)
#define FLOW4V_KEY_RUN    (1u << 7)
#define FLOW4V_KEY_ESC    (1u << 8)
#define FLOW4V_KEY_ENTER  (1u << 9)
#define FLOW4V_KEY_PAUSE  (1u << 10)

void flow4v_platform_init(void);
void flow4v_platform_present_indexed(const uint8_t *pixels,
                                     const uint16_t palette[256]);
uint32_t flow4v_platform_ticks_ms(void);
uint32_t flow4v_platform_keys(void);

bool flow4v_platform_wad_available(const char *path);
bool flow4v_platform_wad_open(const char *path, uint32_t *size);
const uint8_t *flow4v_platform_wad_map(void);
size_t flow4v_platform_wad_read(uint32_t offset, void *buffer, size_t length);
void flow4v_platform_wad_close(void);

#endif
