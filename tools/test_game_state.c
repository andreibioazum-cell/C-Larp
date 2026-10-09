#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game/game.c"

int screen_w = 320;
int screen_h = 180;
double dt = 1.0 / 60.0;
int mouse_clicked;
double ds_mouse_x;
double ds_mouse_y;
Joy joy;

static int ground_texture_missing;
static int ground_tiles;
static int ground_fills;
static uint32_t last_ground_fill;
static const char *last_ground;
static int showdown_playing;
static int showdown_play_calls;
static int lobby_playing;
static int winter_playing;
static int winter_loop_calls;
static int snowflake_draws;
static int snowflake_tinted;
static double snowflake_centres[64];
static int snowflake_centre_n;
static int star_line_count;

struct DSArray {
    double *data;
    size_t length;
    size_t capacity;
};

DSArray *arr_new(void) {
    DSArray *array = calloc(1, sizeof(*array));
    assert(array);
    return array;
}

void arr_push(DSArray *array, double value) {
    if (array->length == array->capacity) {
        size_t capacity = array->capacity ? array->capacity * 2 : 8;
        double *data = realloc(array->data, capacity * sizeof(*data));
        assert(data);
        array->data = data;
        array->capacity = capacity;
    }
    array->data[array->length++] = value;
}

double arr_get(DSArray *array, double index) {
    size_t position = (size_t)index;
    return array && position < array->length ? array->data[position] : 0;
}

void arr_set(DSArray *array, double index, double value) {
    size_t position = (size_t)index;
    while (array->length <= position)
        arr_push(array, 0);
    array->data[position] = value;
}

double arr_len(DSArray *array) {
    return array ? (double)array->length : 0;
}

double clamp(double value, double low, double high) {
    return value < low ? low : value > high ? high : value;
}

void arr_clear(DSArray *array) {
    if (array)
        array->length = 0;
}

void arr_free(DSArray *array) {
    if (!array)
        return;
    free(array->data);
    free(array);
}

void ds_runtime_error(const char *format, ...) {
    (void)format;
    abort();
}

void ds_log(const char *format, ...) {
    (void)format;
}
double dist(double x, double y, double a, double b) {
    return hypot(x - a, y - b);
}
double ds_mod(double value, double divisor) {
    return (double)((int)value % (int)divisor);
}
int str_eq(const char *a, const char *b) {
    return a && b && strcmp(a, b) == 0 ? 1 : 0;
}
double str_len(const char *s) {
    return s ? (double)strlen(s) : 0;
}
/* Склейка строк, как в рантайме: два чередующихся буфера, чтобы вызов
 * ds_concat(ds_concat(a, b), c) не затирал сам себя. */
char *ds_concat(const char *left, const char *right) {
    static char buffers[4][512];
    static int turn = 0;
    turn = (turn + 1) & 3;
    snprintf(buffers[turn], sizeof(buffers[turn]), "%s%s", left ? left : "", right ? right : "");
    return buffers[turn];
}
void keyboard_hide(void) {}
void keyboard_clear(void) {}
void keyboard_show(void) {}
static double stub_net_slot = -1;
static double stub_online[4];
static double stub_level[4];
static double stub_grab[4];
static double stub_gx[4];
static double stub_gy[4];
static double stub_gdx[4];
static double stub_gdy[4];
double net_slot(void) {
    return stub_net_slot;
}
double net_player_online(double slot) {
    return stub_online[(int)slot];
}
double net_player_level(double slot) {
    return stub_level[(int)slot];
}
double net_player_grab(double slot) {
    return stub_grab[(int)slot];
}
double net_player_grab_x(double slot) {
    return stub_gx[(int)slot];
}
double net_player_grab_y(double slot) {
    return stub_gy[(int)slot];
}
double net_player_grab_dx(double slot) {
    return stub_gdx[(int)slot];
}
double net_player_grab_dy(double slot) {
    return stub_gdy[(int)slot];
}
void net_mark_achievement_flag(double flag) {
    (void)flag;
}
void net_save_azum_revives(double value) {
    (void)value;
}
double net_load_bp_level(void) {
    return 0;
}
void net_set_class(double value) {
    (void)value;
}
void net_set_level(double value) {
    (void)value;
}
void net_set_skin(double value) {
    (void)value;
}
void net_save_astra(double owned, double level, double unlocked) {
    (void)owned;
    (void)level;
    (void)unlocked;
}
static double stub_astra_rw;
static double stub_login_status;
static char stub_login_nick[24];
static char stub_login_pass[24];
double net_load_astra_rw(void) {
    return stub_astra_rw;
}
void net_save_astra_rw(double on) {
    stub_astra_rw = on ? 1 : 0;
}
double net_login_status(void) {
    return stub_login_status;
}
const char *net_login_nick(void) {
    return stub_login_nick;
}
const char *net_login_pass(void) {
    return stub_login_pass;
}
static char last_bye[256];
static int bye_calls;
void bye_notice_show(const char *text) {
    bye_calls++;
    snprintf(last_bye, sizeof(last_bye), "%s", text ? text : "");
}
void ring(float x, float y, float r, float t, uint32_t color) {
    (void)x;
    (void)y;
    (void)r;
    (void)t;
    (void)color;
}
void tex_tint(float x, float y, const char *name, float angle, float scale, uint32_t color) {
    (void)x;
    (void)y;
    (void)angle;
    (void)scale;
    (void)color;
    ground_tiles++;
    last_ground = name;
    if (name && SNOWFLAKE_TEX && strcmp(name, SNOWFLAKE_TEX) == 0) {
        snowflake_tinted++;
    }
}
void net_save_progress_all(double a, double b, double c, double d, double e, double f, double g, double h, double i,
                           double j, double k, double l, double m, double n, double o, double p, double q, double r) {
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    (void)h;
    (void)i;
    (void)j;
    (void)k;
    (void)l;
    (void)m;
    (void)n;
    (void)o;
    (void)p;
    (void)q;
    (void)r;
}
void net_save_quest_state(double a, double b, double c, double d, double e, double f, double g, double h, double i,
                          double j, double k, double l) {
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    (void)h;
    (void)i;
    (void)j;
    (void)k;
    (void)l;
}

