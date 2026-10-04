#include "runtime.h"

#include <math.h>
#include <stdint.h>

/* A deliberately small game written directly in C.  There is no generated
 * source and no scripting layer: these hooks are called by main.c. */

enum GameScreen {
    SCREEN_MENU,
    SCREEN_ARENA
};

typedef struct RectF {
    float x;
    float y;
    float w;
    float h;
} RectF;

static const uint32_t COLOR_MENU = 0xFF1A1A2E;
static const uint32_t COLOR_PURPLE = 0xFF5F10A0;
static const uint32_t COLOR_PURPLE_DOWN = 0xFF7A24BE;
static const uint32_t COLOR_RED = 0xFFE94560;
static const uint32_t COLOR_WHITE = 0xFFFFFFFF;
static const float PUNCH_DURATION = 0.28f;

static enum GameScreen screen = SCREEN_MENU;
static float punch_left;
static int grass_loaded;
static int idle_loaded;
static int punch_loaded;

static float minf(float a, float b) {
    return a < b ? a : b;
}

static float maxf(float a, float b) {
    return a > b ? a : b;
}

static float clampf(float value, float low, float high) {
    return minf(maxf(value, low), high);
}

static int point_in_rect(float px, float py, RectF r) {
    return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
}

static RectF play_button(void) {
    RectF r;
    r.w = minf(320.0f, maxf(180.0f, (float)screen_w - 40.0f));
    r.h = 64.0f;
    r.x = ((float)screen_w - r.w) * 0.5f;
    r.y = (float)screen_h * 0.55f;
    return r;
}

static RectF back_button(void) {
    RectF r = {20.0f, 20.0f, 170.0f, 52.0f};
    if (r.w > (float)screen_w - 40.0f) r.w = (float)screen_w - 40.0f;
    return r;
}

static void attack_button(float *cx, float *cy, float *radius) {
    float r = clampf((float)screen_h * 0.105f, 54.0f, 76.0f);
    *radius = r;
    *cx = (float)screen_w - r - 34.0f;
    *cy = (float)screen_h - r - 28.0f;
}

static void centered_text(const char *label, float y, float scale, uint32_t color) {
    float width = (float)text_ink_width(label) * scale;
    text_scaled(label, ((float)screen_w - width) * 0.5f, y, color, scale);
}

static void text_in_box(const char *label, RectF box, float scale, uint32_t color) {
    float width = (float)text_ink_width(label) * scale;
    float height = (float)text_ink_height(label) * scale;
    float top = (float)text_ink_top(label) * scale;
    float x = box.x + (box.w - width) * 0.5f;
    float y = box.y + (box.h - height) * 0.5f - top + 1.5f;
    text_scaled(label, x, y, color, scale);
}

static void draw_button(RectF bounds, const char *label, uint32_t color, float scale) {
    roundrect(bounds.x, bounds.y, bounds.w, bounds.h, 20.0f, color);
    text_in_box(label, bounds, scale, COLOR_WHITE);
}

static void draw_fallback_cube(float x, float y, float size, int punching) {
    float hand = size * 0.18f;
    roundrect(x, y, size, size, size * 0.08f, 0xFF171923);
    circle(x + size * 0.35f, y + size * 0.42f, size * 0.045f, COLOR_WHITE);
    circle(x + size * 0.65f, y + size * 0.42f, size * 0.045f, COLOR_WHITE);
    if (punching) {
        roundrect(x + size - hand * 0.1f, y + size * 0.36f,
                  size * 0.42f, hand, hand * 0.45f, 0xFF171923);
        circle(x + size * 1.35f, y + size * 0.45f, hand * 0.72f, 0xFF171923);
    }
}

static void draw_menu(void) {
    RectF play = play_button();
    float title_y = maxf(54.0f, (float)screen_h * 0.23f);

    clear_screen(COLOR_MENU);
    centered_text("Cubic Battle 4", title_y, 1.35f, COLOR_WHITE);
    centered_text("Игра на чистом C", title_y + 64.0f, 0.62f, 0xFFD9D4E3);
    draw_button(play, "ИГРАТЬ", COLOR_PURPLE, 1.0f);
}

static void draw_grass(void) {
    if (!grass_loaded) {
        clear_screen(0xFF3B8E2F);
        return;
    }
    for (float y = 0.0f; y < (float)screen_h; y += 256.0f) {
        for (float x = 0.0f; x < (float)screen_w; x += 256.0f) {
            tex(x, y, "grass.png", 0.0f, 1.0f);
        }
    }
}

