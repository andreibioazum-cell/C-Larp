static double disco_color(double phase) {
    double r = 127 + 127 * sin(phase);
    double g = 127 + 127 * sin(phase + 2.094);
    double b = 127 + 127 * sin(phase + 4.188);
    double a = 55 + 90 * (0.5 + 0.5 * sin(disco_t * 16));
    return ((((unsigned)(int)a) << 24) | (((unsigned)(int)r) << 16) | (((unsigned)(int)g) << 8) | ((unsigned)(int)b));
}

static void draw_disco(void) {
    if (event_mode != 1) {
        return;
    }
    double col = disco_color(disco_t * 18);
    rect(0, 0, screen_w, screen_h, col);
    double ct = disco_t * 12;
    double r = 110 + 40 * sin(ct);
    ring(screen_w / 2, screen_h / 2, r, 7, 0xC0FFFFFF);
}

static void update_event(void) {
    event_mode = net_event();
    if (event_mode == 3 && candy_enabled == 0) {
        event_mode = 0;
    }
    event_t = event_t + dt;
    if (event_t > 100000) {
        event_t = 0;
    }
    if (event_mode == 1) {
        disco_t = disco_t + dt;
    } else {
        disco_t = 0;
    }
    tick_plates_event();
}

static double banner_named_color(const char *name) {
    if (str_eq(name, "red") == 1) {
        return 0x00FF5252;
    }
    if (str_eq(name, "green") == 1) {
        return 0x0000E676;
    }
    if (str_eq(name, "blue") == 1) {
        return 0x004FC3F7;
    }
    if (str_eq(name, "yellow") == 1) {
        return 0x00FFD54A;
    }
    if (str_eq(name, "orange") == 1) {
        return 0x00FF9800;
    }
    if (str_eq(name, "purple") == 1) {
        return 0x00AB47BC;
    }
    if (str_eq(name, "pink") == 1) {
        return 0x00FF80AB;
    }
    if (str_eq(name, "cyan") == 1) {
        return 0x0000E5FF;
    }
    if (str_eq(name, "black") == 1) {
        return 0x00222222;
    }
    return 0x00FFFFFF;
}

static double banner_color_now(const char *name) {
    return banner_named_color(name);
}

static void update_banner(void) {
    if (game_state != ST_ONLINE) {
        return;
    }
    double ts = net_banner_ts();
    if (ts > 0) {
        if (banner_skip == 1) {
            banner_seen_ts = ts;
            banner_skip = 0;
        } else if (ts != banner_seen_ts) {
            banner_seen_ts = ts;
            banner_t = banner_show_time;
        }
    }
    if (banner_t > 0) {
        banner_t = tick_down(banner_t);
    }
}

static void draw_banner(void) {
    if (banner_t <= 0) {
        return;
    }
    const char *txt = net_banner_text();
    if (str_len(txt) == 0) {
        return;
    }
    double elapsed = banner_show_time - banner_t;
    double a = elapsed;
    if (a > banner_t) {
        a = banner_t;
    }
    double alpha = floor(clamp(a / banner_fade, 0, 1) * 230);
    if (alpha <= 0) {
        return;
    }
    double sc = 1.05;
    double w = text_ink_width(txt) * sc;
    if (w > screen_w - 40) {
        sc = (screen_w - 40) / w;
        w = text_ink_width(txt) * sc;
    }
    double x = (screen_w - w) / 2;
    double y = 118;
    uint32_t col = banner_color_now(net_banner_color());
    text_scaled(txt, x, y, ((((unsigned)(int)alpha) << 24) | col), sc);
}

static void draw_overhead_nick(const char *nick, double cx, double top_y, uint32_t col) {
    double w = text_ink_width(nick) * 0.5;
    double th = text_ink_height(nick) * 0.5;
    double tt = text_ink_top(nick) * 0.5;
    text_scaled(nick, cx - w / 2, top_y - 12 - th - tt, col, 0.5);
}