int snd_load(const char *name) {
    return name && *name;
}
int snd_play(const char *name) {
    if (name && SHOWDOWN_MUSIC && strcmp(name, SHOWDOWN_MUSIC) == 0) {
        showdown_playing = 1;
        showdown_play_calls++;
    }
    return 1;
}
int snd_loop(const char *name) {
    if (name && LOBBY_MUSIC && strcmp(name, LOBBY_MUSIC) == 0) {
        lobby_playing = 1;
    }
    if (name && WINTER_JINGLE && strcmp(name, WINTER_JINGLE) == 0) {
        winter_playing = 1;
        winter_loop_calls++;
    }
    return 1;
}
int snd_playing(const char *name) {
    if (name && LOBBY_MUSIC && strcmp(name, LOBBY_MUSIC) == 0)
        return lobby_playing;
    if (name && WINTER_JINGLE && strcmp(name, WINTER_JINGLE) == 0)
        return winter_playing;
    return name && SHOWDOWN_MUSIC && strcmp(name, SHOWDOWN_MUSIC) == 0 ? showdown_playing : 0;
}
void snd_stop(const char *name) {
    if (name && LOBBY_MUSIC && strcmp(name, LOBBY_MUSIC) == 0)
        lobby_playing = 0;
    if (name && WINTER_JINGLE && strcmp(name, WINTER_JINGLE) == 0)
        winter_playing = 0;
    if (name && SHOWDOWN_MUSIC && strcmp(name, SHOWDOWN_MUSIC) == 0)
        showdown_playing = 0;
}
void snd_volume(const char *name, double volume) {
    (void)name;
    (void)volume;
}

int tex_ready(const char *name) {
    return name && *name && !ground_texture_missing;
}

void rect(float x, float y, float width, float height, uint32_t color) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    last_ground_fill = color;
    ground_fills++;
}

void line(float x1, float y1, float x2, float y2, float thickness, uint32_t color) {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)thickness;
    (void)color;
    star_line_count++;
}

static int candy_draws;
static double candy_alpha_drawn;
static double candy_scale_drawn;
static uint32_t candy_circle_color;
void circle(float x, float y, float r, uint32_t color) {
    (void)x;
    (void)y;
    (void)r;
    candy_circle_color = color;
}
void tex_alpha(float x, float y, const char *name, float angle, float scale, double alpha) {
    (void)x;
    (void)y;
    (void)angle;
    if (name && CANDY_TEX && strcmp(name, CANDY_TEX) == 0) {
        candy_draws++;
        candy_scale_drawn = scale;
        candy_alpha_drawn = alpha;
    }
}

void tex(float x, float y, const char *name, float angle, float scale) {
    (void)x;
    (void)y;
    (void)angle;
    (void)scale;
    ground_tiles++;
    last_ground = name;
    if (name && SNOWFLAKE_TEX && strcmp(name, SNOWFLAKE_TEX) == 0) {
        snowflake_draws++;
        if (snowflake_centre_n < 64) {
            /* tex() receives the top-left corner; the game sets hw = 26*scale, so the centre is 26*scale further right. */
            snowflake_centres[snowflake_centre_n++] = x + 26.0 * scale;
        }
    }
}

