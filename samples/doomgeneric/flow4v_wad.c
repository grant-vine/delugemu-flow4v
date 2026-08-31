/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "doomtype.h"
#include "flow4v_platform.h"
#include "w_file.h"
#include "z_zone.h"

#ifndef FLOW4V_DOOM_IWAD
#define FLOW4V_DOOM_IWAD "freedoom1.wad"
#endif

#ifndef FLOW4V_DOOM_IWAD_SIZE
#error "FLOW4V_DOOM_IWAD_SIZE must match the preloaded SDRAM WAD"
#endif

typedef struct {
    wad_file_t wad;
} flow4v_wad_file_t;

extern wad_file_class_t stdc_wad_file;

static wad_file_t *flow4v_wad_open(char *path)
{
    flow4v_wad_file_t *result;
    const byte *mapped;
    uint32_t size;

    if (!flow4v_platform_wad_open(path, &size)) {
        return NULL;
    }
    mapped = flow4v_platform_wad_map();

    result = Z_Malloc(sizeof(*result), PU_STATIC, NULL);
    result->wad.file_class = &stdc_wad_file;
    result->wad.mapped = (byte *)mapped;
    result->wad.length = size;
    return &result->wad;
}

static void flow4v_wad_close(wad_file_t *wad)
{
    flow4v_platform_wad_close();
    Z_Free(wad);
}

static size_t flow4v_wad_read(wad_file_t *wad, unsigned int offset,
                             void *buffer, size_t buffer_len)
{
    size_t available;

    if (offset >= wad->length) {
        return 0;
    }
    available = wad->length - offset;
    if (buffer_len > available) {
        buffer_len = available;
    }
    return flow4v_platform_wad_read(offset, buffer, buffer_len);
}

wad_file_class_t stdc_wad_file = {
    flow4v_wad_open,
    flow4v_wad_close,
    flow4v_wad_read,
};
