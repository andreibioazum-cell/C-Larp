/*
 * C-Larp Obby
 *
 * This is the native game layer. There is deliberately no script runtime in the
 * game loop: the login, movement, jump and scene are ordinary C code and the
 * existing Vulkan command renderer is the only way pixels reach the screen.
 *
 * The scene is a small Roblox-like parkour course. World coordinates are projected
 * through a perspective camera into Vulkan primitives until the player's GLTF
 * model is supplied in assets/models/player/. The player is intentionally a
 * static placeholder for now; animation and skin loading are the next step.
 */
#include "runtime.h"
#include <ctype.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define OB_PLAYER_HALF 0.34f
#define OB_GRAVITY 14.0f
#define OB_JUMP_SPEED 6.2f
#define OB_WALK_SPEED 3.4f
#define OB_WORLD_SCALE 58.0f
#define OB_MAX_NICK 16

/* Opaque colours are written as 0xRRGGBBAA, matching runtime.h. */
#define C_SKY       0xff8ed8ffu
#define C_SKY_TOP   0xff4d85d6u
#define C_CLOUD     0x8fffffffu
#define C_PLATFORM  0xff41b7a1u
#define C_PLATFORM2 0xff278b81u
#define C_EDGE      0xff19545bu
#define C_PLAYER    0xffffc857u
#define C_PLAYER_SIDE 0xffe28b35u
#define C_WHITE     0xffffffffu
#define C_INK       0xff102438u
#define C_ACCENT    0xffffdc62u
#define C_DANGER    0xffed6a68u
#define C_SUCCESS   0xff58d68du

/* z is the course direction. A platform is a simple cuboid with a flat top. */
typedef struct {
    float x, y, z;
    float width, depth, height;
    uint32_t colour;
} Platform;

static const Platform course[] = {
    {  0.0f, 0.0f,  0.0f,  4.0f, 4.0f, 0.35f, C_PLATFORM },
    {  0.8f, 0.3f,  4.8f,  3.2f, 2.6f, 0.35f, C_PLATFORM2 },
    { -1.0f, 0.6f,  8.7f,  2.8f, 2.2f, 0.35f, C_PLATFORM },
    {  1.2f, 0.9f, 12.4f,  2.4f, 2.0f, 0.35f, C_PLATFORM2 },
    { -0.5f, 1.2f, 16.0f,  2.7f, 2.1f, 0.35f, C_PLATFORM },
    {  1.1f, 1.6f, 20.0f,  2.1f, 1.8f, 0.35f, C_PLATFORM2 },
    {  0.0f, 2.0f, 23.8f,  4.0f, 3.4f, 0.35f, C_SUCCESS },
};
#define COURSE_COUNT ((int)(sizeof(course) / sizeof(course[0])))

typedef enum { SCREEN_NICKNAME, SCREEN_OBBY, SCREEN_FINISH } GameScreen;
static GameScreen screen = SCREEN_NICKNAME;
static char player_nick[OB_MAX_NICK + 1];
static float player_x, player_y, player_z;
static float player_vy;
static int grounded;
static int jump_queued;
static float camera_x, camera_z;
static float elapsed;
static int pointer_id = -1;
static float pointer_x, pointer_y;
static int pointer_down;
static int attempted_bad_nick;
static int finish_flash;

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

