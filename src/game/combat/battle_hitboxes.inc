static double fade_up_a(double a, double t) {
    if (t <= 0) {
        return 1;
    }
    a = a + dt / t;
    if (a > 1) {
        a = 1;
    }
    return a;
}

static double fade_down_a(double a, double t) {
    if (t <= 0) {
        return 0;
    }
    a = a - dt / t;
    if (a < 0) {
        a = 0;
    }
    return a;
}

static double hb_env(double a) {
    if (a < 0) {
        a = 0;
    }
    if (a > 1) {
        a = 1;
    }
    return a;
}

static double hb_zone_col(double env) {
    double a = floor(hitbox_zone_alpha * hb_env(env));
    if (a < 0) {
        a = 0;
    }
    if (a > 255) {
        a = 255;
    }
    return (((unsigned)(int)a) << 24) | ((unsigned)(int)hitbox_rgb);
}

static double hb_zone_bytes(double env) {
    if (env <= 0) {
        return 0;
    }
    double a = floor(hitbox_zone_alpha * hb_env(env));
    if (a < 1) {
        return 0;
    }
    if (a > 255) {
        return 255;
    }
    return a;
}

static void draw_hit_strip(double sx, double sy, double dx, double dy, double off0, double off1, double half_w,
                           double env) {
    if (show_hitboxes == 0 || hb_zone_bytes(env) < 1 || half_w <= 0) {
        return;
    }
    double len = off1 - off0;
    if (len <= 0) {
        return;
    }
    double mid = (off0 + off1) / 2;
    rect_rot(sx + dx * mid - len / 2, sy + dy * mid - half_w, len, half_w * 2, atan2(dy, dx), hb_zone_col(env));
}

static void draw_hit_circle(double x, double y, double r, double env) {
    if (show_hitboxes == 0) {
        return;
    }
    if (r <= 0 || hb_zone_bytes(env) < 1) {
        return;
    }
    circle(x, y, r, hb_zone_col(env));
}

static void draw_hit_snowball(double x, double y, double env) {
    draw_hit_circle(x, y, snow_ball_r, env);
}

static void draw_hit_snow(void) {
    if (snow_ball_a > 0) {
        draw_hit_snowball(gift->x, gift->y, snow_ball_a);
    }
    if (snow_boom_a > 0) {
        draw_hit_circle(boom_x, boom_y, super_radius, snow_boom_a);
    }
    if (game_state == ST_SOLO) {
        if (esnow_ball_a > 0) {
            draw_hit_snowball(enemy_gift->x, enemy_gift->y, esnow_ball_a);
        }
        if (esnow_boom_a > 0) {
            draw_hit_circle(enemy_boom_x, enemy_boom_y, super_radius, esnow_boom_a);
        }
    }
    if (game_state != ST_ONLINE) {
        return;
    }
    if (net_slot() < 0) {
        return;
    }
    double s = 0;
    while (s < max_players) {
        double b = s * snow_fields;
        if (arr_get(rsnow_ball_a, s) > 0) {
            draw_hit_snowball(arr_get(remote_snow, b + 3), arr_get(remote_snow, b + 4), arr_get(rsnow_ball_a, s));
        }
        if (arr_get(rsnow_boom_a, s) > 0) {
            draw_hit_circle(arr_get(remote_snow, b + 6), arr_get(remote_snow, b + 7), super_radius,
                            arr_get(rsnow_boom_a, s));
        }
        s = s + 1;
    }
}

static void draw_hit_dash_path(double sx, double sy, double dx, double dy, double cx, double cy, double pr,
                               double env) {
    if (pr <= 0) {
        return;
    }
    double travel = (cx - sx) * dx + (cy - sy) * dy;
    if (travel < 0) {
        travel = 0;
    }
    double len = azum_dash_speed * azum_dash_time;
    if (travel > len) {
        travel = len;
    }
    draw_hit_strip(sx, sy, dx, dy, 0, travel, pr, env);
}

static void draw_hit_dash(void) {
    if (dash_box_a > 0) {
        double dpr = dash_hit_radius_remote();
        if (game_state == ST_SOLO) {
            dpr = dash_hit_radius_solo();
        }
        draw_hit_dash_path(dash_x0, dash_y0, dash_dx, dash_dy, dash_x0 + dash_dx * dash_travel,
                           dash_y0 + dash_dy * dash_travel, dpr, dash_box_a);
    }
    if (game_state == ST_SOLO && edash_box_a > 0) {
        double epr = dash_hit_radius_solo();
        draw_hit_dash_path(enemy_dash_x0, enemy_dash_y0, enemy_dash_dx, enemy_dash_dy,
                           enemy_dash_x0 + enemy_dash_dx * enemy_dash_travel,
                           enemy_dash_y0 + enemy_dash_dy * enemy_dash_travel, epr, edash_box_a);
    }
    if (game_state != ST_ONLINE) {
        return;
    }
    if (net_slot() < 0) {
        return;
    }
    double s = 0;
    while (s < max_players) {
        double b = s * dash_fields;
        if (s != net_slot() && arr_get(rdash_box_a, s) > 0) {
            double rx = arr_get(remote_dash, b + 2);
            double ry = arr_get(remote_dash, b + 3);
            double rdx = arr_get(remote_dash, b + 4);
            double rdy = arr_get(remote_dash, b + 5);
            double leg = arr_get(remote_dash, b + 8);
            double pr = dash_hit_radius_remote();
            draw_hit_dash_path(rx, ry, rdx, rdy, rx + rdx * leg, ry + rdy * leg, pr, arr_get(rdash_box_a, s));
        }
        s = s + 1;
    }
}

static void draw_hit_universe_zone(double env) {
    if (show_hitboxes == 0 || hb_zone_bytes(env) < 1) {
        return;
    }
    rect(0, 0, screen_w, screen_h, hb_zone_col(env));
}

static void draw_hit_universe(void) {
    if (universe_box_a > 0) {
        draw_hit_universe_zone(universe_box_a);
    }
    if (game_state != ST_ONLINE) {
        return;
    }
    if (net_slot() < 0) {
        return;
    }
    double s = 0;
    while (s < max_players) {
        if (s != net_slot() && arr_get(runiverse_box_a, s) > 0) {
            draw_hit_universe_zone(arr_get(runiverse_box_a, s));
        }
        s = s + 1;
    }
}

static void draw_ability_hitboxes(void) {
    if (show_hitboxes == 0) {
        return;
    }
    draw_hit_snow();
    draw_hit_dash();
    draw_hit_universe();
}
