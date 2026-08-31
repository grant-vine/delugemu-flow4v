/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "hw/display/flow4v_lcd.h"

#define FT811_RAM_DL     0x300000u
#define FT811_RAM_REG    0x302000u
#define FT811_REG_DLSWAP 0x302054u
#define FT811_CHIP_ID    0x000c0000u
#define FT811_CHIP_VALUE 0x08110100u

#define FT811_FORMAT_L4     2u
#define FT811_FORMAT_L8     3u
#define FT811_FORMAT_RGB565 7u
#define FT811_PRIM_BITMAPS  1u

typedef struct Flow4vFt811Bitmap {
    uint32_t source;
    uint16_t stride;
    uint16_t layout_height;
    uint16_t width;
    uint16_t height;
    uint8_t format;
} Flow4vFt811Bitmap;

typedef struct Flow4vFt811Context {
    uint16_t scissor_x;
    uint16_t scissor_y;
    uint16_t scissor_w;
    uint16_t scissor_h;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    uint8_t color_a;
    uint8_t handle;
    uint8_t primitive;
} Flow4vFt811Context;

typedef struct Flow4vFt811Render {
    Flow4vFt811Context context;
    Flow4vFt811Context stack[4];
    Flow4vFt811Bitmap bitmap[32];
    uint32_t clear_color;
    uint32_t unknown;
    uint32_t invalid_source;
    uint8_t depth;
} Flow4vFt811Render;

