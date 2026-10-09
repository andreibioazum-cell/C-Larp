static void draw_candies(void) {
    if (candy_enabled == 0) {
        return;
    }
    double i = 0;
    double cp = 0;
    double p = 0;
    double pop = 0;
    double sc = 0;
    double hw = 0;
    while (i < candy_count) {
        cp = arr_get(candy_pick, i);
        if (cp > 0) {
        } else {
            p = clamp(arr_get(candy_t, i) / candy_pop_time, 0, 1);
            pop = 1 - (1 - p) * (1 - p);
            sc = candy_scale * pop;
            if (sc > 0.02) {
                double ang = arr_get(candy_ang, i);
                double travel = (1 - pop) * candy_fly_dist;
                double cx = arr_get(candy_x, i) - cos(ang) * travel;
                double cy = arr_get(candy_y, i) - sin(ang) * travel;
                hw = 25 * sc;
                if (candy_tex_ok == 1) {
                    tex(cx - hw, cy - hw, CANDY_TEX, ang, sc);
                } else {
                    circle(cx, cy, hw, 0xFFFF4081);
                }
            }
        }
        i = i + 1;
    }
}

static void draw_status_aura(double x, double y, double size, double rgb, double rim, double a) {
    if (a <= 0) {
        return;
    }
    double fill = floor(a * 255 * 0.3);
    if (fill <= 0) {
        return;
    }
    double half = size + 14;
    circle(x, y, half, (((unsigned)(int)fill) << 24) | ((unsigned)(int)rgb));
    /* pack_c() maps alpha 0 to opaque, so a ring whose alpha has faded to
     * zero must be skipped instead of flashing opaque for a frame. */
    double ring_a = floor(a * 255 * 0.7);
    if (ring_a >= 1) {
        ring(x, y, half, 2, (((unsigned)(int)ring_a) << 24) | ((unsigned)(int)rim));
    }
}

static void draw_remote_snow(void) {
    double s = 0;
    while (s < max_players) {
        double b = s * snow_fields;
        if (arr_get(remote_snow, b + 1) == 1) {
            double x = arr_get(remote_snow, b + 3);
            double y = arr_get(remote_snow, b + 4);
            double t = arr_get(remote_snow, b + 2);
            double hw = 16;
            tex(x - hw, y - hw, SNOWFLAKE_TEX, t * 3, 0.5);
        }
        if (arr_get(remote_snow, b + 5) > 0) {
            draw_boom_at(arr_get(remote_snow, b + 6), arr_get(remote_snow, b + 7), arr_get(remote_snow, b + 5));
        }
        s = s + 1;
    }
}

static void finish_game(double result) {
    finished = result;
    aim_id = -1;
    super_id = -1;
    punch->active = 0;
    punch->hit = 0;
    punch_left = 0;
    super_windup = 0;
    santa_follow = 0;
    drop_joy();
    gift->active = 0;
    card_active = 0;
    boom_t = 0;
    dash_active = 0;
    dash_t = 0;
    dash_hit = 0;
    dash_pub = 0;
    enemy_gift->active = 0;
    enemy_boom_t = 0;
    enemy_dash_active = 0;
    enemy_dash_t = 0;
    enemy_dash_hit = 0;
    enemy_super_windup = 0;
    enemy_super_kind = 0;
    enemy_grab_clear();
    turret_heal_t = 0;
    universe_active = 0;
    universe_a = 0;
    keyboard_hide();
    double i = 0;
    while (i < max_players) {
        double sb = i * snow_fields;
        arr_set(remote_snow, sb, 0);
        arr_set(remote_snow, sb + 1, 0);
        arr_set(remote_snow, sb + 5, 0);
        double tb = i * station_fields;
        double k = 0;
        while (k < turret_max) {
            arr_set(remote_station, tb + 1 + k * 5, 0);
            k = k + 1;
        }
        double ub = i * universe_fields;
        arr_set(remote_universe, ub, 0);
        arr_set(remote_universe, ub + 1, 0);
        remote_dash_set(i, DASH_GEN, 0);
        remote_dash_set(i, DASH_ACTIVE, 0);
        clear_remote_punch(i);
        arr_set(rpunch_a, i, 0);
        i = i + 1;
    }
    if (result == 1 && cups_awarded == 0) {
        cups_awarded = 1;
        cups = cups + cup_win;
        if (candy_enabled == 1) {
            candies = candies + candy_win;
        }
        save_progress();
        grant_achievement(ACH_FIRST_WIN);
        ds_log("win +%g cups +%g candies", cup_win, candy_win);
    }
    if (result == 2 && cups_awarded == 0) {
        cups_awarded = 1;
        cups = cups - cup_lose;
        if (cups < 0) {
            cups = 0;
        }
        save_progress();
        ds_log("lose -%g cups", cup_lose);
    }
}