int main(void) {
    state_create();
    state_ready = 1;
    assert(player && enemy && punch && gift && enemy_gift);
    assert(ST_LOBBY == 0 && ST_SOLO == 1 && ST_ONLINE == 5);
    assert(player->hp == 10 && enemy->hp == 10);
    assert(class_level_tbl && remote_punches && plates_candies && flake_cache);

    assert(strcmp(tr_play(), "Play") == 0);
    language = 1;
    assert(strcmp(tr_play(), "Играть") == 0);
    language = 0;

    assert(class_count == 6 && CLASS_ASTRA == 4 && CLASS_ASTRA_REWORK == 5);
    star_line_count = 0;
    draw_star(80, 80, 20, 0xFFFF00);
    assert(star_line_count >= 10);
    star_line_count = 0;
    draw_imbalance_stars();
    int steady_star_lines = star_line_count;
    draw_imbalance_stars();
    assert(steady_star_lines > 0 && star_line_count == 2 * steady_star_lines);
    assert(strcmp(tr_class_level_effect(CLASS_ASTRA, 1), tr_class_level_effect(CLASS_ASTRA_REWORK, 1)) == 0);
    assert(strcmp(tr_class_level_effect(CLASS_ASTRA, 2), tr_class_level_effect(CLASS_ASTRA_REWORK, 2)) == 0);
    assert(strcmp(tr_class_level_effect(CLASS_ASTRA, 3), tr_class_level_effect(CLASS_ASTRA_REWORK, 3)) == 0);
    assert(punch_forward_offset == 19 && astra_grab_forward_offset == 19);
    assert(fabs(astra_hit_interval - 0.2) < 1e-9);
    assert(fabs(astra_final_delay - 0.85) < 1e-9);
    assert(fabs(astra_final_time() - astra_rw_beat_time_for(astra_hit_count, 0) - astra_final_delay) < 1e-9);
    assert(class_cost_of(CLASS_AZUM) == 65);
    assert(class_cost_of(CLASS_SANTA) == 100);
    assert(class_cost_of(CLASS_EBUC) == 120);
    assert(class_cost_of(CLASS_ASTRA) == 90);
    assert(class_has_super(CLASS_ASTRA) == 1);
    assert(astra_cd_for(0) == 8);
    assert(astra_cd_for(1) < astra_cd_for(0));
    assert(astra_zone_reach(2) > astra_zone_reach(1));
    assert(astra_throw_dist_for(3) > astra_throw_dist_for(2));
    assert(fabs(astra_grab_total_fraction() - 0.45) < 1e-9);
    assert(strcmp(WINTER_JINGLE, "winter_jingle.wav") == 0);
    assert(strcmp(fighter_sprite(CLASS_ORDINARY, 0, SKIN_NORMAL), ORDINARY_TEX) == 0);
    assert(strcmp(fighter_sprite(CLASS_ORDINARY, 1, SKIN_NORMAL), PUNCH_TEX) == 0);
    assert(strcmp(fighter_sprite(CLASS_ASTRA_REWORK, 0, SKIN_NORMAL), ORDINARY_TEX) == 0);
    assert(strcmp(class_card_tex(CLASS_ASTRA_REWORK, SKIN_NORMAL), ORDINARY_CARD_TEX) == 0);
    assert(class_has_super(CLASS_ASTRA_REWORK) == 1 && player_max_hp_for(CLASS_ASTRA_REWORK) == astra_hp);
    azum_tex_ok = azum_punch_tex_ok = 1;
    azum_zombie_tex_ok = azum_zombie_punch_tex_ok = 1;
    assert(strcmp(fighter_sprite(CLASS_AZUM, 0, SKIN_NORMAL), AZUM_TEX) == 0);
    assert(strcmp(fighter_sprite(CLASS_AZUM, 1, SKIN_NORMAL), AZUM_PUNCH_TEX) == 0);
    assert(strcmp(fighter_sprite(CLASS_AZUM, 0, SKIN_ZOMBIE), AZUM_ZOMBIE_TEX) == 0);
    assert(strcmp(fighter_sprite(CLASS_AZUM, 1, SKIN_ZOMBIE), AZUM_ZOMBIE_PUNCH_TEX) == 0);
    FighterSpritePair azum_pair = look_sprite_pair(LOOK_AZUM);
    FighterSpritePair zombie_pair = look_sprite_pair(LOOK_ZOMBIE);
    assert(strcmp(azum_pair.idle, AZUM_TEX) == 0 && strcmp(azum_pair.punch, AZUM_PUNCH_TEX) == 0);
    assert(strcmp(zombie_pair.idle, AZUM_ZOMBIE_TEX) == 0 &&
           strcmp(zombie_pair.punch, AZUM_ZOMBIE_PUNCH_TEX) == 0);
    resolve_fighter_look(FLOOK_ME, CLASS_ORDINARY, SKIN_NORMAL);
    assert(fighter_look_of(FLOOK_ME) == LOOK_ORDINARY);
    assert(strcmp(look_tex(fighter_look_of(FLOOK_ME), fighter_pose_of(FLOOK_ME, 1)), PUNCH_TEX) == 0);
    resolve_fighter_look(FLOOK_ME, CLASS_AZUM, SKIN_NORMAL);
    assert(fighter_look_of(FLOOK_ME) == LOOK_AZUM);
    assert(strcmp(look_tex(fighter_look_of(FLOOK_ME), fighter_pose_of(FLOOK_ME, 1)), AZUM_PUNCH_TEX) == 0);
    assert(remote_fields == 12);

    assert(rects_overlap(0, 0, 1, 0, 10, 10, 15, 0, 1, 0, 10, 10) == 1);
    assert(rects_overlap(0, 0, 1, 0, 10, 10, 25, 0, 1, 0, 10, 10) == 0);
    assert(circle_hits_box(0, 0, 5, 8, 0, 0, 4) == 1);
    assert(circle_hits_box(0, 0, 2, 8, 0, 0, 4) == 0);

    /* The virtual stick now gives full directional speed beyond its small
     * deadzone: near-center and rim positions travel the same distance. */
    joy.x = 120;
    joy.y = 90;
    joy.r = 70;
    joy_id = -1;
    player_class = CLASS_ORDINARY;
    player->x = 160;
    player->y = 90;
    joy_touch(joy.x + 14, joy.y, 0, 7);
    assert(fabs(joy.dx - 1) < 1e-9 && fabs(joy.dy) < 1e-9);
    move_player();
    double near_stick_move = player->x - 160;
    player->x = 160;
    joy_touch(joy.x + joy.r, joy.y, 2, 7);
    assert(fabs(joy.dx - 1) < 1e-9 && fabs(joy.dy) < 1e-9);
    move_player();
    assert(fabs((player->x - 160) - near_stick_move) < 1e-9);
    player->x = 160;
    joy_touch(joy.x + 5, joy.y, 2, 7);
    assert(joy.dx == 0 && joy.dy == 0);
    move_player();
    assert(fabs(player->x - 160) < 1e-9);
    joy_touch(joy.x + 5, joy.y, 1, 7);

    player->x = 400;
    player->y = 400;
    enemy->x = 399;
    enemy->y = 400;
    enemy->angle = 0.75;
    assert(punch_hits_enemy(player->x, player->y, 1, 0, punch_reach, punch_width) == 0);
    assert(astra_zone_hits_enemy(player->x, player->y, 1, 0, 0) == 0);
    enemy->x = 470;
    assert(punch_hits_enemy(player->x, player->y, 1, 0, punch_reach, punch_width) == 1);
    assert(astra_zone_hits_enemy(player->x, player->y, 1, 0, 0) == 1);

    winter_theme = 0;
    snow_tex_ok = 1;
    game_state = ST_SOLO;
    ground_tiles = ground_fills = 0;
    draw_arena_background();
    assert(ground_tiles > 0 && ground_fills == 1);
    assert(last_ground_fill == (uint32_t)grass_ground_rgb);
    assert(strcmp(last_ground, GRASS) == 0);

    ground_texture_missing = 1;
    ground_tiles = ground_fills = 0;
    draw_arena_background();
    assert(ground_tiles == 0 && ground_fills == 1);
    ground_texture_missing = 0;

    winter_theme = 1;
    ground_tiles = ground_fills = 0;
    draw_arena_background();
    assert(ground_tiles > 0 && ground_fills == 1);
    assert(strcmp(last_ground, SNOW_TEX) == 0);
    assert(snow_active() == 1);
    assert(newyear_active() == 1);
    assert(newyear_menu_active() == 0);
    game_state = ST_LOBBY;
    warn_open = 1;
    studio_open = 0;
    newyear_jingle_ok = 1;
    assert(snow_active() == 0);
    /* The startup notice and transitions do not pause the snow clock. */
    assert(newyear_menu_active() == 1);
    assert(newyear_music_active() == 1);
    newyear_snow_t = 0;
    update_newyear_snow();
    assert(newyear_snow_t > 0);
    t_fade = 0.5;
    t_dir = 1;
    update_newyear_snow();
    t_fade = 0;
    t_dir = 0;
    assert(newyear_snow_t >= 2 * dt);
    assert(newyear_menu_flakes == 22);

    /* Startup notice open and fully opaque: the epilepsy warning keeps every
     * flake under its panel, and nothing is ever drawn above its text. */
    snowflake_draws = 0;
    snowflake_tinted = 0;
    warn_open = 1;
    warn_a = 1;
    assert(newyear_snow_under_notice_active() == 0 && newyear_snow_over_notice_active() == 0);
    draw_newyear_under_notices();
    assert(snowflake_draws == 0);
    draw_newyear_over_notices();
    assert(snowflake_draws == 0);

    /* The panel is fading: the lobby - and its snow - shows through beneath it,
     * still never on top of the legal text, and still in the lobby layout. */
    warn_a = 0.4;
    assert(newyear_snow_under_notice_active() == 1 && newyear_snow_over_notice_active() == 0);
    snowflake_draws = 0;
    snowflake_tinted = 0;
    draw_newyear_under_notices();
    assert(snowflake_draws == (int)newyear_menu_flakes && snowflake_tinted == 0);
    draw_newyear_over_notices();
    assert(snowflake_draws == (int)newyear_menu_flakes);
    warn_a = 1;

    /* Over the studio splash the snow spans the full width, including the centre
     * column. Beneath the lobby the menu column stays clear. Closing the splash
     * must not move any flake: the over-pass at zero opacity has to match the
     * lobby layout exactly. */
    {
        int saved_w = screen_w;
        int saved_h = screen_h;
        double saved_t = newyear_snow_t;
        double lo = 0;
        double hi = 0;
        double lobby_x[64];
        int centre_over = 0;
        int centre_lobby = 0;
        int handoff_mismatch = 0;
        int k = 0;

        assert(newyear_menu_flakes <= 64);
        screen_w = 1080;
        screen_h = 2400;
        lo = menu_x() - newyear_snow_col_pad;
        hi = menu_x() + btn_w + newyear_snow_col_pad;
        for (k = 0; k < 200; k++) {
            int j = 0;
            newyear_snow_t = 1.0 + k * 0.173;

            studio_open = 0;
            studio_a = 0;
            warn_open = 0;
            warn_a = 0;
            snowflake_centre_n = 0;
            draw_newyear_under_notices();
            assert(snowflake_centre_n == (int)newyear_menu_flakes);
            for (j = 0; j < snowflake_centre_n; j++) {
                lobby_x[j] = snowflake_centres[j];
                if (lobby_x[j] >= lo && lobby_x[j] <= hi) {
                    centre_lobby++;
                }
            }

            studio_open = 1;
            studio_a = 0;
            snowflake_centre_n = 0;
            draw_newyear_over_notices();
            assert(snowflake_centre_n == (int)newyear_menu_flakes);
            for (j = 0; j < snowflake_centre_n; j++) {
                double d = snowflake_centres[j] - lobby_x[j];
                if (d > 1e-3 || d < -1e-3) {
                    handoff_mismatch++;
                }
            }

            studio_a = 1;
            snowflake_centre_n = 0;
            draw_newyear_over_notices();
            assert(snowflake_centre_n == (int)newyear_menu_flakes);
            for (j = 0; j < snowflake_centre_n; j++) {
                if (snowflake_centres[j] >= lo && snowflake_centres[j] <= hi) {
                    centre_over++;
                }
            }
        }
        /* The centre column is about 31% of a 1080 px screen; require at least 10%. */
        assert(centre_lobby == 0);
        assert(handoff_mismatch == 0);
        assert(centre_over >= 200 * (int)newyear_menu_flakes / 10);

        screen_w = saved_w;
        screen_h = saved_h;
        newyear_snow_t = saved_t;
        studio_open = 0;
        studio_a = 0;
        warn_open = 1;
        warn_a = 1;
    }

    /* A transition curtain over the studio splash: no flakes are drawn above
     * black, and the epilepsy warning is never snowed over either. */
    studio_open = 1;
    studio_a = 1;
    t_fade = 0.5;
    t_dir = 1;
    snowflake_draws = 0;
    assert(newyear_snow_over_notice_active() == 0);
    assert(newyear_snow_under_notice_active() == 0);
    draw_newyear_over_notices();
    assert(snowflake_draws == 0);
    studio_open = 0;
    studio_a = 0;
    warn_open = 1;
    warn_a = 1;
    snowflake_draws = 0;
    draw_newyear_under_notices();
    assert(snowflake_draws == 0);
    t_fade = 0;
    t_dir = 0;

    /* Notice closed: flakes draw beneath the curtain and keep drawing through a fade. */
    warn_open = 0;
    assert(newyear_snow_under_notice_active() == 1 && newyear_snow_over_notice_active() == 0);
    snowflake_draws = 0;
    draw_newyear_under_notices();
    assert(snowflake_draws == (int)newyear_menu_flakes);
    t_fade = 0.5;
    t_dir = 1;
    snowflake_draws = 0;
    draw_newyear_under_notices();
    assert(snowflake_draws == (int)newyear_menu_flakes);
    t_fade = 0;
    t_dir = 0;

    /* The studio splash is an opaque notice too. */
    studio_open = 1;
    assert(newyear_snow_under_notice_active() == 0 && newyear_snow_over_notice_active() == 1);
    studio_open = 0;

    t_fade = 0;
    t_dir = 0;
    studio_open = 0;
    warn_open = 1;
    /* The jingle must take over even while the startup notice is visible; do
     * not let the ordinary lobby loop block it. */
    music_ok = 1;
    music_level = 1;
    lobby_playing = 1;
    winter_playing = 0;
    winter_loop_calls = 0;
    update_newyear_jingle();
    assert(lobby_playing == 0 && winter_playing == 1);
    assert(winter_loop_calls == 1 && music_level == 0);
    update_music();
    assert(lobby_playing == 0);
    winter_theme = 0;
    update_newyear_jingle();
    assert(winter_playing == 0);
    winter_theme = 1;
    warn_open = 0;

    /* Candies appear by fading in at full size, not by growing out of nothing. */
    {
        candy_enabled = 1;
        candy_tex_ok = 1;
        arr_set(candy_pick, 0, 0);
        arr_set(candy_t, 0, 0);
        candy_draws = 0;
        draw_candies();
        assert(candy_draws == 0);

        arr_set(candy_t, 0, candy_pop_time * 0.05);
        candy_alpha_drawn = -1;
        candy_scale_drawn = -1;
        draw_candies();
        assert(candy_draws == 1);
        /* scale приходит как float, сравниваем с допуском */
        assert(candy_scale_drawn > candy_scale - 1e-5 && candy_scale_drawn < candy_scale + 1e-5);
        assert(candy_alpha_drawn > 0 && candy_alpha_drawn < 0.2);

        arr_set(candy_t, 0, candy_pop_time * 0.5);
        candy_scale_drawn = -1;
        draw_candies();
        /* scale приходит как float, сравниваем с допуском */
        assert(candy_scale_drawn > candy_scale - 1e-5 && candy_scale_drawn < candy_scale + 1e-5);
        assert(candy_alpha_drawn > 0.4 && candy_alpha_drawn < 1);

        arr_set(candy_t, 0, candy_pop_time);
        candy_scale_drawn = -1;
        draw_candies();
        /* scale приходит как float, сравниваем с допуском */
        assert(candy_scale_drawn > candy_scale - 1e-5 && candy_scale_drawn < candy_scale + 1e-5);
        assert(candy_alpha_drawn > 0.99);

        /* Без текстуры - тот же альфа-канал в цвете круга. */
        candy_tex_ok = 0;
        arr_set(candy_t, 0, candy_pop_time * 0.05);
        candy_circle_color = 0;
        draw_candies();
        assert(((candy_circle_color >> 24) & 0xff) > 0 && ((candy_circle_color >> 24) & 0xff) < 60);
        assert((candy_circle_color & 0x00FFFFFF) == 0x00FF4081);
        arr_set(candy_t, 0, candy_pop_time);
        candy_circle_color = 0;
        draw_candies();
        assert(((candy_circle_color >> 24) & 0xff) > 250);
        candy_tex_ok = 0;
    }

    /* Punch boxes stay close to the body: 86 px long and 86 px wide. */
    assert(punch_reach == 86 && punch_width == 86);
    assert(enemy_punch_reach == 86 && enemy_punch_width == 86);

    /* The rarest achievement: a win on the last single HP, and only that. */
    {
        game_state = ST_SOLO;
        finished = 0;
        cups_awarded = 0;
        achievement_mask = 0;
        player->hp = 4;
        enemy->hp = 0;
        finish_game(1);
        assert(has_achievement(ACH_FIRST_WIN) == 1);
        assert(has_achievement(ACH_LAST_HP) == 0);
        finished = 0;
        cups_awarded = 0;
        achievement_mask = 0;
        player->hp = 1;
        finish_game(1);
        assert(has_achievement(ACH_FIRST_WIN) == 1);
        assert(has_achievement(ACH_LAST_HP) == 1);
        assert(achievement_count() == 4);
        assert(achievement_bit(3) == ACH_LAST_HP);
        finished = 0;
        cups_awarded = 0;
        achievement_mask = 0;
        player->hp = 10;
        enemy->hp = 10;
    }

    /* Leaving the game (game_bye, called when the activity pauses or stops):
     * half of the exits stay silent, the rest say one of the three farewells,
     * and the nick line needs a logged-in account. */
    {
        int i = 0;
        int silent = 0;
        int sorry = 0;
        int cubes = 0;
        int nick_line = 0;
        int empty_nick = 0;
        int other = 0;
        game_state = ST_LOBBY;
        warn_open = 0;
        studio_open = 0;
        t_dir = 0;
        t_fade = 0;
        language = 1;
        stub_login_status = 2;
        snprintf(stub_login_nick, sizeof(stub_login_nick), "%s", "Дима");
        for (i = 0; i < 4000; i++) {
            bye_calls = 0;
            last_bye[0] = 0;
            game_bye();
            if (bye_calls == 0) {
                silent++;
                continue;
            }
            assert(bye_calls == 1);
            if (strcmp(last_bye, "Нам жаль что вы вышли, удачи!") == 0) {
                sorry++;
            } else if (strcmp(last_bye, "Пока, кубы будут вас ждать!") == 0) {
                cubes++;
            } else if (strcmp(last_bye, "Пока, Дима, удачи!") == 0) {
                nick_line++;
            } else {
                other++;
            }
        }
        assert(other == 0);
        /* A 50/50 roll over 4000 exits, with a wide band for the random draw. */
        assert(silent > 1700 && silent < 2300);
        assert(sorry > 400 && cubes > 400 && nick_line > 400);

        /* Without an account the nick line is never picked: no "Пока, , удачи!". */
        stub_login_nick[0] = 0;
        empty_nick = 0;
        for (i = 0; i < 4000; i++) {
            bye_calls = 0;
            last_bye[0] = 0;
            game_bye();
            if (bye_calls == 1 && strstr(last_bye, ", , ") != NULL) {
                empty_nick++;
            }
        }
        assert(empty_nick == 0);

        /* The English wording follows the language setting. */
        language = 0;
        snprintf(stub_login_nick, sizeof(stub_login_nick), "%s", "Dima");
        other = 0;
        for (i = 0; i < 4000; i++) {
            bye_calls = 0;
            last_bye[0] = 0;
            game_bye();
            if (bye_calls == 0) {
                continue;
            }
            if (strcmp(last_bye, "Sorry that you left, good luck!") != 0 &&
                strcmp(last_bye, "Bye, the cubes will be waiting for you!") != 0 &&
                strcmp(last_bye, "Bye, Dima, good luck!") != 0) {
                other++;
            }
        }
        assert(other == 0);

        /* Back in the lobby closes the game, but the farewell itself is said by
         * the lifecycle hook (game_bye): one goodbye per exit, whichever way the
         * player leaves - back button, swipe-up gesture, swipe out of recents.
         * An ordinary back from a sub-screen only returns to the lobby. */
        language = 1;
        game_state = ST_LOBBY;
        bye_calls = 0;
        assert(logic_back() == 0);
        assert(bye_calls == 0);
        game_state = ST_SETTINGS;
        assert(logic_back() == 1);
        assert(bye_calls == 0);
        t_dir = 0;
        t_target = ST_LOBBY;
        game_state = ST_LOBBY;
    }

    assert(strcmp(SHOWDOWN_MUSIC, "astra_azum_showdown.wav") == 0);
    finished = 0;
    player->hp = enemy->hp = 10;
    game_state = ST_SOLO;
    player_class = CLASS_ASTRA;
    enemy_class = CLASS_AZUM;
    assert(showdown_music_matchup() == 1);
    player_class = CLASS_AZUM;
    enemy_class = CLASS_ASTRA;
    assert(showdown_music_matchup() == 1);
    enemy_class = CLASS_ORDINARY;
    assert(showdown_music_matchup() == 0);

    game_state = ST_ONLINE;
    online_ready = 1;
    stub_net_slot = 0;
    player_class = CLASS_AZUM;
    arr_set(remotes, 1 * remote_fields + 5, 1);
    arr_set(remotes, 1 * remote_fields + 10, CLASS_ASTRA);
    assert(showdown_music_matchup() == 1);
    arr_set(remotes, 2 * remote_fields + 5, 1);
    arr_set(remotes, 2 * remote_fields + 10, CLASS_ORDINARY);
    assert(showdown_music_matchup() == 0);
    arr_set(remotes, 2 * remote_fields + 5, 0);

    showdown_music_ok = 1;
    showdown_music_reset();
    showdown_play_calls = 0;
    update_showdown_music();
    assert(showdown_playing == 1 && showdown_play_calls == 1);
    showdown_playing = 0;
    update_showdown_music();
    assert(showdown_music_done == 1);
    update_showdown_music();
    assert(showdown_play_calls == 1);
    showdown_music_reset();
    /* Rework is its own developer-only class; accept the former pair only for
     * compatibility with clients that still send Astra + special skin. */
    assert(astra_rw_of(CLASS_ASTRA_REWORK, SKIN_NORMAL) == 1);
    assert(astra_rw_of(CLASS_ASTRA_REWORK, SKIN_SPECIAL) == 1);
    assert(astra_rw_of(CLASS_ASTRA, SKIN_SPECIAL) == 1);
    assert(astra_rw_of(CLASS_ASTRA, SKIN_NORMAL) == 0);
    assert(astra_rw_of(CLASS_AZUM, SKIN_SPECIAL) == 0);
    assert(admin_slot("DIMASI4EK229") == 1);
    assert(admin_slot("QWERTYUIOPAJ1234") == 2);
    assert(admin_slot("regular_player") == 0);
    stub_login_status = 2;
    snprintf(stub_login_nick, sizeof(stub_login_nick), "%s", ADMIN_NICK);
    snprintf(stub_login_pass, sizeof(stub_login_pass), "%s", ADMIN_PASS);
    astra_rw_refresh();
    assert(astra_rw_allowed() == 1);
    assert(astra_rw_is_dev() == 1);
    stub_login_pass[0] = 'x';
    stub_login_pass[1] = 0;
    astra_rw_refresh();
    assert(astra_rw_allowed() == 0);
    snprintf(stub_login_pass, sizeof(stub_login_pass), "%s", ADMIN_PASS);
    astra_rw_refresh();
    assert(astra_rw_allowed() == 1);
    snprintf(stub_login_nick, sizeof(stub_login_nick), "%s", ADMIN2_NICK);
    snprintf(stub_login_pass, sizeof(stub_login_pass), "%s", ADMIN2_PASS);
    astra_rw_refresh();
    assert(astra_rw_allowed() == 1 && astra_rw_is_dev() == 1);
    stub_login_pass[0] = 'x';
    astra_rw_refresh();
    assert(astra_rw_allowed() == 0);
    snprintf(stub_login_nick, sizeof(stub_login_nick), "%s", ADMIN_NICK);
    snprintf(stub_login_pass, sizeof(stub_login_pass), "%s", ADMIN_PASS);
    astra_rw_refresh();
    assert(astra_rw_allowed() == 1);

    player_class = CLASS_ASTRA_REWORK;
    player_level = 0;
    astra_skin = 0;
    assert(astra_rw_on() == 1 && astra_rw_local() == 1);
    assert(class_skin_of(CLASS_ASTRA_REWORK) == SKIN_NORMAL && current_skin() == SKIN_NORMAL);
    assert(class_owned_of(CLASS_ASTRA_REWORK) == 1 && class_visible(CLASS_ASTRA_REWORK) == 1);
    assert(strcmp(class_name_of(CLASS_ASTRA_REWORK), tr_class_astra_rework()) == 0);
    assert(strcmp(class_desc_of(CLASS_ASTRA_REWORK), "devdevdev") == 0);
    assert(strcmp(astra_rw_desc(), "devdevdev") == 0);
    assert(fabs(astra_rw_max_hp_for(CLASS_ASTRA_REWORK, 1) - astra_hp * 0.95) < 1e-9);
    assert(astra_rw_max_hp_for(CLASS_ASTRA_REWORK, 1) < astra_hp);
    assert(astra_rw_max_hp_for(CLASS_AZUM, 1) == azum_hp);
    assert(fabs(astra_rw_punch_damage_for(CLASS_ASTRA_REWORK, 0, 1) - punch_damage * 0.93) < 1e-9);
    assert(astra_rw_punch_damage_for(CLASS_ASTRA_REWORK, 0, 1) <
           astra_rw_punch_damage_for(CLASS_ASTRA_REWORK, 0, 0));
    assert(fabs(astra_rw_punch_damage_net(CLASS_ASTRA_REWORK, 0, SKIN_NORMAL) -
                astra_rw_punch_damage_for(CLASS_ASTRA_REWORK, 0, 1)) < 1e-9);
    assert(fabs(class_hp_of(CLASS_ASTRA) - astra_hp) < 1e-9);
    assert(fabs(class_hp_of(CLASS_ASTRA_REWORK) - astra_hp * 0.95) < 1e-9);
    assert(fabs(class_power_of(CLASS_ASTRA_REWORK) - punch_damage * 0.93) < 1e-9);
    classes_page = 0;
    screen_w = 320;
    assert(classes_page_size() == 1 && classes_page_count() == 6);
    assert(classes_page_start() == 0 && visible_class_at(5) == CLASS_ASTRA_REWORK);
    classes_page_next();
    assert(classes_page_start() == 1);
    classes_page_prev();
    assert(classes_page_start() == 0);
    screen_w = 600;
    assert(classes_page_size() == 3 && classes_page_count() == 2);
    classes_page_next();
    assert(classes_page_start() == 3 && classes_page_items() == 3);
    assert(visible_class_at(classes_page_start() + 2) == CLASS_ASTRA_REWORK);
    classes_page = 0;
    screen_w = 900;
    assert(classes_page_size() == 5 && classes_page_count() == 2);
    double swipe_y = classes_top() + class_card_h / 2;
    double swipe_x = classes_card_x(0) + classes_card_w() / 2;
    double selected_before_swipe = player_class;
    assert(classes_swipe_zone(swipe_x, swipe_y) == 1);
    touch_classes(swipe_x, swipe_y, 0, 7);
    touch_classes(swipe_x - 100, swipe_y, 2, 7);
    touch_classes(swipe_x - 100, swipe_y, 1, 7);
    assert(classes_page == 1 && player_class == selected_before_swipe);
    swipe_x = classes_card_x(classes_page_start()) + classes_card_w() / 2;
    touch_classes(swipe_x, swipe_y, 0, 7);
    touch_classes(swipe_x + 100, swipe_y, 2, 7);
    touch_classes(swipe_x + 100, swipe_y, 1, 7);
    assert(classes_page == 0 && player_class == selected_before_swipe);
    double had_rework_access = astra_rw_ok;
    astra_rw_ok = 0;
    assert(classes_visible_count() == 5 && classes_page_count() == 1);
    assert(classes_swipe_zone(swipe_x, swipe_y) == 0);
    astra_rw_ok = had_rework_access;
    classes_page = 0;
    screen_w = 320;
    player_class = CLASS_ASTRA;
    astra_skin = 1;
    progress_normalize();
    assert(player_class == CLASS_ASTRA_REWORK && astra_skin == 0);
    assert(astra_rw_hit_interval_for(1) < astra_rw_hit_interval_for(0));
    /* the first three beats land sooner... */
    assert(astra_rw_beat_time_for(1, 1) < astra_rw_beat_time_for(1, 0));
    assert(astra_rw_beat_time_for(3, 1) < astra_rw_beat_time_for(3, 0));
    assert(astra_rw_beats_done_for(astra_rw_beat_time_for(3, 1), 1) == 3);
    /* ...and then the throw waits only ~0.4 s after the last beat, so the
     * whole ultimate reads as fast hits, a short pause, the throw. */
    assert(fabs(astra_rw_final_delay_for(1) - 0.4) < 1e-9);
    assert(fabs(astra_rw_final_time_for(1) - astra_rw_beat_time_for(3, 1) - 0.4) < 1e-9);
    assert(astra_rw_final_time_for(1) < astra_rw_final_time_for(0));
    assert(astra_rw_grab_time_for(1) < astra_rw_grab_time_for(0));
    assert(fabs(astra_rw_throw_dist_for(0, 1) - astra_throw_dist_for(0) * 1.10) < 1e-9);
    assert(astra_rw_throw_dist_for(0, 1) < astra_throw_dist_for(0) * 1.35);
    assert(astra_rw_throw_stun_for(0, 1) == astra_rw_throw_stun_for(0, 0));
    assert(astra_rw_tint() != 0 && astra_rw_tint_local() == astra_rw_tint());
    /* pressing the ultimate takes a small forward step: the grab origin is
     * the end of the step, while the body slides there smoothly frame by
     * frame instead of jumping in one go */
    player->angle = 0;
    player->size = 45;
    player->x = 150;
    player->y = 90;
    astra_punch_t = 0.05;
    start_grab_now();
    assert(astra_punch_t == 0);
    assert(fabs(player->x - 150) < 1e-9);
    assert(astra_x > 150 && astra_x - 150 <= astra_rw_lunge_dist + 1e-9);
    assert(fabs(astra_y - 90) < 1e-9);
    double lunge_prev = player->x;
    astra_rw_lunge_tick();
    assert(player->x > lunge_prev && player->x < astra_x);
    lunge_prev = player->x;
    int lunge_frames = 1;
    while (lunge_frames < 60 && player->x < astra_x) {
        astra_rw_lunge_tick();
        assert(player->x >= lunge_prev);
        lunge_prev = player->x;
        lunge_frames = lunge_frames + 1;
    }
    assert(fabs(player->x - astra_x) < 1e-9);
    assert(fabs(player->y - astra_y) < 1e-9);
    assert(lunge_frames * dt >= astra_rw_lunge_time);
    assert(astra_rw_zone_live_for(astra_rw_lunge_time / 2, 0, 1) == 0);
    assert(astra_rw_zone_live_for(astra_rw_lunge_time, 0, 1) == 1);
    astra_state = 0;
    astra_cd = 0;
    /* The grab/catch waits for the visual dash to finish; its first hit is
     * still scheduled before regular Astra's first beat. */
    player->x = 100;
    player->y = 90;
    player->hp = 10;
    enemy->hp = enemy->max_hp = 10;
    enemy_revive_prot = 0;
    enemy->y = player->y;
    game_state = ST_SOLO;
    start_grab_now();
    enemy->x = astra_x + 70;
    for (int frame = 0; frame < 9; frame++) {
        tick_astra();
        assert(astra_caught == 0 && astra_hits_done == 0);
    }
    assert(astra_caught == 0 && enemy->hp == 10);
    tick_astra();
    assert(astra_caught == 1 && astra_hits_done == 0);
    for (int frame = 10; frame < 22; frame++)
        tick_astra();
    assert(astra_hits_done == 0);
    tick_astra();
    assert(astra_hits_done == 1 && astra_punch_t > 0);
    astra_release(0);
    astra_cd = 0;
    /* Rework is no longer an Astra skin: both class cards use a cube icon,
     * while only the separate Rework card receives the developer tuning. */
    player_class = CLASS_ASTRA_REWORK;
    assert(class_has_skins(CLASS_ASTRA) == 0 && class_has_skins(CLASS_ASTRA_REWORK) == 0);
    assert(skins_btn_visible() == 0 && current_skin() == SKIN_NORMAL);
    pick_skin(SKIN_SPECIAL);
    assert(astra_skin == 0 && astra_rw_local() == 1 && current_skin() == SKIN_NORMAL);
    assert(strcmp(class_card_tex(CLASS_ASTRA_REWORK, SKIN_NORMAL), ORDINARY_CARD_TEX) == 0);
    assert(strcmp(class_card_tex(CLASS_ORDINARY, SKIN_NORMAL), ORDINARY_CARD_TEX) == 0);
    assert(fabs(class_hp_of(CLASS_ASTRA) - astra_hp) < 1e-9);
    assert(fabs(class_hp_of(CLASS_ASTRA_REWORK) - astra_hp * 0.95) < 1e-9);
    /* The classes screen rates imbalance in stars, with Rework separate from
     * regular Astra and never outside the 0..5 scale. */
    assert(class_imbalance_of(CLASS_ORDINARY) == 1);
    assert(class_imbalance_of(CLASS_SANTA) == 2);
    assert(class_imbalance_of(CLASS_AZUM) == 3);
    assert(class_imbalance_of(CLASS_EBUC) == 3);
    assert(class_imbalance_of(CLASS_ASTRA) == 4);
    assert(class_imbalance_of(CLASS_ASTRA_REWORK) == 5);
    double star_cls = 0;
    while (star_cls < class_count) {
        assert(class_imbalance_of(star_cls) >= 0 && class_imbalance_of(star_cls) <= 5);
        star_cls = star_cls + 1;
    }
    /* no developer account, no rework: same save file cannot enable it */
    stub_login_nick[0] = 'x';
    stub_login_nick[1] = 0;
    astra_rw_refresh();
    assert(astra_rw_allowed() == 0 && astra_rw_local() == 0);
    assert(class_owned_of(CLASS_ASTRA_REWORK) == 0 && class_visible(CLASS_ASTRA_REWORK) == 0);
    assert(class_visible_index(CLASS_ASTRA_REWORK) == -1 && classes_visible_count() == 5);
    assert(classes_page_count() == 5);
    assert(class_skin_of(CLASS_ASTRA_REWORK) == SKIN_NORMAL && current_skin() == SKIN_NORMAL);
    assert(astra_rw_tint_local() == 0);
    assert(class_imbalance_of(CLASS_ASTRA) == 4);
    progress_normalize();
    assert(player_class == CLASS_ORDINARY);
    player->angle = 0;
    player->x = 150;
    player->y = 90;
    double plain_x = player->x;
    start_grab_now();
    assert(fabs(player->x - plain_x) < 1e-9);
    astra_state = 0;
    astra_cd = 0;
    player_class = CLASS_ORDINARY;
    astra_skin = 0;

    stub_net_slot = -1;
    online_ready = 0;

    player_class = CLASS_ASTRA;
    player_level = 0;
    game_state = ST_SOLO;
    finished = 0;
    player->x = 400;
    player->y = 400;
    player->angle = 0;
    player->hp = player->max_hp = 10;
    enemy_class = CLASS_ORDINARY;
    enemy->x = 470;
    enemy->y = 400;
    enemy->hp = enemy->max_hp = 10;
    astra_cd = 0;
    start_grab_now();
    for (int frame = 0; frame < 400 && astra_state != 0; frame++)
        tick_astra();
    assert(fabs(enemy->hp - 5.5) < 1e-6);
    assert(enemy_throw_t > 0);

    astra_state = 0;
    enemy_throw_t = 0;
    player_class = CLASS_ORDINARY;
    game_state = ST_SOLO;
    finished = 0;
    player->x = 470;
    player->y = 400;
    player->hp = player->max_hp = 10;
    enemy_class = CLASS_ASTRA;
    enemy_level = 0;
    enemy->x = 400;
    enemy->y = 400;
    enemy->angle = 0;
    enemy->hp = enemy->max_hp = 10;
    pgrab_active = 0;
    pthrow_t = 0;
    enemy_start_grab();
    for (int frame = 0; frame < 400 && egrab_state != 0; frame++)
        tick_enemy_grab();
    assert(fabs(player->hp - 5.5) < 1e-6);
    assert(pthrow_t > 0);

    egrab_state = 0;
    pgrab_active = 0;
    pthrow_t = 0;
    game_state = ST_ONLINE;
    finished = 0;
    player->x = 470;
    player->y = 400;
    player->hp = player->max_hp = 10;
    stub_net_slot = 0;
    stub_online[1] = 1;
    stub_level[1] = 0;
    stub_gx[1] = 400.0 / screen_w;
    stub_gy[1] = 400.0 / screen_h;
    stub_gdx[1] = 1;
    stub_gdy[1] = 0;
    stub_grab[1] = 1;
    for (int frame = 0; frame < 400; frame++)
        update_remote_grabs();
    assert(fabs(player->hp - 5.5) < 1e-6);
    assert(pthrow_t > 0);
    double plain_throw = pthrow_speed * astra_throw_time;

    /* The same grab from an Astra (Rework) attacker: the variant is read from
     * the class-ID/legacy-pair snapshot, the three hits land sooner, the throw
     * waits only ~0.4 s after the last beat and goes farther; the total
     * damage of the grab is unchanged. */
    pgrab_active = 0;
    pgrab_slot = -1;
    pthrow_t = 0;
    pgrab_hits = 0;
    finished = 0;
    player->x = 470;
    player->y = 400;
    player->hp = player->max_hp = 10;
    arr_set(remotes, 1 * remote_fields + 10, CLASS_ASTRA);
    arr_set(remotes, 1 * remote_fields + 11, SKIN_SPECIAL);
    stub_grab[1] = 2;
    for (int frame = 0; frame < 9; frame++)
        update_remote_grabs();
    assert(pgrab_active == 0 && pgrab_hits == 0);
    for (int frame = 9; frame < 45; frame++)
        update_remote_grabs();
    assert(pgrab_hits == 3);
    assert(fabs((double)pgrab_t - 45 * dt) < 1e-9);
    assert(astra_rw_beat_time_for(3, 1) < astra_rw_beat_time_for(3, 0));
    assert(45 * dt > astra_rw_beat_time_for(3, 1) && 45 * dt < astra_rw_beat_time_for(3, 0) + 1e-9);
    for (int frame = 0; frame < 400 && pthrow_t <= 0; frame++)
        update_remote_grabs();
    assert(fabs(player->hp - 5.5) < 1e-6);
    assert(pthrow_t > 0);
    assert(pthrow_speed * astra_throw_time > plain_throw);
    assert(fabs(pthrow_speed * astra_throw_time - plain_throw * 1.10) < 1e-6);
    pgrab_active = 0;
    pgrab_slot = -1;
    pthrow_t = 0;
    stub_grab[1] = 0;
    arr_set(remotes, 1 * remote_fields + 10, CLASS_ORDINARY);
    arr_set(remotes, 1 * remote_fields + 11, SKIN_NORMAL);
    stub_net_slot = -1;

    player->hp = 1;
    game_state = ST_ONLINE;
    cups = 999;
    game_reset();
    assert(player->hp == 10);
    assert(game_state == ST_LOBBY);
    assert(cups == 0);

    state_destroy();
    state_ready = 0;
    puts("Состояние полной C-версии: норма");
    return 0;
}
