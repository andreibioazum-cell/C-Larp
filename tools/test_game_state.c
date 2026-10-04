#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game/game.c"

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
    while (array->length <= position) arr_push(array, 0);
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

    assert(class_cost_of(CLASS_AZUM) == 65);
    assert(class_cost_of(CLASS_SANTA) == 100);
    assert(class_cost_of(CLASS_EBUC) == 120);
    assert(strcmp(fighter_sprite(CLASS_ORDINARY, 0, SKIN_NORMAL), ORDINARY_TEX) == 0);
    azum_tex_ok = azum_punch_tex_ok = 1;
    azum_zombie_tex_ok = azum_zombie_punch_tex_ok = 1;
    assert(strcmp(fighter_sprite(CLASS_AZUM, 1, SKIN_NORMAL), AZUM_PUNCH_TEX) == 0);
    assert(strcmp(fighter_sprite(CLASS_AZUM, 0, SKIN_ZOMBIE), AZUM_ZOMBIE_TEX) == 0);

    assert(rects_overlap(0, 0, 1, 0, 10, 10, 15, 0, 1, 0, 10, 10) == 1);
    assert(rects_overlap(0, 0, 1, 0, 10, 10, 25, 0, 1, 0, 10, 10) == 0);
    assert(circle_hits_box(0, 0, 5, 8, 0, 0, 4) == 1);
    assert(circle_hits_box(0, 0, 2, 8, 0, 0, 4) == 0);

    winter_theme = snow_tex_ok = 1;
    game_state = ST_SOLO;
    assert(strcmp(arena_ground_tex(), SNOW_TEX) == 0);
    assert(snow_active() == 1);
    game_state = ST_LOBBY;
    assert(snow_active() == 0);

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