static void draw_punch_box(double x, double y, double dx, double dy, double reach, double width, double alpha) {
    draw_hit_strip(x, y, dx, dy, punch_forward_offset, punch_forward_offset + reach, width / 2, alpha);
}

static void draw_field(void) { rect(0, 0, screen_w, screen_h, 0x33000000); }

static double player_punching_now(void) {
    if (punch_left > 0) {
        return 1;
    }
    if (astra_punch_t > 0) {
        return 1;
    }
    return 0;
}

static void draw_player_shadow_layer(void) {
    draw_fighter_shadow(player->x, player->y, player->angle, fighter_look_of(FLOOK_ME),
                        fighter_pose_of(FLOOK_ME, player_punching_now()));
}

static void draw_player_hitbox_layer(void) {
    if (show_hitboxes == 0) {
        return;
    }
    if (aim_a > 0) {
        draw_punch_box(punch->x, punch->y, punch->dx, punch->dy, punch_reach, punch_width, aim_a);
    }
}

static void draw_player_body_layer(void) {
    double punching = fighter_pose_of(FLOOK_ME, player_punching_now());
    double look = fighter_look_of(FLOOK_ME);
    draw_fighter_body(player->x, player->y, player->angle, look, punching);
    draw_fighter_tint(player->x, player->y, player->angle, look, punching, player_scale, astra_rw_tint_local());
    draw_fighter_hurt(player->x, player->y, player->angle, look, punching, player_scale, player_hurt_a);
}

static double enemy_punching_now(void) {
    if (enemy->state == 1 || enemy->state == 2) {
        return 1;
    }
    if (enemy_super_windup > 0) {
        return 1;
    }
    if (enemy_grab_punching() == 1) {
        return 1;
    }
    return 0;
}

static GAME_MAYBE_UNUSED const char *enemy_battle_tex(void) {
    return look_tex(fighter_look_of(FLOOK_BOT), fighter_pose_of(FLOOK_BOT, enemy_punching_now()));
}

static GAME_MAYBE_UNUSED double enemy_battle_tex_visible(void) {
    if (fighter_look_of(FLOOK_BOT) == LOOK_NONE) {
        return 0;
    }
    return 1;
}

static void draw_enemy_shadow(void) {
    double ehw = 25 * cube_scale * enemy_scale;
    double es = cube_scale * enemy_scale;
    double look = fighter_look_of(FLOOK_BOT);
    if (look == LOOK_NONE) {
        circle(enemy->x - shadow_off, enemy->y + shadow_off, ehw, (((unsigned)(int)shadow_alpha) << 24));
        return;
    }
    tex_tint(enemy->x - ehw - shadow_off, enemy->y - ehw + shadow_off,
             look_tex(look, fighter_pose_of(FLOOK_BOT, enemy_punching_now())), sprite_a(enemy->angle), es,
             (((unsigned)(int)shadow_alpha) << 24));
}

static void draw_enemy_hitbox(void) {
    if (show_hitboxes == 0) {
        return;
    }
    if (enemy_punch_a > 0) {
        draw_punch_box(enemy_punch_x, enemy_punch_y, enemy_punch_dx, enemy_punch_dy, enemy_punch_reach,
                       enemy_punch_width, enemy_punch_a);
    }
}

static void draw_enemy_body(void) {
    double ehw = 25 * cube_scale * enemy_scale;
    double es = cube_scale * enemy_scale;
    double punching = fighter_pose_of(FLOOK_BOT, enemy_punching_now());
    double look = fighter_look_of(FLOOK_BOT);
    if (look == LOOK_NONE) {
        rect(enemy->x - ehw, enemy->y - ehw, 2 * ehw, 2 * ehw, fighter_fallback_rgb);
    } else {
        tex(enemy->x - ehw, enemy->y - ehw, look_tex(look, punching), sprite_a(enemy->angle), es);
    }
    draw_fighter_hurt(enemy->x, enemy->y, enemy->angle, look, punching, enemy_scale, enemy_hurt_a);
}