static uint32_t flow4v_ft811_load_le32(const uint8_t *data)
{
    return data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static void flow4v_ft811_context_reset(Flow4vFt811Context *context)
{
    memset(context, 0, sizeof(*context));
    context->scissor_w = FLOW4V_LCD_WIDTH;
    context->scissor_h = FLOW4V_LCD_HEIGHT;
    context->color_r = 255;
    context->color_g = 255;
    context->color_b = 255;
    context->color_a = 255;
}

static bool flow4v_ft811_inside_scissor(const Flow4vFt811Context *context,
                                        unsigned x, unsigned y)
{
    return x >= context->scissor_x && y >= context->scissor_y &&
           x < context->scissor_x + context->scissor_w &&
           y < context->scissor_y + context->scissor_h &&
           x < FLOW4V_LCD_WIDTH && y < FLOW4V_LCD_HEIGHT;
}

static void flow4v_ft811_store_rgb565(Flow4vLcdState *s, unsigned x,
                                      unsigned y, uint8_t r, uint8_t g,
                                      uint8_t b, uint8_t alpha)
{
    size_t off = ((size_t)y * FLOW4V_LCD_WIDTH + x) * 2;
    uint16_t dst = s->fb[off] | ((uint16_t)s->fb[off + 1] << 8);
    uint8_t dr = ((dst >> 11) & 0x1f) * 255 / 31;
    uint8_t dg = ((dst >> 5) & 0x3f) * 255 / 63;
    uint8_t db = (dst & 0x1f) * 255 / 31;
    uint16_t result;

    r = ((uint16_t)r * alpha + (uint16_t)dr * (255 - alpha)) / 255;
    g = ((uint16_t)g * alpha + (uint16_t)dg * (255 - alpha)) / 255;
    b = ((uint16_t)b * alpha + (uint16_t)db * (255 - alpha)) / 255;
    result = ((uint16_t)(r >> 3) << 11) |
             ((uint16_t)(g >> 2) << 5) | (b >> 3);
    s->fb[off] = result;
    s->fb[off + 1] = result >> 8;
}

static bool flow4v_ft811_bitmap_pixel(Flow4vLcdState *s,
                                      const Flow4vFt811Bitmap *bitmap,
                                      unsigned cell, unsigned x, unsigned y,
                                      uint8_t *r, uint8_t *g, uint8_t *b,
                                      uint8_t *alpha)
{
    uint64_t base = bitmap->source +
                    (uint64_t)cell * bitmap->stride * bitmap->layout_height;
    uint64_t addr = base + (uint64_t)y * bitmap->stride;
    uint8_t value;

    switch (bitmap->format) {
    case FT811_FORMAT_L4:
        addr += x / 2;
        if (addr >= FLOW4V_FT811_RAM_G_SIZE) {
            return false;
        }
        value = s->ft811_ram_g[addr];
        value = x & 1 ? value & 0x0f : value >> 4;
        *alpha = value * 17;
        return true;
    case FT811_FORMAT_L8:
        addr += x;
        if (addr >= FLOW4V_FT811_RAM_G_SIZE) {
            return false;
        }
        *alpha = s->ft811_ram_g[addr];
        return true;
    case FT811_FORMAT_RGB565: {
        uint16_t pixel;

        addr += x * 2;
        if (addr + 1 >= FLOW4V_FT811_RAM_G_SIZE) {
            return false;
        }
        pixel = s->ft811_ram_g[addr] |
                ((uint16_t)s->ft811_ram_g[addr + 1] << 8);
        *r = ((pixel >> 11) & 0x1f) * 255 / 31;
        *g = ((pixel >> 5) & 0x3f) * 255 / 63;
        *b = (pixel & 0x1f) * 255 / 31;
        return true;
    }
    default:
        return false;
    }
}

static void flow4v_ft811_draw_bitmap(Flow4vLcdState *s,
                                     Flow4vFt811Render *render,
                                     unsigned x, unsigned y,
                                     unsigned handle, unsigned cell)
{
    const Flow4vFt811Context *context = &render->context;
    const Flow4vFt811Bitmap *bitmap = &render->bitmap[handle];

    for (unsigned by = 0; by < bitmap->height; by++) {
        for (unsigned bx = 0; bx < bitmap->width; bx++) {
            unsigned px = x + bx;
            unsigned py = y + by;
            uint8_t r = context->color_r;
            uint8_t g = context->color_g;
            uint8_t b = context->color_b;
            uint8_t alpha = 255;

            if (!flow4v_ft811_inside_scissor(context, px, py)) {
                continue;
            }
            if (!flow4v_ft811_bitmap_pixel(s, bitmap, cell, bx, by,
                                           &r, &g, &b, &alpha)) {
                render->invalid_source++;
                return;
            }
            alpha = (uint16_t)alpha * context->color_a / 255;
            r = (uint16_t)r * context->color_r / 255;
            g = (uint16_t)g * context->color_g / 255;
            b = (uint16_t)b * context->color_b / 255;
            flow4v_ft811_store_rgb565(s, px, py, r, g, b, alpha);
        }
    }
}

static void flow4v_ft811_clear(Flow4vLcdState *s, Flow4vFt811Render *render)
{
    uint8_t r = render->clear_color >> 16;
    uint8_t g = render->clear_color >> 8;
    uint8_t b = render->clear_color;
    const Flow4vFt811Context *context = &render->context;

    for (unsigned y = 0; y < FLOW4V_LCD_HEIGHT; y++) {
        for (unsigned x = 0; x < FLOW4V_LCD_WIDTH; x++) {
            if (flow4v_ft811_inside_scissor(context, x, y)) {
                flow4v_ft811_store_rgb565(s, x, y, r, g, b, 255);
            }
        }
    }
}

static bool flow4v_ft811_render(Flow4vLcdState *s, uint32_t *word_count,
                                uint32_t *unknown, uint32_t *invalid_source)
{
    Flow4vFt811Render render = { 0 };
    uint32_t pc = 0;
    uint32_t steps = 0;

    memset(s->fb, 0, sizeof(s->fb));
    flow4v_ft811_context_reset(&render.context);
    while (pc + 4 <= FLOW4V_FT811_RAM_DL_SIZE &&
           steps++ < FLOW4V_FT811_RAM_DL_SIZE / 4) {
        uint32_t word = flow4v_ft811_load_le32(&s->ft811_active_dl[pc]);
        uint8_t op = word >> 24;

        pc += 4;
        if (word == 0) {
            *word_count = steps;
            *unknown = render.unknown;
            *invalid_source = render.invalid_source;
            return true;
        }
        if ((word >> 30) == 2) {
            unsigned x = (word >> 21) & 0x1ff;
            unsigned y = (word >> 12) & 0x1ff;
            unsigned handle = (word >> 7) & 0x1f;
            unsigned cell = word & 0x7f;

            if (render.context.primitive == FT811_PRIM_BITMAPS) {
                flow4v_ft811_draw_bitmap(s, &render, x, y, handle, cell);
            }
            continue;
        }
        switch (op) {
        case 0x01:
            render.bitmap[render.context.handle].source = word & 0x3fffff;
            break;
        case 0x02:
            render.clear_color = word & 0xffffff;
            break;
        case 0x04:
            render.context.color_r = word >> 16;
            render.context.color_g = word >> 8;
            render.context.color_b = word;
            break;
        case 0x05:
            render.context.handle = word & 0x1f;
            break;
        case 0x07: {
            Flow4vFt811Bitmap *bitmap =
                &render.bitmap[render.context.handle];

            bitmap->format = (word >> 19) & 0x1f;
            bitmap->stride = (word >> 9) & 0x3ff;
            bitmap->layout_height = word & 0x1ff;
            break;
        }
        case 0x08: {
            Flow4vFt811Bitmap *bitmap =
                &render.bitmap[render.context.handle];

            bitmap->width = (word >> 9) & 0x1ff;
            bitmap->height = word & 0x1ff;
            break;
        }
        case 0x10:
            render.context.color_a = word & 0xff;
            break;
        case 0x1b:
            render.context.scissor_x = (word >> 11) & 0x7ff;
            render.context.scissor_y = word & 0x7ff;
            break;
        case 0x1c:
            render.context.scissor_w = (word >> 12) & 0xfff;
            render.context.scissor_h = word & 0xfff;
            break;
        case 0x1e:
            pc = (word & 0xffff) * 4;
            break;
        case 0x1f:
            render.context.primitive = word & 0xf;
            break;
        case 0x22:
            if (render.depth < ARRAY_SIZE(render.stack)) {
                render.stack[render.depth++] = render.context;
            }
            break;
        case 0x23:
            if (render.depth > 0) {
                render.context = render.stack[--render.depth];
            } else {
                flow4v_ft811_context_reset(&render.context);
            }
            break;
        case 0x26:
            if (word & BIT(2)) {
                flow4v_ft811_clear(s, &render);
            }
            break;
        case 0x28:
            render.bitmap[render.context.handle].stride |=
                ((word >> 2) & 3) << 10;
            render.bitmap[render.context.handle].layout_height |=
                (word & 3) << 9;
            break;
        case 0x29:
            render.bitmap[render.context.handle].width |=
                ((word >> 2) & 3) << 9;
            render.bitmap[render.context.handle].height |=
                (word & 3) << 9;
            break;
        case 0x03:
        case 0x06:
        case 0x0b:
        case 0x20:
        case 0x21:
        case 0x25:
        case 0x27:
        case 0x2d:
            break;
        default:
            render.unknown++;
            break;
        }
    }
    *word_count = steps;
    *unknown = render.unknown;
    *invalid_source = render.invalid_source;
    return false;
}

void flow4v_lcd_ft811_write(Flow4vLcdState *s, uint32_t addr, uint8_t value)
{
    if (addr < FLOW4V_FT811_RAM_G_SIZE) {
        s->ft811_ram_g[addr] = value;
        s->ft811_ram_g_writes++;
        if (value != 0) {
            s->ft811_ram_g_nonzero_writes++;
        }
    } else if (addr >= FT811_RAM_DL &&
               addr < FT811_RAM_DL + FLOW4V_FT811_RAM_DL_SIZE) {
        s->ft811_ram_dl[addr - FT811_RAM_DL] = value;
    } else if (addr >= FT811_RAM_REG &&
               addr < FT811_RAM_REG + FLOW4V_FT811_RAM_REG_SIZE) {
        s->ft811_ram_reg[addr - FT811_RAM_REG] = value;
    }
}

uint32_t flow4v_lcd_ft811_read32(Flow4vLcdState *s, uint32_t addr)
{
    uint32_t value = 0;

    if (addr == FT811_CHIP_ID) {
        return FT811_CHIP_VALUE;
    }
    for (unsigned i = 0; i < 4; i++) {
        uint8_t byte = 0;
        uint32_t current = addr + i;

        if (current < FLOW4V_FT811_RAM_G_SIZE) {
            byte = s->ft811_ram_g[current];
        } else if (current >= FT811_RAM_DL &&
                   current < FT811_RAM_DL + FLOW4V_FT811_RAM_DL_SIZE) {
            byte = s->ft811_ram_dl[current - FT811_RAM_DL];
        } else if (current >= FT811_RAM_REG &&
                   current < FT811_RAM_REG + FLOW4V_FT811_RAM_REG_SIZE) {
            byte = s->ft811_ram_reg[current - FT811_RAM_REG];
        }
        value = (value << 8) | byte;
    }
    return value;
}

void flow4v_lcd_ft811_end_write(Flow4vLcdState *s, uint32_t addr,
                                uint32_t len)
{
    uint32_t words = 0;
    uint32_t unknown = 0;
    uint32_t invalid_source = 0;
    uint32_t dlswap = FT811_REG_DLSWAP - FT811_RAM_REG;
    bool valid;

    if (addr > FT811_REG_DLSWAP || addr + len <= FT811_REG_DLSWAP ||
        s->ft811_ram_reg[dlswap] != 2) {
        return;
    }
    memcpy(s->ft811_active_dl, s->ft811_ram_dl,
           sizeof(s->ft811_active_dl));
    valid = flow4v_ft811_render(s, &words, &unknown, &invalid_source);
    s->ft811_ram_reg[dlswap] = 0;
    if (!valid) {
        qemu_log_mask(LOG_UNIMP,
                      "flow4v-ft811: reject malformed list words %" PRIu32
                      " unknown %" PRIu32 " invalid-source %" PRIu32 "\n",
                      words, unknown, invalid_source);
        return;
    }
    s->ft811_frame_count++;
    s->write_count++;
    s->dirty = true;
    qemu_log_mask(LOG_UNIMP,
                  "flow4v-ft811: swap frame %" PRIu64 " words %" PRIu32
                  " unknown %" PRIu32 " invalid-source %" PRIu32
                  " ram-g-writes %" PRIu64 " ram-g-nonzero %" PRIu64 "\n",
                  s->ft811_frame_count, words, unknown, invalid_source,
                  s->ft811_ram_g_writes, s->ft811_ram_g_nonzero_writes);
}