static int nick_valid(const char *nick) {
    size_t n;
    if (!nick) return 0;
    n = strlen(nick);
    if (n == 0 || n > OB_MAX_NICK) return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)nick[i];
        if (!(isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

static void copy_nick_from_java(void) {
    const char *raw = keyboard_get_raw();
    if (!raw) raw = "";
    size_t n = strlen(raw);
    if (n > OB_MAX_NICK) n = OB_MAX_NICK;
    memcpy(player_nick, raw, n);
    player_nick[n] = '\0';
}

static void start_obby(void) {
    copy_nick_from_java();
    if (!nick_valid(player_nick)) {
        attempted_bad_nick = 1;
        return;
    }
    attempted_bad_nick = 0;
    keyboard_hide();
    screen = SCREEN_OBBY;
    player_x = 0.0f;
    player_y = 0.9f;
    player_z = -0.7f;
    player_vy = 0.0f;
    grounded = 1;
    jump_queued = 0;
    camera_x = 0.0f;
    camera_z = -4.0f;
}

void init(AAssetManager *assets) {
    ds_set_asset_manager(assets);
    screen = SCREEN_NICKNAME;
    player_nick[0] = '\0';
    attempted_bad_nick = 0;
    finish_flash = 0;
    elapsed = 0.0f;
    keyboard_clear();
    /* Java owns the actual EditText and IME. The Vulkan screen below only paints
     * the surrounding UI and consumes its value through keyboard_get_raw(). */
    keyboard_show();
}

void reset(void) {
    keyboard_hide();
    screen = SCREEN_NICKNAME;
    player_nick[0] = '\0';
    attempted_bad_nick = 0;
    keyboard_clear();
    keyboard_show();
}

static int near(float a, float b, float radius) {
    return fabsf(a - b) <= radius;
}

/* A small perspective projection. y is vertical, z points away from the camera. */
typedef struct { float x, y, scale; } Projected;
static Projected project_world(float x, float y, float z) {
    float dz = z - camera_z;
    float depth = clampf(dz, 1.0f, 40.0f);
    float scale = 4.4f / (depth + 4.4f);
    float horizon = screen_h * 0.38f;
    Projected p;
    p.scale = scale;
    p.x = screen_w * 0.5f + (x - camera_x) * OB_WORLD_SCALE * scale;
    p.y = horizon + (z - camera_z) * 11.0f * scale - y * OB_WORLD_SCALE * scale;
    return p;
}

static void draw_cloud(float x, float y, float r) {
    circle(x, y, r, C_CLOUD);
    circle(x + r * 0.8f, y + r * 0.15f, r * 0.78f, C_CLOUD);
    circle(x - r * 0.75f, y + r * 0.18f, r * 0.65f, C_CLOUD);
}

static void draw_sky(void) {
    int bands = 8;
    float band_h = screen_h > 0 ? (float)screen_h / bands : 100.0f;
    for (int i = 0; i < bands; ++i) {
        float t = (float)i / (float)(bands - 1);
        uint32_t r = (uint32_t)(78.0f + 64.0f * t);
        uint32_t g = (uint32_t)(137.0f + 75.0f * t);
        uint32_t b = (uint32_t)(214.0f + 41.0f * t);
        rect(0.0f, band_h * i, (float)screen_w, band_h + 2.0f,
             (r << 24) | (g << 16) | (b << 8) | 0xffu);
    }
    draw_cloud(screen_w * 0.16f, screen_h * 0.18f, 25.0f);
    draw_cloud(screen_w * 0.78f, screen_h * 0.27f, 33.0f);
    draw_cloud(screen_w * 0.52f, screen_h * 0.13f, 18.0f);
}

static void draw_platform(const Platform *block) {
    Projected top = project_world(block->x, block->y, block->z);
    float w = block->width * OB_WORLD_SCALE * top.scale;
    float d = block->depth * 11.0f * top.scale;
    float side = block->height * OB_WORLD_SCALE * top.scale;
    float x = top.x - w * 0.5f;
    float y = top.y - d * 0.5f;
    /* Layered faces give the course volume while all vertices are still submitted
     * through the Vulkan renderer. The dark rim makes landings readable on phones. */
    rect(x + 4.0f, y + side, w, side * 0.55f, C_EDGE);
    rect(x, y + side * 0.20f, w, side * 0.80f, block->colour);
    rect_rot(x, y, w, d, -0.06f, block->colour);
    line(x, y + d, x + w, y + d, 2.0f, C_EDGE);
}

static void draw_course(void) {
    /* Back-to-front painter order is stable and makes the furthest landing sit
     * behind the near one even on GPUs where depth is disabled for the 2D HUD. */
    for (int i = COURSE_COUNT - 1; i >= 0; --i) draw_platform(&course[i]);
    Projected p = project_world(player_x, player_y, player_z);
    float s = clampf(p.scale, 0.45f, 1.25f);
    float body = 0.78f * OB_WORLD_SCALE * s;
    float px = p.x - body * 0.5f;
    float py = p.y - body;
    circle(p.x, p.y + 5.0f, body * 0.30f, 0x40142738u);
    /* Static player placeholder. A GLTF file dropped in assets/models/player/
     * will replace this block when the model loader is added; there are no
     * animation assumptions in this first native C version. */
    rect_rot(px, py, body, body, 0.0f, C_PLAYER);
    rect(px + body * 0.18f, py + body * 0.25f, body * 0.15f, body * 0.15f, C_INK);
    rect(px + body * 0.67f, py + body * 0.25f, body * 0.15f, body * 0.15f, C_INK);
    rect(px + body * 0.22f, py + body * 0.78f, body * 0.56f, body * 0.12f, C_PLAYER_SIDE);
}

static void draw_hud(void) {
    float pad = clampf(screen_w * 0.025f, 14.0f, 32.0f);
    roundrect(pad, pad, 250.0f, 58.0f, 14.0f, 0xdd102438u);
    text_scaled("C-Larp OBBY", pad + 16.0f, pad + 9.0f, C_ACCENT, 0.72f);
    text_scaled(player_nick, pad + 16.0f, pad + 34.0f, C_WHITE, 0.55f);
    float progress = clampf((player_z + 1.0f) / 25.0f, 0.0f, 1.0f);
    float bar_w = clampf(screen_w * 0.30f, 170.0f, 420.0f);
    float bx = (screen_w - bar_w) * 0.5f;
    roundrect(bx, 22.0f, bar_w, 16.0f, 8.0f, 0x70102438u);
    roundrect(bx + 3.0f, 25.0f, (bar_w - 6.0f) * progress, 10.0f, 5.0f, C_SUCCESS);
    text_scaled("MOVE", screen_w - 108.0f, screen_h - 70.0f, C_WHITE, 0.52f);
    circle(screen_w - 68.0f, screen_h - 45.0f, 30.0f, 0xb0102438u);
    text_scaled("JUMP", screen_w - 98.0f, screen_h - 51.0f, C_WHITE, 0.48f);
    if (finish_flash > 0) {
        roundrect(screen_w * 0.5f - 170.0f, screen_h * 0.23f, 340.0f, 88.0f, 20.0f, 0xee102438u);
        text_scaled("COURSE COMPLETE!", screen_w * 0.5f - 134.0f, screen_h * 0.25f + 12.0f, C_ACCENT, 0.70f);
        text_scaled("Tap to run it again", screen_w * 0.5f - 96.0f, screen_h * 0.25f + 48.0f, C_WHITE, 0.55f);
    }
}

static void draw_nickname_screen(void) {
    draw_sky();
    float cx = screen_w * 0.5f;
    float card_w = clampf(screen_w * 0.78f, 300.0f, 650.0f);
    float card_h = 310.0f;
    float x = cx - card_w * 0.5f;
    float y = screen_h * 0.5f - card_h * 0.5f;
    roundrect(x + 7.0f, y + 9.0f, card_w, card_h, 24.0f, 0x50102438u);
    roundrect(x, y, card_w, card_h, 24.0f, 0xf5ffffffu);
    text_scaled("C-Larp", cx - 92.0f, y + 30.0f, C_INK, 1.28f);
    text_scaled("3D OBBY", cx - 73.0f, y + 82.0f, C_PLATFORM2, 0.70f);
    text_scaled("Enter your nickname", x + 36.0f, y + 133.0f, C_INK, 0.60f);
    roundrect(x + 30.0f, y + 160.0f, card_w - 60.0f, 54.0f, 12.0f,
              attempted_bad_nick ? 0xffffe3e0u : 0xffedf5f7u);
    const char *nick = keyboard_get_raw();
    if (!nick || !*nick) {
        text_scaled("nickname", x + 49.0f, y + 176.0f, 0xff607d8bu, 0.62f);
    } else {
        text_scaled(nick, x + 49.0f, y + 176.0f, C_INK, 0.62f);
    }
    roundrect(cx - 115.0f, y + 235.0f, 230.0f, 52.0f, 15.0f,
              attempted_bad_nick ? C_DANGER : C_PLATFORM2);
    text_scaled("PLAY", cx - 29.0f, y + 251.0f, C_WHITE, 0.70f);
    if (attempted_bad_nick)
        text_scaled("Use 1-16: A-Z, 0-9 or _", cx - 136.0f, y + 296.0f, C_DANGER, 0.46f);
    else
        text_scaled("Vulkan native • C game", cx - 116.0f, y + 296.0f, C_INK, 0.42f);
}

void update(void) {
    elapsed += (float)dt;
    if (screen == SCREEN_NICKNAME) {
        copy_nick_from_java();
        if (keyboard_enter_pressed()) start_obby();
        return;
    }
    if (screen == SCREEN_FINISH) {
        if (finish_flash > 0) --finish_flash;
        if (pointer_down && pointer_y < screen_h * 0.7f) start_obby();
        return;
    }
    float axis = 0.0f;
    if (pointer_down && pointer_x < screen_w * 0.55f) {
        axis = clampf((pointer_x - screen_w * 0.23f) / (screen_w * 0.18f), -1.0f, 1.0f);
    }
    /* The course advances forward automatically, like a simple obby runner. */
    player_x += axis * OB_WALK_SPEED * (float)dt;
    player_z += OB_WALK_SPEED * (float)dt * (0.72f + 0.28f * fabsf(axis));
    player_x = clampf(player_x, -2.0f, 2.0f);
    if (jump_queued && grounded) {
        player_vy = OB_JUMP_SPEED;
        grounded = 0;
    }
    jump_queued = 0;
    player_vy -= OB_GRAVITY * (float)dt;
    player_y += player_vy * (float)dt;
    /* Land on a platform when the player's feet cross its top. */
    grounded = 0;
    for (int i = 0; i < COURSE_COUNT; ++i) {
        const Platform *p = &course[i];
        if (player_z + 0.28f >= p->z && player_z - 0.28f <= p->z + p->depth &&
            player_x >= p->x - p->width * 0.5f && player_x <= p->x + p->width * 0.5f &&
            player_y <= p->y + p->height + 0.8f && player_vy <= 0.0f) {
            player_y = p->y + p->height + 0.45f;
            player_vy = 0.0f;
            grounded = 1;
            break;
        }
    }
    if (player_y < -4.0f) {
        player_x = 0.0f; player_y = 0.9f; player_z = -0.7f; player_vy = 0.0f; grounded = 1;
    }
    camera_x += (player_x - camera_x) * clampf((float)dt * 5.0f, 0.0f, 1.0f);
    camera_z += (player_z - 4.0f - camera_z) * clampf((float)dt * 3.0f, 0.0f, 1.0f);
    if (player_z > 22.5f) {
        screen = SCREEN_FINISH;
        finish_flash = 180;
        pointer_down = 0;
    }
}

void draw(Buffer *buffer) {
    (void)buffer;
    if (screen == SCREEN_NICKNAME) {
        draw_nickname_screen();
        return;
    }
    draw_sky();
    draw_course();
    draw_hud();
}

void touch(float x, float y, int action, int id) {
    if (action == 0) {
        pointer_id = id;
        pointer_x = x; pointer_y = y; pointer_down = 1;
        if (screen == SCREEN_NICKNAME) {
            float cx = screen_w * 0.5f;
            float card_y = screen_h * 0.5f - 155.0f;
            if (near(x, cx, 150.0f) && y > card_y + 215.0f && y < card_y + 315.0f) start_obby();
            else keyboard_show();
        } else if (screen == SCREEN_OBBY && x > screen_w * 0.55f) {
            jump_queued = 1;
        }
        return;
    }
    if (id != pointer_id && action != 2) return;
    pointer_x = x; pointer_y = y;
    if (action == 1 || action == 3) {
        pointer_down = 0;
        pointer_id = -1;
    }
}

int back_pressed(void) {
    if (keyboard_visible()) {
        keyboard_hide();
        return 1;
    }
    if (screen != SCREEN_NICKNAME) {
        reset();
        return 1;
    }
    return 0;
}