static void draw_online(void) {
    if (chat_open == 1) {
        draw_chat();
        return;
    }
    resolve_fighter_looks();
    draw_arena_background();
    draw_disco();
    draw_field();
    draw_event_plates();
    draw_event_candies();
    draw_dust();
    draw_player_shadow_layer();
    double i = 0;
    double b = 0;
    double rb = 0;
    double rx = 0;
    double ry = 0;
    double ra = 0;
    double punching = 0;
    while (i < max_players) {
        b = i * remote_fields;
        rb = i * punch_fields;
        if (arr_get(remotes, b + 5) == 1) {
            rx = arr_get(remotes, b + 1);
            ry = arr_get(remotes, b + 2);
            ra = arr_get(remotes, b + 3);
            punching = 0;
            if (arr_get(remote_punches, rb) == 1) {
                punching = 1;
                ra = atan2(arr_get(remote_punches, rb + 4), arr_get(remote_punches, rb + 3));
            } else if (remote_grab_punching(i) == 1) {
                punching = 1;
            }
            draw_fighter_shadow(rx, ry, ra, fighter_look_of(FLOOK_REMOTE + i),
                                fighter_pose_of(FLOOK_REMOTE + i, punching));
        }
        i = i + 1;
    }
    draw_turret_shadows();
    draw_player_hitbox_layer();
    if (show_hitboxes == 1) {
        i = 0;
        while (i < max_players) {
            rb = i * punch_fields;
            if (arr_get(rpunch_a, i) > 0) {
                draw_punch_box(arr_get(remote_punches, rb + 1), arr_get(remote_punches, rb + 2),
                               arr_get(remote_punches, rb + 3), arr_get(remote_punches, rb + 4), punch_reach,
                               punch_width, arr_get(rpunch_a, i));
            }
            i = i + 1;
        }
    }
    draw_ability_hitboxes();
    draw_gift();
    draw_boom();
    draw_remote_snow();
    draw_universe();
    draw_remote_universe();
    draw_station();
    draw_remote_station();
    draw_player_body_layer();
    double hw = 25 * cube_scale * player_scale;
    const char *myn = net_player_nick(net_slot());
    if (str_len(myn) > 0) {
        uint32_t ncol = admin_head_color_of(myn, 0xFFFFFF);
        draw_overhead_nick(myn, player->x, player->y - hw, ncol);
    }
    i = 0;
    while (i < max_players) {
        b = i * remote_fields;
        rb = i * punch_fields;
        if (arr_get(remotes, b + 5) == 1) {
            rx = arr_get(remotes, b + 1);
            ry = arr_get(remotes, b + 2);
            ra = arr_get(remotes, b + 3);
            punching = 0;
            if (arr_get(remote_punches, rb) == 1) {
                punching = 1;
                ra = atan2(arr_get(remote_punches, rb + 4), arr_get(remote_punches, rb + 3));
            } else if (remote_grab_punching(i) == 1) {
                punching = 1;
            }
            draw_fighter_body(rx, ry, ra, fighter_look_of(FLOOK_REMOTE + i),
                              fighter_pose_of(FLOOK_REMOTE + i, punching));
            const char *rn = net_player_nick(i);
            if (str_len(rn) > 0) {
                uint32_t rncol = admin_head_color_of(rn, 0xFFFFFF);
                draw_overhead_nick(rn, rx, ry - hw, rncol);
            }
        }
        i = i + 1;
    }
    draw_event_santa();
    draw_chat_bubbles();
    draw_status_aura(player->x, player->y, player->size, freeze_rgb, freeze_rim, pfreeze_a);
    draw_status_aura(player->x, player->y, player->size, poison_rgb, poison_rim, ppoison_a);
    draw_status_aura(player->x, player->y, player->size, stun_rgb, stun_rim, pstun_a);
    draw_snow();
    double w = screen_w / 3;
    hud_bar(20, 20, w, 28, player->hp, player->max_hp, 0x7ED321, 0);
    draw_controls();
    draw_fps_meter();
    draw_chat();
    draw_plates_hud();
    double st = net_status();
    double sy = chat_btn_y() + btn_h + 12;
    if (st == 4) {
        ctext(tr_no_connection(), sy, 0xFFFFFF);
        ctext(tr_reconnecting(), sy + 32, 0xFFFFFF);
    } else if (st == 1) {
        ctext(tr_connecting(), sy, 0xFFFFFF);
    }
    if (finished == 2) {
        draw_result(tr_defeat());
    }
    draw_banner();
}

static void drop_joy(void) {
    joy_id = -1;
    joy.dx = 0;
    joy.dy = 0;
    joy.ox = 0;
    joy.oy = 0;
}

static void joy_touch(double x, double y, double action, double id) {
    if (action == 1) {
        if (id == joy_id) {
            drop_joy();
        }
        return;
    }
    if (action == 0) {
        if (joy_id != -1) {
            return;
        }
        if (sqrt((x - joy.x) * (x - joy.x) + (y - joy.y) * (y - joy.y)) > 150) {
            return;
        }
        joy_id = id;
    }
    if (id != joy_id) {
        return;
    }
    double dx = x - joy.x;
    double dy = y - joy.y;
    double d = sqrt(dx * dx + dy * dy);
    if (d > joy.r) {
        joy.dx = dx / d;
        joy.dy = dy / d;
        joy.ox = joy.dx * joy.r;
        joy.oy = joy.dy * joy.r;
    } else {
        joy.dx = dx / joy.r;
        joy.dy = dy / joy.r;
        joy.ox = dx;
        joy.oy = dy;
    }
}

static void touch_game(double x, double y, double action, double id) {
    if (card_open != 0) {
        if (action == 0) {
            card_close();
        }
        return;
    }
    if (finished != 0) {
        if (action == 0 && finish_t >= alert_hold) {
            start_transition(0);
        }
        return;
    }
    if (action == 3) {
        aim_id = -1;
        super_id = -1;
        drop_joy();
        return;
    }
    if (game_state == ST_ONLINE) {
        if (touch_chat(x, y, action) == 1) {
            if (chat_open == 1) {
                aim_id = -1;
                super_id = -1;
                drop_joy();
            }
            return;
        }
        if (chat_open == 1) {
            return;
        }
    }
    if (back_hit(x, y) == 1 && action == 0) {
        start_transition(0);
        return;
    }
    if (class_has_super(player_class) == 1 && (game_state == ST_SOLO || game_state == ST_ONLINE)) {
        if (action == 0 && sqrt((x - atk2_x) * (x - atk2_x) + (y - atk2_y) * (y - atk2_y)) <= 96) {
            super_id = id;
            return;
        }
        if (action == 1 && id == super_id) {
            super_id = -1;
            fire_super();
            return;
        }
    }
    if (action == 0 && sqrt((x - atk_x) * (x - atk_x) + (y - atk_y) * (y - atk_y)) <= 96) {
        aim_id = id;
        return;
    }
    if (action == 1 && id == aim_id) {
        aim_id = -1;
        if (game_state == ST_SOLO || online_ready == 1) {
            fire_punch();
        }
        return;
    }
    joy_touch(x, y, action, id);
}
