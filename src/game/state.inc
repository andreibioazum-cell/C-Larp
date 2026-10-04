#include "state/globals/core/config.inc"
#include "state/globals/core/entities.inc"
#include "state/globals/ui/progress_classes.inc"
#include "state/globals/ui/promo.inc"
#include "state/globals/ui/layout.inc"
#include "state/globals/ui/chat.inc"
#include "state/globals/combat/battle_turrets.inc"
#include "state/globals/combat/battle_enemy_turrets.inc"
#include "state/globals/combat/battle_event_plates.inc"
#include "state/globals/fx/weather.inc"
#include "state/globals/ui/chat_bubbles.inc"

#include "objects.inc"

static void state_create(void) {
#include "state/create/core/config.inc"
#include "state/create/core/entities.inc"
#include "state/create/ui/progress_classes.inc"
#include "state/create/ui/promo.inc"
#include "state/create/ui/layout.inc"
#include "state/create/ui/chat.inc"
#include "state/create/combat/battle_turrets.inc"
#include "state/create/combat/battle_enemy_turrets.inc"
#include "state/create/combat/battle_event_plates.inc"
#include "state/create/fx/weather.inc"
#include "state/create/ui/chat_bubbles.inc"
}

static void state_destroy(void) {
    if (quest_type)
        arr_free(quest_type);
    if (quest_prog)
        arr_free(quest_prog);
    if (quest_need)
        arr_free(quest_need);
    if (quest_next)
        arr_free(quest_next);
    if (player)
        player_free(player);
    if (enemy)
        enemy_free(enemy);
    if (punch)
        punch_free(punch);
    if (gift)
        gift_free(gift);
    if (enemy_gift)
        gift_free(enemy_gift);
    if (enemy_turret_x)
        arr_free(enemy_turret_x);
    if (enemy_turret_y)
        arr_free(enemy_turret_y);
    if (enemy_turret_hp)
        arr_free(enemy_turret_hp);
    if (enemy_turret_maxhp)
        arr_free(enemy_turret_maxhp);
    if (enemy_turret_spawn_t)
        arr_free(enemy_turret_spawn_t);
    if (rpunch_a)
        arr_free(rpunch_a);
    if (station_boom_xs)
        arr_free(station_boom_xs);
    if (station_boom_ys)
        arr_free(station_boom_ys);
    if (station_boom_ts)
        arr_free(station_boom_ts);
    if (turret_x)
        arr_free(turret_x);
    if (turret_y)
        arr_free(turret_y);
    if (turret_hp)
        arr_free(turret_hp);
    if (turret_maxhp)
        arr_free(turret_maxhp);
    if (turret_spawn_t)
        arr_free(turret_spawn_t);
    if (runiverse_fx_a)
        arr_free(runiverse_fx_a);
    if (remote_station)
        arr_free(remote_station);
    if (remote_universe)
        arr_free(remote_universe);
    if (remotes)
        arr_free(remotes);
    if (remote_punches)
        arr_free(remote_punches);
    if (remote_snow)
        arr_free(remote_snow);
    if (remote_dash)
        arr_free(remote_dash);
    if (rdash_box_a)
        arr_free(rdash_box_a);
    if (rsnow_ball_a)
        arr_free(rsnow_ball_a);
    if (rsnow_boom_a)
        arr_free(rsnow_boom_a);
    if (runiverse_box_a)
        arr_free(runiverse_box_a);
    if (rstation_box_a)
        arr_free(rstation_box_a);
    if (turret_box_a)
        arr_free(turret_box_a);
    if (enemy_turret_box_a)
        arr_free(enemy_turret_box_a);
    if (remote_thuds)
        arr_free(remote_thuds);
    if (candy_x)
        arr_free(candy_x);
    if (candy_y)
        arr_free(candy_y);
    if (candy_t)
        arr_free(candy_t);
    if (candy_pick)
        arr_free(candy_pick);
    if (candy_ang)
        arr_free(candy_ang);
    if (dust)
        arr_free(dust);
    if (remote_dust_cd)
        arr_free(remote_dust_cd);
    if (class_level_tbl)
        arr_free(class_level_tbl);
    if (class_unlocked_tbl)
        arr_free(class_unlocked_tbl);
    if (class_owned_tbl)
        arr_free(class_owned_tbl);
    if (plates_candies)
        arr_free(plates_candies);
    if (flake_cache)
        arr_free(flake_cache);
}
