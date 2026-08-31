/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * Public excerpt of the DoomGeneric-to-FLOW platform adapter.
 *
 * The private engineering submission supplies the engine headers, lifecycle
 * integration, hardware startup code, and complete build. This excerpt keeps
 * only the portable frame, timing, and edge-triggered input seam.
 */

#include <stdint.h>

#include "flow4v_platform.h"

extern uint8_t *DG_ScreenBuffer;

static uint32_t previous_keys;
static uint32_t pending_keys;
static uint32_t pending_down;
static uint16_t palette_rgb565[256];

static unsigned char key_code_for_bit(uint32_t bit)
{
    switch (bit) {
    case FLOW4V_KEY_UP:
        return 0xad;
    case FLOW4V_KEY_DOWN:
        return 0xaf;
    case FLOW4V_KEY_LEFT:
        return 0xac;
    case FLOW4V_KEY_RIGHT:
        return 0xae;
    case FLOW4V_KEY_FIRE:
        return 0xa3;
    case FLOW4V_KEY_USE:
        return ' ';
    case FLOW4V_KEY_STRAFE:
        return 0xa0;
    case FLOW4V_KEY_RUN:
        return 0xa2;
    case FLOW4V_KEY_ESC:
        return 27;
    case FLOW4V_KEY_ENTER:
        return 13;
    case FLOW4V_KEY_PAUSE:
        return 0xff;
    default:
        return 0;
    }
}

static void poll_keys(void)
{
    uint32_t keys = flow4v_platform_keys();
    uint32_t changed = keys ^ previous_keys;

    pending_keys |= changed;
    pending_down |= changed & keys;
    previous_keys = keys;
}

void DG_Init(void)
{
    flow4v_platform_init();
}

void DG_DrawFrame(void)
{
    flow4v_platform_present_indexed(DG_ScreenBuffer, palette_rgb565);
}

void DG_SleepMs(uint32_t ms)
{
    uint32_t start = flow4v_platform_ticks_ms();

    while (flow4v_platform_ticks_ms() - start < ms) {
        poll_keys();
    }
}

uint32_t DG_GetTicksMs(void)
{
    return flow4v_platform_ticks_ms();
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    uint32_t bit;

    poll_keys();
    bit = pending_keys & (0u - pending_keys);
    if (!bit) {
        return 0;
    }

    pending_keys &= ~bit;
    *pressed = (pending_down & bit) != 0;
    pending_down &= ~bit;
    *key = key_code_for_bit(bit);
    return *key != 0;
}

void DG_SetWindowTitle(const char *title)
{
    (void)title;
}
