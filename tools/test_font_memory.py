#!/usr/bin/env python3
"""Offline ASan/UBSan regression checks for the native TrueType parser."""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FONT = ROOT / "assets/fonts/ComicRelief-Regular.ttf"
HARNESS = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine/runtime.h"
typedef struct {
    uint32_t codepoint;
    float advance, bearing_x, bearing_top;
    int width, height;
    float u0, v0, u1, v1;
} DSFontGlyph;
#include "engine/font/parse_bake.inc"
#include "engine/font/bake.inc"
#include "engine/font/public_api.inc"

static void put16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
static void put32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}
static uint8_t *load_file(const char *path, size_t *size) {
    FILE *in = fopen(path, "rb");
    assert(in);
    assert(fseek(in, 0, SEEK_END) == 0);
    long length = ftell(in);
    assert(length > 0 && fseek(in, 0, SEEK_SET) == 0);
    uint8_t *data = (uint8_t *)malloc((size_t)length);
    assert(data && fread(data, 1, (size_t)length, in) == (size_t)length);
    fclose(in);
    *size = (size_t)length;
    return data;
}
static int setup_font(uint8_t *data, size_t size, DSFont *meta, int *glyph) {
    memset(meta, 0, sizeof(*meta));
    meta->data = data;
    meta->size = size;
    if (!init_tables(meta)) return 0;
    *glyph = g_for(meta, ' ');
    return *glyph >= 0 && *glyph + 1 < meta->ng;
}
static int set_glyph_range(DSFont *meta, int glyph, uint32_t start, uint32_t end) {
    if (start > end) return 0;
    if (meta->loc_format == 0) {
        if ((start & 1u) || (end & 1u) || start / 2 > 65535u || end / 2 > 65535u) return 0;
        put16(meta->data + meta->loca + (size_t)glyph * 2, (uint16_t)(start / 2));
        put16(meta->data + meta->loca + (size_t)(glyph + 1) * 2, (uint16_t)(end / 2));
    } else {
        put32(meta->data + meta->loca + (size_t)glyph * 4, start);
        put32(meta->data + meta->loca + (size_t)(glyph + 1) * 4, end);
    }
    return 1;
}
static int outline_result(DSFont *meta, int glyph) {
    DSOutline outline;
    ol_init(&outline);
    int ok = read_outline(meta, glyph, 0, 1, 0, 0, 1, 0, 0, &outline);
    ol_free(&outline);
    return ok;
}
static void test_valid_font(const char *path) {
    size_t size;
    uint8_t *data = load_file(path, &size);
    DSFont *font = ds_font_create(data, size, 16);
    assert(font);
    const DSFontGlyph *glyph = ds_font_glyph(font, 'A');
    assert(glyph && glyph->width > 0 && glyph->height > 0);
    ds_font_destroy(font);
    free(data);
}
static void test_glyph_bounds(const char *path) {
    size_t size;
    uint8_t *data = load_file(path, &size);
    DSFont meta;
    int glyph;
    assert(setup_font(data, size, &meta, &glyph));
    assert(meta.loc_format == 0);
    uint32_t start = (uint32_t)(size - meta.glyf - 16);
    assert(set_glyph_range(&meta, glyph, start, start + 16));
    memset(data + meta.glyf + start, 0, 16);
    put16(data + meta.glyf + start, 1);
    put16(data + meta.glyf + start + 10, 0);
    put16(data + meta.glyf + start + 12, 1);
    data[meta.glyf + start + 14] = 0;
    data[meta.glyf + start + 15] = 8; /* RLE count is outside the glyf table. */
    assert(!outline_result(&meta, glyph));
    free(data);
}
static void test_truncated_repeat(const char *path) {
    size_t size;
    uint8_t *data = load_file(path, &size);
    DSFont meta;
    int glyph;
    assert(setup_font(data, size, &meta, &glyph));
    assert(meta.glyf_len >= 16);
    uint32_t start = meta.glyf_len - 16;
    assert(set_glyph_range(&meta, glyph, start, meta.glyf_len));
    memset(data + meta.glyf + start, 0, 16);
    put16(data + meta.glyf + start, 1);
    put16(data + meta.glyf + start + 10, 0);
    put16(data + meta.glyf + start + 12, 1);
    data[meta.glyf + start + 14] = 0;
    data[meta.glyf + start + 15] = 8; /* Repeat-count byte lies after this glyph. */
    assert(!outline_result(&meta, glyph));
    free(data);
}
static void test_bad_contour_ends(const char *path) {
    size_t size;
    uint8_t *data = load_file(path, &size);
    DSFont meta;
    int glyph;
    assert(setup_font(data, size, &meta, &glyph));
    uint32_t start = (meta.glyf_len - 20u) & ~1u;
    assert(set_glyph_range(&meta, glyph, start, start + 20));
    memset(data + meta.glyf + start, 0, 20);
    put16(data + meta.glyf + start, 2);
    put16(data + meta.glyf + start + 10, 65535); /* Endpoints must increase. */
    put16(data + meta.glyf + start + 12, 0);     /* Final endpoint says one point. */
    data[meta.glyf + start + 16] = 0x31;
    assert(!outline_result(&meta, glyph));
    free(data);
}
static void test_truncated_composite(const char *path) {
    size_t size;
    uint8_t *data = load_file(path, &size);
    DSFont meta;
    int glyph;
    assert(setup_font(data, size, &meta, &glyph));
    uint32_t start = (meta.glyf_len - 14u) & ~1u;
    assert(set_glyph_range(&meta, glyph, start, start + 14));
    memset(data + meta.glyf + start, 0, 14);
    put16(data + meta.glyf + start, 0xffff); /* Composite glyph, missing arguments. */
    put16(data + meta.glyf + start + 10, 0);
    put16(data + meta.glyf + start + 12, 0);
    assert(!outline_result(&meta, glyph));
    free(data);
}
static void test_tall_glyph(const char *path) {
    size_t size;
    uint8_t *data = load_file(path, &size);
    DSFont meta;
    int glyph;
    assert(setup_font(data, size, &meta, &glyph));
    const size_t points = 1032;
    const size_t record_size = 10 + 2 + 2 + 10 + points;
    assert(meta.glyf_len >= record_size);
    uint32_t start = meta.glyf_len - (uint32_t)record_size;
    assert(set_glyph_range(&meta, glyph, start, meta.glyf_len));
    uint8_t *record = data + meta.glyf + start;
    memset(record, 0, record_size);
    put16(record, 1);
    put16(record + 10, (uint16_t)(points - 1));
    put16(record + 12, 0);
    size_t at = 14, remaining = points;
    while (remaining) {
        size_t run = remaining > 255 ? 255 : remaining;
        record[at++] = 0x3d; /* On-curve, positive short Y delta, repeated. */
        record[at++] = (uint8_t)(run - 1);
        remaining -= run;
    }
    assert(at + points == record_size);
    memset(record + at, 255, points);
    DSFont *font = ds_font_create(data, size, 16);
    assert(font);
    const DSFontGlyph *space = ds_font_glyph(font, ' ');
    assert(space && space->width == 0 && space->height == 0);
    ds_font_destroy(font);
    free(data);
}
int main(int argc, char **argv) {
    assert(argc == 2);
    test_valid_font(argv[1]);
    test_glyph_bounds(argv[1]);
    test_truncated_repeat(argv[1]);
    test_bad_contour_ends(argv[1]);
    test_truncated_composite(argv[1]);
    test_tall_glyph(argv[1]);
    puts("font memory: valid font and malformed glyph bounds OK");
    return 0;
}
'''


def main():
    if not FONT.is_file():
        raise SystemExit(f"font memory: missing test font {FONT}")
    with tempfile.TemporaryDirectory(prefix="cb4-font-memory-") as directory:
        temp = Path(directory)
        source = temp / "test.c"
        binary = temp / "test"
        source.write_text(HARNESS)
        subprocess.run(
            [
                *shlex.split(os.environ.get("CC", "clang")),
                "-std=c99", "-O1", "-g",
                "-fsanitize=address,undefined,float-cast-overflow",
                "-fno-sanitize-recover=all", "-fno-omit-frame-pointer",
                "-fno-pie", "-no-pie",
                "-I", str(ROOT / "tools/host_test/stub"),
                "-I", str(ROOT / "src"), "-I", str(ROOT),
                str(source), "-lm", "-o", str(binary),
            ],
            check=True,
        )
        result = subprocess.run([str(binary), str(FONT)], capture_output=True, text=True, timeout=20)
        print(result.stdout, end="")
        if result.returncode:
            print(result.stderr[:6000])
            raise SystemExit(result.returncode)


if __name__ == "__main__":
    main()