static void draw_controls(void) {
    ring(joy.x, joy.y, joy.r, 4, 0x000000);
    circle(joy.x + joy.ox, joy.y + joy.oy, 34, 0x000000);
    circle(atk_x, atk_y, 70, 0xE94560);
    ring(atk_x, atk_y, 70, 4, 0x000000);
    text_in_box(tr_punch(), atk_x - 70, atk_y - 70, 140, 140, 0xFFFFFF, 0.55);
    if (class_has_super(player_class) == 1 && (game_state == ST_SOLO || game_state == ST_ONLINE)) {
        circle(atk2_x, atk2_y, 70, 0x5F10A0);
        ring(atk2_x, atk2_y, 70, 4, 0x000000);
        const char *sl = tr_super();
        if (player_class == CLASS_EBUC) {
            sl = tr_super_ebuc();
        } else if (player_class == CLASS_AZUM) {
            sl = tr_super_azum();
        } else if (class_is_astra(player_class) == 1) {
            sl = tr_super_astra();
        }
        double sls = 0.55;
        double left = super_cd;
        if (player_class == CLASS_AZUM) {
            left = dash_cd;
        } else if (class_is_astra(player_class) == 1) {
            left = astra_cd;
        }
        if (left > 0) {
            sl = ds_concat(ds_num_to_string((double)(floor(left + 0.999))), "");
            sls = 0.95;
        }
        text_in_box(sl, atk2_x - 70, atk2_y - 70, 140, 140, 0xFFFFFF, sls);
    }
    draw_back();
}

static void draw_result(const char *title) {
    rect(0, 0, screen_w, screen_h, 0x99000000);
    double w = 430;
    double h = 250;
    double x = (screen_w - w) / 2;
    double y = screen_h / 2 - h / 2;
    roundrect(x, y, w, h, 30, 0x5F10A0);
    double reward = 0;
    if (finished == 1) {
        reward = cup_win;
    } else if (finished == 2) {
        reward = 0 - cup_lose;
    }
    ctext_scaled(title, y + 36, 0xFFFFFF, 1.12);
    if (finished == 2) {
        ctext_scaled(tr_cups_lose(reward), y + 126, 0xFFFFFF, 0.9);
    } else {
        ctext_scaled(tr_cups_reward(reward), y + 126, 0xFFFFFF, 0.9);
    }
    ctext_scaled(tr_tap_to_return(), y + 188, 0xFFFFFF, 0.62);
}

static void draw_fps_meter(void) {
    if (show_fps == 0) {
        return;
    }
    const char *label = ds_concat("FPS ", ds_num_to_string((double)(floor(fps_value + 0.5))));
    double sc = fps_text_scale;
    double tw = text_ink_width(label) * sc;
    double th = text_ink_height(label) * sc;
    double tt = text_ink_top(label) * sc;
    double w = tw + 28;
    double h = 36;
    double x = (screen_w - w) / 2;
    double y = screen_h - h - 16;
    roundrect(x - 2, y - 2, w + 4, h + 4, h / 2 + 2, fps_frame_rgb);
    roundrect(x, y, w, h, h / 2, fps_plate_rgb);
    text_scaled(label, x + (w - tw) / 2, y + (h - th) / 2 - tt + 1.5, fps_text_rgb, sc);
}

static void draw_game(void) {
    resolve_fighter_looks();
    draw_arena_background();
    draw_field();
    draw_dust();
    draw_player_shadow_layer();
    draw_enemy_shadow();
    draw_turret_shadows();
    draw_candies();
    draw_card_item();
    draw_player_hitbox_layer();
    draw_enemy_hitbox();
    draw_ability_hitboxes();
    draw_gift();
    draw_boom();
    draw_enemy_gift();
    draw_enemy_boom();
    draw_universe();
    draw_enemy_turrets();
    draw_player_body_layer();
    draw_status_aura(player->x, player->y, player->size, freeze_rgb, freeze_rim, pfreeze_a);
    draw_status_aura(player->x, player->y, player->size, poison_rgb, poison_rim, ppoison_a);
    draw_status_aura(player->x, player->y, player->size, stun_rgb, stun_rim, pstun_a);
    draw_station();
    draw_enemy_body();
    draw_status_aura(enemy->x, enemy->y, enemy->size, freeze_rgb, freeze_rim, freeze_a);
    draw_status_aura(enemy->x, enemy->y, enemy->size, poison_rgb, poison_rim, poison_a);
    draw_status_aura(enemy->x, enemy->y, enemy->size, stun_rgb, stun_rim, stun_a);
    draw_snow();
    double w = screen_w / 4;
    hud_bar(20, 20, w, 28, player->hp, player->max_hp, 0x7ED321, 0);
    hud_bar(screen_w - w - 20, 20, w, 28, enemy->hp, enemy->max_hp, 0xE94560, 1);
    draw_controls();
    draw_fps_meter();
    if (finished == 1) {
        draw_result(tr_victory());
    } else if (finished == 2) {
        draw_result(tr_defeat());
    }
    draw_card_open();
}
