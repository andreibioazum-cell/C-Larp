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
void keyboard_hide(void) {}
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
int snd_playing(const char *name) {
    return name && SHOWDOWN_MUSIC && strcmp(name, SHOWDOWN_MUSIC) == 0 ? showdown_playing : 0;
}
void snd_stop(const char *name) {
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

void tex(float x, float y, const char *name, float angle, float scale) {
    (void)x;
    (void)y;
    (void)angle;
    (void)scale;
    ground_tiles++;
    last_ground = name;
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

    assert(class_count == 5 && CLASS_ASTRA == 4);
    assert(punch_forward_offset == 19 && astra_grab_forward_offset == 19);
    assert(fabs(astra_hit_interval - 0.2) < 1e-9);
    assert(fabs(astra_final_delay - 0.85) < 1e-9);
    assert(fabs(astra_final_time() - astra_beat_time(astra_hit_count) - astra_final_delay) < 1e-9);
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
    assert(remote_fields == 12);

    assert(rects_overlap(0, 0, 1, 0, 10, 10, 15, 0, 1, 0, 10, 10) == 1);
    assert(rects_overlap(0, 0, 1, 0, 10, 10, 25, 0, 1, 0, 10, 10) == 0);
    assert(circle_hits_box(0, 0, 5, 8, 0, 0, 4) == 1);
    assert(circle_hits_box(0, 0, 2, 8, 0, 0, 4) == 0);

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
    warn_open = studio_open = 0;
    newyear_jingle_ok = 1;
    assert(snow_active() == 0);
    assert(newyear_menu_active() == 1);
    assert(newyear_music_active() == 1);

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
