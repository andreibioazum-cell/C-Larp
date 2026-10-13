#include "engine/runtime.h"
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#ifdef STB_IMAGE_STATIC
#undef STB_IMAGE_STATIC
#endif
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DSFont DSFont;
typedef struct Texture Texture;
typedef struct {
    uint32_t codepoint;
    float advance;
    float bearing_x;
    float bearing_top;
    int width;
    int height;
    float u0, v0, u1, v1;
} DSFontGlyph;
DSFont *ds_font_create(const uint8_t *data, size_t size, int ph);
void ds_font_destroy(DSFont *font);
const DSFontGlyph *ds_font_glyph(const DSFont *font, uint32_t cp);
int ds_font_aw(const DSFont *font);
int ds_font_ah(const DSFont *font);
const uint8_t *ds_font_alpha(const DSFont *font);
float ds_font_lineh(const DSFont *font);
float ds_font_ascent(const DSFont *font);

typedef struct {
    long long image;
    long long memory;
    long long view;
    long long desc;
    int uploaded;
} TexGpu;

struct Texture {
    Texture *next;
    char *name;
    int w, h, opaque;

    int upload_tries;
    uint32_t *pixels;
    /* Pre-grayscale copy of pixels while the black-and-white filter is on, so
     * the original colors can be restored when it switches off. */
    uint32_t *gray_backup;
    TexGpu gpu;
};

typedef enum {
    DS_CMD_RECT,
    DS_CMD_ROUND,
    DS_CMD_CIRCLE,
    DS_CMD_RING,
    DS_CMD_LINE,
    DS_CMD_TEX,
    DS_CMD_TEXT,
    DS_CMD_TEX_TINT,
    DS_CMD_RECT_ROT,
    DS_CMD_TEX_ROUND,
    DS_CMD_TEX_FADE
} DSCommand;
typedef struct {
    DSCommand t;
    union {
        struct {
            float x, y, w, h;
            uint32_t c;
        } rc;
        struct {
            float x, y, w, h, r;
            uint32_t c;
        } rr;
        struct {
            float x, y, r;
            uint32_t c;
        } ci;
        struct {
            float x, y, r, th;
            uint32_t c;
        } rg;
        struct {
            float x1, y1, x2, y2, th;
            uint32_t c;
        } ln;
        struct {
            float x, y, a, sc;
            Texture *tx;
        } tx;
        struct {
            float x, y, a, sc;
            Texture *tx;
            uint32_t c;
        } tx2;
        struct {
            float x, y, sc, r;
            Texture *tx;
        } txr;

        struct {
            float x, y, w, h, ang;
            uint32_t c;
        } rot;
        struct {
            char *s;
            float x, y, sc;
            uint32_t c;
        } tt;
    } v;
} DSCmd;

/* Black-and-white filter and screen shake state shared by the command color
 * packer, the geometry layer and the frame loop (one translation unit). */
static int gfx_gray_on;
static int gfx_grade_on;
static float gfx_shake_dx, gfx_shake_dy;
static float gfx_shake_amp, gfx_shake_left, gfx_shake_dur;

/* Настоящая цветокоррекция, а не цветная плёнка поверх кадра: тени уходят в
 * холодный оттенок, света - в тёплый, сверху немного контраста и
 * насыщенности. Считается для каждого цвета, который идёт в рендер. */
static void grade_channels(uint32_t *pr, uint32_t *pg, uint32_t *pb) {
    double r = *pr / 255.0, g = *pg / 255.0, b = *pb / 255.0;
    double y = 0.299 * r + 0.587 * g + 0.114 * b;
    double sh = (1 - y) * (1 - y);
    double hi = y * y;
    r = r - 0.014 * sh + 0.080 * hi;
    g = g + 0.016 * sh + 0.024 * hi;
    b = b + 0.070 * sh - 0.038 * hi;
    r = (r - 0.5) * 1.06 + 0.5;
    g = (g - 0.5) * 1.06 + 0.5;
    b = (b - 0.5) * 1.06 + 0.5;
    double y2 = 0.299 * r + 0.587 * g + 0.114 * b;
    r = y2 + (r - y2) * 1.16;
    g = y2 + (g - y2) * 1.16;
    b = y2 + (b - y2) * 1.16;
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    if (r > 1) r = 1;
    if (g > 1) g = 1;
    if (b > 1) b = 1;
    *pr = (uint32_t)(r * 255.0 + 0.5);
    *pg = (uint32_t)(g * 255.0 + 0.5);
    *pb = (uint32_t)(b * 255.0 + 0.5);
}

static uint32_t pack_c(uint32_t c) {
    uint32_t a = (c >> 24) & 0xff, r = (c >> 16) & 0xff, g = (c >> 8) & 0xff, b = c & 0xff;
    if (!a)
        a = 255;
    if (gfx_gray_on) {
        /* Classic luma weights; keeps the same perceived brightness. */
        uint32_t y = (299 * r + 587 * g + 114 * b) / 1000;
        r = g = b = y;
    } else if (gfx_grade_on) {
        grade_channels(&r, &g, &b);
    }
    return r | (g << 8) | (b << 16) | (a << 24);
}
static char *strdup_safe(const char *s) {
    if (!s) {
        char *o = (char *)malloc(1);
        if (o)
            o[0] = '\0';
        return o;
    }
    size_t n = strlen(s) + 1;
    char *o = (char *)malloc(n);
    if (o)
        memcpy(o, s, n);
    return o;
}
static int utf8_dec(const char **c) {
    const uint8_t *p = (const uint8_t *)*c;
    int r;
    if (!p || !*p)
        return -1;
    if (*p < 0x80)
        r = *p++;
    else if ((*p & 0xe0) == 0xc0 && (p[1] & 0xc0) == 0x80) {
        r = ((*p & 0x1f) << 6) | (p[1] & 0x3f);
        p += 2;
    } else if ((*p & 0xf0) == 0xe0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80) {
        r = ((*p & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
        p += 3;
    } else if ((*p & 0xf8) == 0xf0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80 && (p[3] & 0xc0) == 0x80) {
        r = ((*p & 7) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
        p += 4;
    } else
        r = *p++;
    *c = (const char *)p;
    return r;
}

static Buffer *cur_buf;
static DSCmd *cmds;
static size_t cmd_n, cmd_cap;
static int frame_open;
static AAssetManager *amgr;
static Texture *textures;
static DSFont *font;
static int font_tried;