static void draw_punch_effect(float center_x, float center_y, float sprite_size) {
    float life = punch_left / PUNCH_DURATION;
    float progress = 1.0f - life;
    int alpha = (int)(190.0f * life);
    uint32_t color;
    float impact_x;

    if (alpha < 1) return;
    color = ((uint32_t)alpha << 24) | 0x00FFFFFFu;
    impact_x = center_x + sprite_size * 0.72f + progress * 45.0f;
    ring(impact_x, center_y, 24.0f + progress * 42.0f, 6.0f, color);
    line(center_x + sprite_size * 0.34f, center_y - 25.0f,
         impact_x + 34.0f, center_y - 48.0f, 6.0f, color);
    line(center_x + sprite_size * 0.34f, center_y + 25.0f,
         impact_x + 34.0f, center_y + 48.0f, 6.0f, color);
}

static void draw_fighter(void) {
    float scale = clampf((float)screen_h / 300.0f, 1.65f, 2.75f);
    float sprite_size = 50.0f * scale;
    float progress = punch_left > 0.0f ? 1.0f - punch_left / PUNCH_DURATION : 0.0f;
    float lunge = punch_left > 0.0f ? sinf(progress * 3.14159265f) * 18.0f : 0.0f;
    float cx = (float)screen_w * 0.5f + lunge;
    float cy = (float)screen_h * 0.52f;
    float x = cx - sprite_size * 0.5f;
    float y = cy - sprite_size * 0.5f;
    int punching = punch_left > 0.0f;

    if ((punching && punch_loaded) || (!punching && idle_loaded)) {
        const char *sprite = punching ? "ordinary_punch.png" : "ordinary.png";
        tex_tint(x - 8.0f, y + 10.0f, sprite, 0.0f, scale, 0x65000000);
        tex(x, y, sprite, 0.0f, scale);
    } else {
        draw_fallback_cube(x, y, sprite_size, punching);
    }
    if (punching) draw_punch_effect(cx, cy, sprite_size);
}

static void draw_arena(void) {
    RectF back = back_button();
    float attack_x;
    float attack_y;
    float attack_radius;
    RectF attack_box;

    draw_grass();
    rect(0.0f, 0.0f, (float)screen_w, (float)screen_h, 0x24000000);
    draw_fighter();

    draw_button(back, "НАЗАД", COLOR_PURPLE, 0.72f);
    centered_text("Нажми «УДАР»", 32.0f, 0.58f, COLOR_WHITE);

    attack_button(&attack_x, &attack_y, &attack_radius);
    circle(attack_x, attack_y, attack_radius,
           punch_left > 0.0f ? COLOR_PURPLE_DOWN : COLOR_RED);
    ring(attack_x, attack_y, attack_radius, 5.0f, 0xFF171923);
    attack_box.x = attack_x - attack_radius;
    attack_box.y = attack_y - attack_radius;
    attack_box.w = attack_radius * 2.0f;
    attack_box.h = attack_radius * 2.0f;
    text_in_box("УДАР", attack_box, 0.62f, COLOR_WHITE);
}

void game_reset(void) {
    screen = SCREEN_MENU;
    punch_left = 0.0f;
}

void game_init(AAssetManager *assets) {
    ds_set_asset_manager(assets);
    grass_loaded = png_load("grass.png");
    idle_loaded = png_load("ordinary.png");
    punch_loaded = png_load("ordinary_punch.png");
    game_reset();
}

void game_update(void) {
    if (punch_left > 0.0f) {
        punch_left -= (float)dt;
        if (punch_left < 0.0f) punch_left = 0.0f;
    }
}

void game_draw(void) {
    if (screen == SCREEN_ARENA) draw_arena();
    else draw_menu();
}

void game_touch(float x, float y, int action, int pointer_id) {
    float attack_x;
    float attack_y;
    float attack_radius;
    float dx;
    float dy;
    (void)pointer_id;

    if (action != 0) return;
    if (screen == SCREEN_MENU) {
        if (point_in_rect(x, y, play_button())) screen = SCREEN_ARENA;
        return;
    }
    if (point_in_rect(x, y, back_button())) {
        screen = SCREEN_MENU;
        punch_left = 0.0f;
        return;
    }
    attack_button(&attack_x, &attack_y, &attack_radius);
    dx = x - attack_x;
    dy = y - attack_y;
    if (dx * dx + dy * dy <= (attack_radius + 24.0f) * (attack_radius + 24.0f)) {
        punch_left = PUNCH_DURATION;
    }
}

int game_back(void) {
    if (screen == SCREEN_ARENA) {
        screen = SCREEN_MENU;
        punch_left = 0.0f;
        return 1;
    }
    return 0;
}
