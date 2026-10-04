#include "runtime.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int screen_w = 1280;
int screen_h = 720;
double dt;

static int saw_play;
static int saw_attack;
static int saw_grass;
static int saw_idle;
static int saw_punch;
static int loaded_assets;

static void clear_observations(void) {
    saw_play = 0;
    saw_attack = 0;
    saw_grass = 0;
    saw_idle = 0;
    saw_punch = 0;
}

void ds_set_asset_manager(AAssetManager *assets) {
    assert(assets != NULL);
}

int png_load(const char *name) {
    if (!strcmp(name, "grass.png") || !strcmp(name, "ordinary.png") ||
        !strcmp(name, "ordinary_punch.png")) {
        ++loaded_assets;
        return 1;
    }
    return 0;
}

void rect(float x, float y, float w, float h, uint32_t color) {
    (void)x; (void)y; (void)w; (void)h; (void)color;
}
void roundrect(float x, float y, float w, float h, float radius, uint32_t color) {
    (void)x; (void)y; (void)w; (void)h; (void)radius; (void)color;
}
void rect_rot(float x, float y, float w, float h, float angle, uint32_t color) {
    (void)x; (void)y; (void)w; (void)h; (void)angle; (void)color;
}
void circle(float x, float y, float radius, uint32_t color) {
    (void)x; (void)y; (void)radius; (void)color;
}
void ring(float x, float y, float radius, float thickness, uint32_t color) {
    (void)x; (void)y; (void)radius; (void)thickness; (void)color;
}
void line(float x1, float y1, float x2, float y2, float thickness, uint32_t color) {
    (void)x1; (void)y1; (void)x2; (void)y2; (void)thickness; (void)color;
}
void clear_screen(uint32_t color) { (void)color; }

static void observe_texture(const char *name) {
    if (!strcmp(name, "grass.png")) saw_grass = 1;
    if (!strcmp(name, "ordinary.png")) saw_idle = 1;
    if (!strcmp(name, "ordinary_punch.png")) saw_punch = 1;
}

void tex(float x, float y, const char *name, float angle, float scale) {
    (void)x; (void)y; (void)angle; (void)scale;
    observe_texture(name);
}
void tex_tint(float x, float y, const char *name, float angle, float scale, uint32_t color) {
    (void)x; (void)y; (void)angle; (void)scale; (void)color;
    observe_texture(name);
}

static void observe_text(const char *value) {
    if (!strcmp(value, "ИГРАТЬ")) saw_play = 1;
    if (!strcmp(value, "УДАР")) saw_attack = 1;
}

void text(const char *value, float x, float y, uint32_t color) {
    (void)x; (void)y; (void)color;
    observe_text(value);
}
void text_scaled(const char *value, float x, float y, uint32_t color, float scale) {
    (void)x; (void)y; (void)color; (void)scale;
    observe_text(value);
}
int text_width(const char *value) { return (int)strlen(value) * 8; }
int text_height(const char *value) { (void)value; return 24; }
int text_ink_width(const char *value) { return text_width(value); }
int text_ink_height(const char *value) { return text_height(value); }
int text_ink_top(const char *value) { (void)value; return 0; }

int main(void) {
    game_init((AAssetManager *)1);
    assert(loaded_assets == 3);

    clear_observations();
    game_draw();
    assert(saw_play);
    assert(!saw_grass && !saw_idle && !saw_punch);

    game_touch(640.0f, 420.0f, 0, 0);
    clear_observations();
    game_draw();
    assert(saw_grass && saw_idle && saw_attack);
    assert(!saw_punch);

    game_touch(1170.0f, 616.0f, 0, 0);
    clear_observations();
    game_draw();
    assert(saw_grass && saw_punch && saw_attack);

    dt = 0.3;
    game_update();
    clear_observations();
    game_draw();
    assert(saw_idle && !saw_punch);

    assert(game_back() == 1);
    clear_observations();
    game_draw();
    assert(saw_play && !saw_grass);
    assert(game_back() == 0);

    puts("pure C game flow: ok");
    return 0;
}
